#include <Geode/modify/UILayer.hpp>
#include "BetterGeodeLogs.hpp"
#include "GJBaseGameLayer.hpp"


using namespace geode::prelude;


class $modify(MyLayer, UILayer) {

    struct Fields {
        // Maps a Touch ID to the ClickActionData currently handling that touch.
        std::unordered_map<int, ClickActionData*> claimedTouches;
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
        Log.d("UILayer", "touched: {} {} {}", touched, usedPos.x, getObjectHitbox(block).getMinX());
        return touched;
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) {
        if (!UILayer::ccTouchBegan(touch, event))
            return false;

        auto layer = static_cast<MyBaseLayer*>(m_gameLayer);
        auto& clickActions = layer->m_fields->clickActions;

        // Iterate through available actions to find one being touched.
        for (ClickActionData& actionData : clickActions) {
            if (actionData.taken)
                continue;

            if (isTouchInsideBlock(touch, actionData.collblock)) {
                layer->spawnGroup(actionData.action.groupIdCursorDown);

                actionData.taken = true;
                m_fields->claimedTouches.emplace(touch->getID(), &actionData);
                return true;
            }
        }

        return true;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent* event) {
        UILayer::ccTouchMoved(touch, event);

        auto layer = static_cast<MyBaseLayer*>(m_gameLayer);
        auto& claimedTouches = m_fields->claimedTouches;
        auto& clickActions = layer->m_fields->clickActions;

        auto it = claimedTouches.find(touch->getID());

        // --- Case 1: Touch is currently claimed by an action ---
        if (it != claimedTouches.end()) {
            ClickActionData* currentData = it->second;
            bool isInside = isTouchInsideBlock(touch, currentData->collblock);

            if (isInside && !currentData->calledEnter) {
                layer->spawnGroup(currentData->action.groupIdCursorEnter);
                currentData->calledEnter = true;
                currentData->calledExit = false;
            } else if (!isInside && !currentData->calledExit) {
                layer->spawnGroup(currentData->action.groupIdCursorExit);
                currentData->calledExit = true;
                currentData->calledEnter = false;
            }

            // Stealing Logic: Only attempt to steal if we are currently outside the claimed button
            // and have already triggered the Exit event.
            else if (!isInside && currentData->action.allowStealFrom) {
                // Check all other actions to see if we can steal one.
                for (ClickActionData& candidateData : clickActions) {
                    if (candidateData.action.stealTouches && !candidateData.taken) {
                        bool touchesCandidate = isTouchInsideBlock(touch, candidateData.collblock);

                        if (touchesCandidate) {
                            // Enter the new target
                            layer->spawnGroup(candidateData.action.groupIdCursorEnter);

                            candidateData.taken = true;
                            candidateData.calledEnter = true; // Mark as inside/entered
                            candidateData.calledExit = false;

                            // Release the old target
                            currentData->taken = false;
                            currentData->calledEnter = true; // Reset to default state
                            currentData->calledExit = false;

                            // Update the map to point to the new target
                            it->second = &candidateData;

                            // We found a new target, no need to check others
                            break;
                        }
                    }
                }
            }
        }
        // --- Case 2: Touch is not claimed, try to "steal" a new action mid-drag ---
        else {
            for (ClickActionData& actionData : clickActions) {
                if (actionData.taken)
                    continue;

                // Only objects configured to steal touches can be picked up here.
                if (actionData.action.stealTouches && isTouchInsideBlock(touch, actionData.collblock)) {
                    layer->spawnGroup(actionData.action.groupIdCursorEnter);

                    actionData.taken = true;
                    claimedTouches.emplace(touch->getID(), &actionData);
                }
            }
        }
    }

    void ccTouchEnded(CCTouch* touch, CCEvent* event) {
        UILayer::ccTouchEnded(touch, event);

        auto layer = static_cast<MyBaseLayer*>(m_gameLayer);
        auto& claimedTouches = m_fields->claimedTouches;

        if (auto it = claimedTouches.find(touch->getID()); it != claimedTouches.end()) {
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
        }
    }
};
