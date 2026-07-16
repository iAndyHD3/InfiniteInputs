#include "MyUIWrapper.hpp"
#include "BetterGeodeLogs.hpp"
#include "GJBaseGameLayer.hpp"

void MyUIWrapper::registerWithTouchDispatcher() {
    CCDirector::get()->getTouchDispatcher()->addTargetedDelegate(this, INT_MAX, editorUI != nullptr);
}

bool MyUIWrapper::isPointInOBB(const CCPoint& point, const std::array<CCPoint, 4>& corners) {
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

bool MyUIWrapper::isTouchInsideBlock(CCTouch* touch, CollisionBlock* block) {
    if (!block || !block->m_isActivated)
        return false;
    auto touchPos = touch->getLocation();
    auto layer = static_cast<MyBaseLayer*>(m_gameLayer);
    auto usedPos = block->m_isUIObject ? touchPos : layer->screenToGame(touchPos);
    if (OBB2D* box = block->getOrientedBox()) {
        return isPointInOBB(usedPos, box->m_corners);
    }
    return block->m_objectRect.containsPoint(usedPos);
}

void MyUIWrapper::updateNonUILayerTouches(float) {
    auto bs = static_cast<MyBaseLayer*>(m_gameLayer);
    if (bs->m_isEditor && bs->m_playbackMode != PlaybackMode::Playing) {
        this->unschedule(schedule_selector(MyUIWrapper::updateNonUILayerTouches));
        return;
    }
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

    auto* gf = layer->m_fields.self();
    auto* clickVec = gf ? &gf->clickActions : nullptr;
    auto mapSize = claimedTouches.size();
    uintptr_t curDataPtr = clickVec ? reinterpret_cast<uintptr_t>(clickVec->data()) : 0;
    bool reallocd = clickActionsDataAtInsert != 0 && curDataPtr != clickActionsDataAtInsert;
    Log.i("UILayer", "DIAG ENTER: claimedTouches={}, clickActions(size={}, cap={}, data={:x}), dataAtInsert={:x}{}",
        mapSize,
        clickVec ? clickVec->size() : -1,
        clickVec ? clickVec->capacity() : -1,
        curDataPtr,
        clickActionsDataAtInsert,
        reallocd ? " *** REALLOCATED ***" : "");

    processingCameraMove = true;
    for (auto& [touch, actionDataSet] : claimedTouches) {
        bool hasNonUI = false;
        for (auto* data : actionDataSet) {
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
                        clickActionsDataAtInsert);
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
    processingCameraMove = false;
}

void MyUIWrapper::touchMoved(CCTouch* touch) {
    if (processingCameraMove) {
        Log.w("UILayer", "DIAG REENTRY: touchMoved called DURING camera move iteration! touch={:x}, map_size={}",
            reinterpret_cast<uintptr_t>(touch), claimedTouches.size());
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

    auto it = claimedTouches.find(touch);

    if (it == claimedTouches.end()) {
        for (ClickActionData& data : gf->clickActions) {
            if (data.taken || !data.action.stealTouches)
                continue;
            if (!isTouchInsideBlock(touch, data.collblock))
                continue;

            layer->spawnGroup(data.action.groupIdCursorEnter);
            data.taken = true;
            claimedTouches[touch].insert(&data);
            clickActionsDataAtInsert = reinterpret_cast<uintptr_t>(gf->clickActions.data());
            Log.i("UILayer", "DIAG: touchMoved(unclaimed) inserted touch={:x}, map_now={}, vecData={:x}",
                reinterpret_cast<uintptr_t>(touch), claimedTouches.size(),
                clickActionsDataAtInsert);
        }
        return;
    }

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

    for (ClickActionData& candidate : gf->clickActions) {
        if (!candidate.action.stealTouches || candidate.taken)
            continue;
        if (!isTouchInsideBlock(touch, candidate.collblock))
            continue;

        layer->spawnGroup(candidate.action.groupIdCursorEnter);
        candidate.taken = true;
        candidate.calledEnter = true;
        candidate.calledExit = false;

        actionDataSet.insert(&candidate);
        clickActionsDataAtInsert = reinterpret_cast<uintptr_t>(gf->clickActions.data());
        Log.i("UILayer", "DIAG: touchMoved(steal) inserted into touch={:x}, set_now={}, vecData={:x}",
            reinterpret_cast<uintptr_t>(touch), actionDataSet.size(),
            clickActionsDataAtInsert);
    }
}

bool MyUIWrapper::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    geode::log::info("MyUIWrapper: ccTouchBegan called with touch id: {}", touch->getID());
    auto layer = static_cast<MyBaseLayer*>(m_gameLayer);
    auto gf = layer->m_fields.self();

    if (!gf->active) {

        return originalBegan(touch, event);
    }

    for (ClickActionData& actionData : gf->clickActions) {
        if (actionData.taken)
            continue;
        if (isTouchInsideBlock(touch, actionData.collblock) && actionData.action.ignoreInput) {
            Log.i("UILayer", "Touch began inside click action with ignoreInput=true, spawning down group and blocking touch");
            layer->spawnGroup(actionData.action.groupIdCursorDown);
            ignoredTouches.insert(touch);
            claimedTouches[touch].insert(&actionData);
            return true;
        }
    }

    if (!originalBegan(touch, event))
        return false;

    Log.i("UILayer", "Touch began with id: {}", touch->getID());

    for (ClickActionData& actionData : gf->clickActions) {
        if (actionData.taken)
            continue;

        if (isTouchInsideBlock(touch, actionData.collblock)) {
            layer->spawnGroup(actionData.action.groupIdCursorDown);

            actionData.taken = true;
            claimedTouches[touch].insert(&actionData);
            clickActionsDataAtInsert = reinterpret_cast<uintptr_t>(gf->clickActions.data());
            Log.i("UILayer", "DIAG: ccTouchBegan inserted touch={:x}, map_now={}, vecData={:x}",
                reinterpret_cast<uintptr_t>(touch), claimedTouches.size(),
                clickActionsDataAtInsert);
            schedule(schedule_selector(MyUIWrapper::updateNonUILayerTouches));
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

void MyUIWrapper::ccTouchMoved(CCTouch* touch, CCEvent* event) {
    geode::log::info("MyUIWrapper: ccTouchMoved called with touch id: {}", touch->getID());
    auto layer = static_cast<MyBaseLayer*>(m_gameLayer);
    auto gf = layer->m_fields.self();

    if (!gf->active) {
        originalMoved(touch, event);
        return;
    }

    if (ignoredTouches.find(touch) == ignoredTouches.end()) {
        originalMoved(touch, event);
    }

    touchMoved(touch);
}

void MyUIWrapper::ccTouchEnded(CCTouch* touch, CCEvent* event) {
    geode::log::info("MyUIWrapper: ccTouchEnded called with touch id: {}", touch->getID());
    auto layer = static_cast<MyBaseLayer*>(m_gameLayer);
    auto gf = layer->m_fields.self();

    if (ignoredTouches.find(touch) == ignoredTouches.end()) {
        originalEnded(touch, event);
    }

    if (!gf->active) {
        ignoredTouches.erase(touch);
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

    if (auto it = claimedTouches.find(touch); it != claimedTouches.end()) {
        for (auto* data : it->second) {
            data->taken = false;
            data->calledEnter = true;
            data->calledExit = false;

            if (isTouchInsideBlock(touch, data->collblock)) {
                layer->spawnGroup(data->action.groupIdCursorUp);
            }
        }

        claimedTouches.erase(it);
        Log.i("UILayer", "DIAG: ccTouchEnded erased touch={:x}, map_now={}, dataAtInsert={:x}",
            reinterpret_cast<uintptr_t>(touch), claimedTouches.size(),
            clickActionsDataAtInsert);
        if (claimedTouches.empty()) {
            this->unschedule(schedule_selector(MyUIWrapper::updateNonUILayerTouches));
        }
    }

    ignoredTouches.erase(touch);
}

bool MyUIWrapper::init() {
    if (!CCLayer::init()) {
        return false;
    }
    this->setTouchEnabled(true);
    return true;
}

MyUIWrapper* MyUIWrapper::create(MyBaseLayer* gameLayer, EditorUI* editorUI) {
    auto ret = new MyUIWrapper();
    if (ret && ret->init()) {
        ret->m_gameLayer = gameLayer;
        ret->editorUI = editorUI;
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

#include "UILayer.hpp";

bool MyUIWrapper::originalBegan(CCTouch* touch, CCEvent* event) {
    if(editorUI) {
        return editorUI->ccTouchBegan(touch, event);
    }
    auto ui = static_cast<MyUILayer*>(UILayer::get());
    ui->m_fields->allow = true;
    return ui->ccTouchBegan(touch, event);
}

void MyUIWrapper::originalMoved(CCTouch* touch, CCEvent* event) {
    if(editorUI) {
        editorUI->ccTouchMoved(touch, event);
    } else {
        auto ui = static_cast<MyUILayer*>(UILayer::get());
        ui->m_fields->allow = true;
        ui->ccTouchMoved(touch, event);
    }
}

void MyUIWrapper::originalEnded(CCTouch* touch, CCEvent* event) {
    if(editorUI) {
        editorUI->ccTouchEnded(touch, event);
    } else {
        auto ui = static_cast<MyUILayer*>(UILayer::get());
        ui->m_fields->allow = true;
        ui->ccTouchEnded(touch, event);
    }
}
