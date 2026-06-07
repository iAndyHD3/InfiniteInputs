#include <Geode/modify/UILayer.hpp>
#include "BetterGeodeLogs.hpp"
#include "GJBaseGameLayer.hpp"
#include "Geode/loader/Log.hpp"


using namespace geode::prelude;


class $modify(MyLayer, UILayer) {

    struct Fields {
        // Maps a Touch ID to the ClickActionData currently handling that touch.

        struct TouchHasher {
            std::size_t operator()(CCTouch* p) const { return p->getID(); }
        };
        struct TouchEquality {
            bool operator()(CCTouch* lhs, CCTouch* rhs) const { return lhs->getID() == rhs->getID(); }
        };

        boost::unordered_flat_map<CCTouch*, ClickActionData*, TouchHasher, TouchEquality> claimedTouches;
    };

    // Helper to safely retrieve the bounding box of a collision block.
    CCRect getObjectHitbox(CollisionBlock* block) {
        if (OBB2D* box = block->getOrientedBox()) {
            return box->getBoundingRect();
        }
        return block->m_objectRect;
    }

    // Determines if a touch intersects a specific collision block.
    // Handles coordinate conversion for UI vs Game objects.
    bool isTouchInsideBlock(CCTouch* touch, CollisionBlock* block) {
        if (!block->m_isActivated)
            return false;

        auto touchPos = touch->getLocation();
        auto layer = static_cast<MyBaseLayer*>(m_gameLayer);

        // UI objects use screen coordinates; game objects use world coordinates.
        auto usedPos = block->m_isUIObject ? touchPos : layer->screenToGame(touchPos);

        bool touched = getObjectHitbox(block).containsPoint(usedPos);
        // Log.d("UILayer", "touched: {} {} {}", touched, usedPos.x, getObjectHitbox(block).getMinX());
        return touched;
    }

    //reason for this:
    // If a touch starts on a non-UI object and the camera moves, the object's location will be updated according to the new camera position, which can cause it to no longer intersect with the originally touched object. By checking for this and updating the touch's position accordingly, we can ensure that the touch continues to interact with the intended object even if the camera moves.
    void updateNonUILayerTouches(float) {
        auto bs = static_cast<MyBaseLayer*>(m_gameLayer);

        if(bs->m_isEditor && bs->m_playbackMode != PlaybackMode::Playing) {
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
            // Log.i("UILayer", "skipping extra move check!");
            return;
        }

        lastCameraPos = gs.m_cameraPosition;
        lastCameraAngle = gs.m_cameraAngle;
        lastCameraZoom = gs.m_cameraZoom;

        // Log.d("UILayer", "in update");
        for (auto& [touch, actionData] : m_fields->claimedTouches) {
            if (actionData->collblock->m_isUIObject)
                continue;
            touchMoved(touch);
        }
    }

#pragma region touch hooks

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) {
        if (!UILayer::ccTouchBegan(touch, event))
            return false;

        auto layer = static_cast<MyBaseLayer*>(m_gameLayer);
        auto gf = layer->m_fields.self();

        if (!gf->active) {
            return true;
        }
        Log.i("UILayer", "Touch began with id: {}", touch->getID());

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
            for (GameObject* obj : it->second) {
                layer->moveObjectCorrectly(obj, touch->getLocation());
            }
        }

        // button actions
        for (ClickActionData& actionData : gf->clickActions) {
            if (actionData.taken)
                continue;

            if (isTouchInsideBlock(touch, actionData.collblock)) {
                layer->spawnGroup(actionData.action.groupIdCursorDown);

                actionData.taken = true;
                m_fields->claimedTouches.emplace(touch, &actionData);
                // Log.i("UILayer", "scheduling");
                schedule(schedule_selector(MyLayer::updateNonUILayerTouches));
                return true;
            }
        }

        return true;
    }

    // this is NOT the hook.
    void touchMoved(CCTouch* touch) {
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
            for (GameObject* obj : itobjs->second) {
                layer->moveObjectCorrectly(obj, touch->getLocation());
            }
        }

        auto& claimedTouches = m_fields.self()->claimedTouches;
        auto it = claimedTouches.find(touch);

        // -------------------------------------------------------------------------
        // Case 2: Touch is unclaimed — pick up a steal-eligible action mid-drag
        // -------------------------------------------------------------------------
        if (it == claimedTouches.end()) {
            for (ClickActionData& data : gf->clickActions) {
                if (data.taken || !data.action.stealTouches)
                    continue;
                if (!isTouchInsideBlock(touch, data.collblock))
                    continue;

                layer->spawnGroup(data.action.groupIdCursorEnter);
                data.taken = true;
                claimedTouches.emplace(touch, &data);
            }
            return;
        }

        // -------------------------------------------------------------------------
        // Case 1: Touch is already claimed — handle enter/exit events
        // -------------------------------------------------------------------------
        ClickActionData* current = it->second;
        bool isInside = isTouchInsideBlock(touch, current->collblock);

        // Trigger enter event when touch re-enters the claimed button's bounds
        if (isInside && !current->calledEnter) {
            layer->spawnGroup(current->action.groupIdCursorEnter);
            current->calledEnter = true;
            current->calledExit = false;
        }
        // Trigger exit event when touch leaves the claimed button's bounds
        else if (!isInside && !current->calledExit) {
            layer->spawnGroup(current->action.groupIdCursorExit);
            current->calledExit = true;
            current->calledEnter = false;
        }

        // Steal logic: only runs when outside the current button and it permits stealing
        if (isInside || !current->action.allowStealFrom)
            return;

        for (ClickActionData& candidate : gf->clickActions) {
            // Skip actions that don't accept stolen touches or are already taken
            if (!candidate.action.stealTouches || candidate.taken)
                continue;
            // Skip if the touch isn't over this candidate
            if (!isTouchInsideBlock(touch, candidate.collblock))
                continue;

            // Claim the new target and fire its enter event
            layer->spawnGroup(candidate.action.groupIdCursorEnter);
            candidate.taken = true;
            candidate.calledEnter = true;
            candidate.calledExit = false;

            // Release the old target and reset its state
            current->taken = false;
            current->calledEnter = true;
            current->calledExit = false;

            // Redirect the touch claim to the new target and stop searching
            it->second = &candidate;
            break;
        }
    }
    void ccTouchMoved(CCTouch* touch, CCEvent* event) {
        UILayer::ccTouchMoved(touch, event);
        touchMoved(touch);
    }

    void ccTouchEnded(CCTouch* touch, CCEvent* event) {
        UILayer::ccTouchEnded(touch, event);

        auto layer = static_cast<MyBaseLayer*>(m_gameLayer);
        auto gf = layer->m_fields.self();
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

        auto& claimedTouches = m_fields->claimedTouches;

        if (auto it = claimedTouches.find(touch); it != claimedTouches.end()) {
            ClickActionData* data = it->second;

            // Reset state for the next interaction
            data->taken = false;
            data->calledEnter = true;
            data->calledExit = false;

            // Trigger Up event if released inside the button
            if (isTouchInsideBlock(touch, data->collblock)) {
                layer->spawnGroup(data->action.groupIdCursorUp);
            }

            claimedTouches.erase(it);
            if (claimedTouches.empty()) {
                unschedule(schedule_selector(MyLayer::updateNonUILayerTouches));
            }
        }
    }

#pragma endregion
};
