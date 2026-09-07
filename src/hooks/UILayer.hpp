#pragma once

#include <Geode/Enums.hpp>
#include <Geode/binding/PlayerButtonCommand.hpp>
#include <Geode/modify/UILayer.hpp>
#include "BetterGeodeLogs.hpp"
#include "GJBaseGameLayer.hpp"
#include "Geode/loader/Log.hpp"


using namespace geode::prelude;


class $modify(MyLayer, UILayer) {

    static void onModify(auto& self) {
        if (!self.setHookPriorityPre("UILayer::ccTouchBegan", Priority::First)) {
            geode::log::warn("Failed to set hook priority for UILayer::ccTouchBegan");
        }
    }

    struct Fields {
        // Maps a Touch ID to the ClickActionData currently handling that touch.

        struct TouchHasher {
            std::size_t operator()(CCTouch* p) const { return p->getID(); }
        };
        struct TouchEquality {
            bool operator()(CCTouch* lhs, CCTouch* rhs) const { return lhs->getID() == rhs->getID(); }
        };

        boost::unordered_flat_map<CCTouch*, boost::unordered_flat_set<ClickActionData*>, TouchHasher, TouchEquality> claimedTouches;
        bool processingCameraMove = false;
        uintptr_t clickActionsDataAtInsert = 0;
    };

    static bool isPointInOBB(const CCPoint& point, const std::array<CCPoint, 4>& corners) {
        bool hasNeg = false, hasPos = false;
        for (int i = 0; i < 4; i++) {
            const auto& a = corners[i];
            const auto& b = corners[(i + 1) % 4];
            float cross = (b.x - a.x) * (point.y - a.y) - (b.y - a.y) * (point.x - a.x);
            if (cross < 0) hasNeg = true;
            else if (cross > 0) hasPos = true;
            if (hasNeg && hasPos) return false;
        }
        return true;
    }

    // Determines if a touch intersects a specific collision block.
    // Handles coordinate conversion for UI vs Game objects.
    bool isTouchInsideBlock(CCTouch* touch, CollisionBlock* block) {

        if (!block || !block->m_isActivated)
            return false;

        auto touchPos = touch->getLocation();
        auto layer = static_cast<MyBaseLayer*>(m_gameLayer);

        // UI objects use screen coordinates; game objects use world coordinates.
        auto usedPos = block->m_isUIObject ? touchPos : layer->screenToGame(touchPos);

        if (OBB2D* box = block->getOrientedBox()) {
            return isPointInOBB(usedPos, box->m_corners);
        }
        return block->m_objectRect.containsPoint(usedPos);
    }

    //reason for this:
    // If a touch starts on a non-UI object and the camera moves, the object's location will be updated according to the new camera position, which can cause it to no longer intersect with the originally touched object. By checking for this and updating the touch's position accordingly, we can ensure that the touch continues to interact with the intended object even if the camera moves.
    void updateNonUILayerTouches(float) {
        auto bs = static_cast<MyBaseLayer*>(m_gameLayer);

        if (bs->m_isEditor && bs->m_playbackMode != PlaybackMode::Playing) {
            unschedule(schedule_selector(MyLayer::updateNonUILayerTouches));
            return;
        }
        // check if camera moved...
        auto layer = static_cast<MyBaseLayer*>(m_gameLayer);
        auto& gs = layer->m_gameState;
        static CCPoint lastCameraPos = gs.m_cameraPosition;
        static float lastCameraAngle = gs.m_cameraAngle;
        static float lastCameraZoom = gs.m_cameraZoom;

        if (lastCameraPos == gs.m_cameraPosition && lastCameraAngle == gs.m_cameraAngle &&
            lastCameraZoom == gs.m_cameraZoom) {
            return;
        }

        lastCameraPos = gs.m_cameraPosition;
        lastCameraAngle = gs.m_cameraAngle;
        lastCameraZoom = gs.m_cameraZoom;

        // -- DIAG --
        auto* gf = layer->m_fields.self();
        auto* clickVec = gf ? &gf->clickActions : nullptr;
        auto mapSize = m_fields->claimedTouches.size();
        uintptr_t curDataPtr = clickVec ? reinterpret_cast<uintptr_t>(clickVec->data()) : 0;
        bool reallocd = m_fields->clickActionsDataAtInsert != 0 && curDataPtr != m_fields->clickActionsDataAtInsert;
        Log.i("UILayer", "DIAG ENTER: claimedTouches={}, clickActions(size={}, cap={}, data={:x}), dataAtInsert={:x}{}",
            mapSize,
            clickVec ? clickVec->size() : -1,
            clickVec ? clickVec->capacity() : -1,
            curDataPtr,
            m_fields->clickActionsDataAtInsert,
            reallocd ? " *** REALLOCATED ***" : "");

        m_fields->processingCameraMove = true;
        for (auto& [touch, actionDataSet] : m_fields->claimedTouches) {
            bool hasNonUI = false;
            for (auto* data : actionDataSet) {
                // -- DIAG crash-site guard (logs before crash, crash still happens) --
                if (!data) {
                    Log.e("UILayer", "DIAG CRASH: data is NULL! touch={:x}, setIdSize={}",
                        reinterpret_cast<uintptr_t>(touch), actionDataSet.size());
                } else if (!data->collblock) {
                    Log.e("UILayer", "DIAG CRASH: collblock is NULL! data={:x}, taken={}, calledEnter={}, calledExit={}",
                        reinterpret_cast<uintptr_t>(data), data->taken, data->calledEnter, data->calledExit);
                    if (clickVec) {
                        ptrdiff_t idx = data - clickVec->data();
                        Log.e("UILayer", "DIAG CRASH: clickActions size={}, cap={}, data={:x}, computed_idx={}, dataAtInsert={:x}",
                            clickVec->size(), clickVec->capacity(),
                            reinterpret_cast<uintptr_t>(clickVec->data()), idx,
                            m_fields->clickActionsDataAtInsert);
                    }
                }
                if (!data->collblock->m_isUIObject) { hasNonUI = true; break; }
            }
            if (hasNonUI) {
                Log.i("UILayer", "DIAG: calling touchMoved from camMove loop, touch={:x}",
                    reinterpret_cast<uintptr_t>(touch));
                touchMoved(touch);
            }
        }
        m_fields->processingCameraMove = false;
    }

#pragma region touch hooks

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) {

        auto layer = static_cast<MyBaseLayer*>(m_gameLayer);
        auto gf = layer->m_fields.self();

        if (!gf->active) {
            return UILayer::ccTouchBegan(touch, event);
        }

        // Check for ignored inputs first, before any other hooks run
        for (ClickActionData& actionData : gf->clickActions) {
            if (actionData.taken)
                continue;

            bool touchInside = isTouchInsideBlock(touch, actionData.collblock);
            if(touchInside && actionData.action.jump) {
                auto buttons = layer->m_queuedButtons;
                UILayer::ccTouchBegan(touch, event);
                buttons.push_back(PlayerButtonCommand{
                    .m_button = static_cast<PlayerButton>(1),
                    .m_isPush = true,
                    .m_isPlayer2 = false,
                    .m_step = 0,
                    .m_timestamp = 0
                });
                layer->m_queuedButtons = buttons;
                return true;
            }

            if (!actionData.action.jump && touchInside && actionData.action.ignoreInput) {
                Log.i("UILayer", "Touch id {} began inside click action with ignoreInput=true, spawning down group and blocking touch", touch->getID());
                layer->spawnGroup(actionData.action.groupIdCursorDown);
                m_fields->claimedTouches[touch].insert(&actionData);

                UILayer::ccTouchBegan(touch, event);

                gd::vector<PlayerButtonCommand> buttons;
                for(const auto& b : layer->m_queuedButtons) {
                    if(b.m_button != PlayerButton::Jump) {
                        buttons.emplace_back(b);
                    }
                }
                layer->m_queuedButtons = buttons;

                this->m_p1TouchId = -1;
                return true;
            }
        }

        // Call original (triggers other hooks in priority chain)
        if (!UILayer::ccTouchBegan(touch, event))
            return false;

        Log.i("UILayer", "Touch began with id: {}", touch->getID());

        // button actions
        for (ClickActionData& actionData : gf->clickActions) {
            if (actionData.taken)
                continue;

            if (isTouchInsideBlock(touch, actionData.collblock)) {
                layer->spawnGroup(actionData.action.groupIdCursorDown);

                actionData.taken = true;
                m_fields->claimedTouches[touch].insert(&actionData);
                m_fields->clickActionsDataAtInsert = reinterpret_cast<uintptr_t>(gf->clickActions.data());
                Log.i("UILayer", "DIAG: ccTouchBegan inserted touch={:x}, map_now={}, vecData={:x}",
                    reinterpret_cast<uintptr_t>(touch), m_fields->claimedTouches.size(),
                    m_fields->clickActionsDataAtInsert);
                schedule(schedule_selector(MyLayer::updateNonUILayerTouches));
            }
        }

        auto [first, last] = gf->touchActions.equal_range(touch->getID());
        for (auto it = first; it != last; ++it) {
            const TouchAction& action = it->second;
            layer->spawnGroup(action.groupIdTouchDown);
            layer->updateItemId(action.itemId_x, touch->getLocation().x);
            layer->updateItemId(action.itemId_y, touch->getLocation().y);
            layer->updateItemId(action.itemId_deltaX, touch->getDelta().x);
            layer->updateItemId(action.itemId_deltaY, touch->getDelta().y);
            Log.i("UILayer", "Touch down with touch action: {}, {}, {}, {}, {}", action.groupIdTouchDown,
                  action.itemId_x, action.itemId_y, action.itemId_deltaX, action.itemId_deltaY);
        }

        auto it = gf->touchFollowObjects.find(touch->getID());
        if (it != gf->touchFollowObjects.end()) {
            bool nextNew = true;
            bool applyDelta = false;
            CCPoint delta;
            for (GameObject* obj : it->second) {
                if (!obj) {
                    nextNew = true;
                    applyDelta = false;
                    continue;
                }
                if (applyDelta) {
                    layer->moveObjectByDelta(obj, delta);
                } else if (nextNew) {
                    nextNew = false;
                    if (obj->m_hasGroupParentsString) {
                        applyDelta = true;
                        delta = layer->moveObjectCorrectlyGetDelta(obj, touch->getLocation());
                    } else {
                        applyDelta = false;
                        layer->moveObjectCorrectly(obj, touch->getLocation());
                    }
                } else {
                    layer->moveObjectCorrectly(obj, touch->getLocation());
                }
            }
        }

        return true;
    }

    // this is NOT the hook.
    void touchMoved(CCTouch* touch) {
        // -- DIAG re-entrancy detection --
        if (m_fields->processingCameraMove) {
            Log.w("UILayer", "DIAG REENTRY: touchMoved called DURING camera move iteration! touch={:x}, map_size={}",
                reinterpret_cast<uintptr_t>(touch), m_fields->claimedTouches.size());
        }

        auto layer = static_cast<MyBaseLayer*>(m_gameLayer);
        auto gf = layer->m_fields.self();

        if (!gf->active)
            return;

        auto [first, last] = gf->touchActions.equal_range(touch->getID());
        for (auto it = first; it != last; ++it) {
            const TouchAction& action = it->second;
            layer->updateItemId(action.itemId_x, touch->getLocation().x);
            layer->updateItemId(action.itemId_y, touch->getLocation().y);
            layer->updateItemId(action.itemId_deltaX, touch->getDelta().x);
            layer->updateItemId(action.itemId_deltaY, touch->getDelta().y);
            Log.i("UILayer", "Touch moved with touch action: {}, {}, {}, {}, {}", action.groupIdTouchDown,
                  action.itemId_x, action.itemId_y, action.itemId_deltaX, action.itemId_deltaY);
        }

        auto itobjs = gf->touchFollowObjects.find(touch->getID());
        if (itobjs != gf->touchFollowObjects.end()) {
            bool nextNew = true;
            bool applyDelta = false;
            CCPoint delta;
            for (GameObject* obj : itobjs->second) {
                if (!obj) {
                    nextNew = true;
                    applyDelta = false;
                    continue;
                }
                if (applyDelta) {
                    layer->moveObjectByDelta(obj, delta);
                } else if (nextNew) {
                    nextNew = false;
                    if (obj->m_hasGroupParentsString) {
                        applyDelta = true;
                        delta = layer->moveObjectCorrectlyGetDelta(obj, touch->getLocation());
                    } else {
                        applyDelta = false;
                        layer->moveObjectCorrectly(obj, touch->getLocation());
                    }
                } else {
                    layer->moveObjectCorrectly(obj, touch->getLocation());
                }
            }
        }

        auto it = m_fields->claimedTouches.find(touch);

        // -------------------------------------------------------------------------
        // Case 2: Touch is unclaimed — pick up a steal-eligible action mid-drag
        // -------------------------------------------------------------------------
        if (it == m_fields->claimedTouches.end()) {
            for (ClickActionData& data : gf->clickActions) {
                if (data.taken || !data.action.stealTouches)
                    continue;
                if (!isTouchInsideBlock(touch, data.collblock))
                    continue;

                layer->spawnGroup(data.action.groupIdCursorEnter);
                data.taken = true;
                m_fields->claimedTouches[touch].insert(&data);
                m_fields->clickActionsDataAtInsert = reinterpret_cast<uintptr_t>(gf->clickActions.data());
                Log.i("UILayer", "DIAG: touchMoved(unclaimed) inserted touch={:x}, map_now={}, vecData={:x}",
                    reinterpret_cast<uintptr_t>(touch), m_fields->claimedTouches.size(),
                    m_fields->clickActionsDataAtInsert);
            }
            return;
        }

        // -------------------------------------------------------------------------
        // Case 1: Touch is already claimed — handle enter/exit events for each
        // -------------------------------------------------------------------------
        auto& actionDataSet = it->second;

        for (auto* current : actionDataSet) {
            bool isInside = isTouchInsideBlock(touch, current->collblock);

            if (isInside && !current->calledEnter) {
                layer->spawnGroup(current->action.groupIdCursorEnter);
                current->calledEnter = true;
                current->calledExit = false;
            } else if (!isInside && !current->calledExit) {
                layer->spawnGroup(current->action.groupIdCursorExit);
                current->calledExit = true;
                current->calledEnter = false;
            }
        }

        // Steal logic: claim any untaken, stealable, intersected action
        for (ClickActionData& candidate : gf->clickActions) {
            if (!candidate.action.stealTouches || candidate.taken)
                continue;
            if (!isTouchInsideBlock(touch, candidate.collblock))
                continue;

            layer->spawnGroup(candidate.action.groupIdCursorEnter);
            candidate.taken = true;
            candidate.calledEnter = true;
            candidate.calledExit = false;

            m_fields->claimedTouches[touch].insert(&candidate);
            m_fields->clickActionsDataAtInsert = reinterpret_cast<uintptr_t>(gf->clickActions.data());
            Log.i("UILayer", "DIAG: touchMoved(steal) inserted into touch={:x}, set_now={}, vecData={:x}",
                reinterpret_cast<uintptr_t>(touch), m_fields->claimedTouches.size(),
                m_fields->clickActionsDataAtInsert);
        }
    }

    void ccTouchMoved(CCTouch* touch, CCEvent* event) {
        auto layer = static_cast<MyBaseLayer*>(m_gameLayer);
        auto gf = layer->m_fields.self();

        if (!gf->active) {
            UILayer::ccTouchMoved(touch, event);
            return;
        }


        touchMoved(touch);
    }

    void ccTouchEnded(CCTouch* touch, CCEvent* event) {
        auto layer = static_cast<MyBaseLayer*>(m_gameLayer);
        auto gf = layer->m_fields.self();

            UILayer::ccTouchEnded(touch, event);

        if (!gf->active) {
            return;
        }

        auto [first, last] = gf->touchActions.equal_range(touch->getID());
        for (auto it = first; it != last; ++it) {
            const TouchAction& action = it->second;
            layer->spawnGroup(action.groupIdTouchUp);
            layer->updateItemId(action.itemId_x, 0);
            layer->updateItemId(action.itemId_y, 0);
            layer->updateItemId(action.itemId_deltaX, 0);
            layer->updateItemId(action.itemId_deltaY, 0);
            Log.i("UILayer", "Touch up with touch action: {}, {}, {}, {}, {}", action.groupIdTouchUp, action.itemId_x,
                  action.itemId_y, action.itemId_deltaX, action.itemId_deltaY);
        }

        if (auto it = m_fields->claimedTouches.find(touch); it != m_fields->claimedTouches.end()) {
            for (auto* data : it->second) {
                // Reset state for the next interaction
                data->taken = false;
                data->calledEnter = true;
                data->calledExit = false;

                // Trigger Up event if released inside the button
                if (isTouchInsideBlock(touch, data->collblock)) {
                    layer->spawnGroup(data->action.groupIdCursorUp);
                    if(data->action.jump) {
                        layer->m_queuedButtons.push_back(PlayerButtonCommand{
                    .m_button = static_cast<PlayerButton>(1),
                    .m_isPush = false,
                    .m_isPlayer2 = false,
                    .m_step = 0,
                    .m_timestamp = 0
                    });
                    }
                }
            }

            m_fields->claimedTouches.erase(it);
            Log.i("UILayer", "DIAG: ccTouchEnded erased touch={:x}, map_now={}, dataAtInsert={:x}",
                reinterpret_cast<uintptr_t>(touch), m_fields->claimedTouches.size(),
                m_fields->clickActionsDataAtInsert);
            if (m_fields->claimedTouches.empty()) {
                unschedule(schedule_selector(MyLayer::updateNonUILayerTouches));
            }
        }

    }

#pragma endregion
};
