#include "GJBaseGameLayer.hpp"
#include <Geode/binding/EffectGameObject.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/UILayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/PlatformToolbox.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/TextGameObject.hpp>
#include <Geode/cocos/CCDirector.h>
#include <arc/prelude.hpp>
#include <arc/runtime/Runtime.hpp>
#include <climits>
#include <enchantum/enchantum.hpp>
#include <numbers>
#include <scn/scan.h>
#include "BetterGeodeLogs.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/ui/Popup.hpp"
#include "Geode/utils/cocos.hpp"
#include "LevelKeys.hpp"
#include "TextParsing.hpp"

#define ETOSTRING(k) enchantum::to_string(k)



static gd::vector<short> getGroupIDs(GameObject* obj) {
    gd::vector<short> res;

    if (obj->m_groups && obj->m_groups->at(0))
        for (auto i = 0; i < obj->m_groupCount; i++)
            res.push_back(obj->m_groups->at(i));
    return res;
}

static bool hasGroup(GameObject* obj, int group) {
    for (const auto& g : getGroupIDs(obj)) {
        if (g == group)
            return true;
    }
    return false;
}


void MyBaseLayer::Fields::addKeyBind(LevelKeys key, bool down, int groupId) {
    // getKeysMap(down).emplace(key, groupId);
}


std::optional<boost::unordered_flat_set<groupId>> MyBaseLayer::Fields::getGroupId(const KeyActionMapKey& key) {
    auto groupids = keyMap.find(key);
    return groupids != keyMap.end() ? std::optional<boost::unordered_flat_set<groupId>>(groupids->second) : std::nullopt;
}


void MyBaseLayer::Fields::spawnGroupKeys(const KeyActionMapKey& key) {
    if (auto groups = getGroupId(key)) {
        for (auto group : *groups) {
            Log.i("gjbgl", "KEY: {}, {}, GROUP: {}", enchantum::to_string(key.key), key.keyDown ? "down" : "up", group);
            layer->spawnGroup(group);
        }
    }
}

void MyBaseLayer::Fields::spawnGroupSimple(LevelKeys key) {
    if (key == LevelKeys::deltaX || key == LevelKeys::deltaY) {
        return;
    }
    if (auto it = simpleKeyMap.find(key); it != simpleKeyMap.end()) {
        for (auto group : it->second) {
            Log.i("gjbgl", "[SIMPLE] KEY: {}, GROUP: {}", enchantum::to_string(key), group);
            layer->spawnGroup(group);
        }
    } else {
        Log.e("gjbl", "Could not find group to spawn on key: {}, size: {}", ETOSTRING(key), simpleKeyMap.size());
    }
}

bool MyBaseLayer::Fields::hasAnyMouseKeyActive() {
    return !cursorFollowGroupIds.empty() || simpleKeyMap.contains(LevelKeys::mouseX) ||
           simpleKeyMap.contains(LevelKeys::mouseY) || simpleKeyMap.contains(LevelKeys::x) ||
           simpleKeyMap.contains(LevelKeys::y);
}


$override
bool MyBaseLayer::init() {
    if (!GJBaseGameLayer::init())
        return false;

    scheduleOnce(schedule_selector(MyBaseLayer::delayedInit), 0);
    return true;
}


#include "MyUIWrapper.hpp"



void MyBaseLayer::delayedInit(float) {

    if (!m_isEditor) {
        // return scheduleOnce(schedule_selector(MyBaseLayer::setupKeybinds_step0), 0);
        setupKeybinds_step0(0);
        return;
    }


    for (auto obj : CCArrayExt<GameObject*>(m_objects)) {
        if (obj->m_objectID != 914)
            continue;

        auto text = static_cast<TextGameObject*>(obj)->m_text;

        if (!isOldFormatString(text))
            continue;

        auto popup = geode::createQuickPopup(
                "Old Input Format Detected",
                "<cr>MAKE A BACKUP OF THE LEVEL BEFORE CONVERTING! THIS PROCESS IS NOT REVERSIBLE!</c>\nThe input "
                "trigger data belongs to an older version and needs to be <cg>converted</c> to work,  <co> Convert all "
                "input triggres to new format?</c>",
                "Convert", "Cancel",
                [this](FLAlertLayer* layer, bool btn2) {
                    for (auto obj : CCArrayExt<GameObject*>(m_objects)) {
                        if (obj->m_objectID != 914)
                            continue;
                        auto label = static_cast<TextGameObject*>(obj);
                        std::string text = label->m_text.c_str();
                        if (auto reuslt = scn::scan<std::string, int>(text, "inf_inp:{} = {}")) {
                            auto& [key, group] = reuslt->values();
                            label->updateTextObject(fmt::format("inf_inp:2 {} {}", key, group), false);
                        }
                        if (auto result = scn::scan<std::string, char, int>(text, "inf_inp:{} {} = {}")) {
                            auto& [key, keyDown, group] = result->values();
                            label->updateTextObject(fmt::format("inf_inp:1 {} {} {}", key, keyDown, group), false);
                        }
                    }
                },
                false, true);

        popup->m_scene = this;
        popup->show();

        break;
    }
    schedule(schedule_selector(MyBaseLayer::editorActiveHandlerLoop), 0);
}



void MyBaseLayer::editorActiveHandlerLoop(float) {

    auto fields = m_fields.self();


    auto editor = reinterpret_cast<LevelEditorLayer*>(this);
    bool inPlaytest = m_playbackMode == PlaybackMode::Playing;

    // start playtest
    if (!fields->active && inPlaytest) {

        struct Test : CCLayer {
            virtual bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
                log::info("Test: ccTouchBegan called with touch id: {}", touch->getID());
                return true;
            }
            virtual void ccTouchMoved(CCTouch* touch, CCEvent* event) override {
                log::info("Test: ccTouchMoved called with touch id: {}", touch->getID());
            }
            virtual void ccTouchEnded(CCTouch* touch, CCEvent* event) override {
                log::info("Test: ccTouchEnded called with touch id: {}", touch->getID());
            }
        };

        fields->active = true;
        setupKeybinds_step0(0);


    }
    // stop playtest
    else if (fields->active && !inPlaytest) {
        // Clear claimed touches in wrapper to prevent dangling ClickActionData pointers
        // if (fields->uiWrapper) {
        //     fields->uiWrapper->claimedTouches.clear();
        //     fields->uiWrapper->ignoredTouches.clear();
        //     fields->uiWrapper->unschedule(schedule_selector(MyUIWrapper::updateNonUILayerTouches));
        // }

        *fields = MyBaseLayer::Fields();
    }
}

void MyBaseLayer::moveObjectCorrectly(GameObject* o, CCPoint to) {
    CCPoint layerObjPos;
    if (o->m_isUIObject) {
        layerObjPos = o->getRealPosition();
    } else {
        layerObjPos = this->convertToNodeSpace(o->getRealPosition());
        to = screenToGame(to);
    }
    auto delta = to - layerObjPos;
    moveObject(o, delta.x, delta.y, false);
}

CCPoint MyBaseLayer::moveObjectCorrectlyGetDelta(GameObject* obj, CCPoint to) {
    CCPoint layerObjPos;
    if (obj->m_isUIObject) {
        layerObjPos = obj->getRealPosition();
    } else {
        layerObjPos = this->convertToNodeSpace(obj->getRealPosition());
        to = screenToGame(to);
    }
    auto delta = to - layerObjPos;
    moveObject(obj, delta.x, delta.y, false);
    return delta;
}

void MyBaseLayer::moveObjectByDelta(GameObject* obj, CCPoint delta) {
    moveObject(obj, delta.x, delta.y, false);
}

void MyBaseLayer::updateLoop(float) {


    auto fields = m_fields.self();
    auto mousePos = geode::cocos::getMousePos();
    bool nextNew = true;
    bool applyDelta = false;
    CCPoint delta;
    for (const auto& o : fields->cursorFollowObjects) {
        if(!o) {
            nextNew = true;
            applyDelta = false;
            continue;
        }
        if(applyDelta) {
            moveObject(o, delta.x, delta.y, false);
        }
        else if(nextNew) {
            nextNew = false;
            if(o->m_hasGroupParentsString) {
                applyDelta = true;
                delta = moveObjectCorrectlyGetDelta(o, mousePos);
            } else {
                applyDelta = false;
                moveObjectCorrectly(o, mousePos);
            }
        }
        else {
            moveObjectCorrectly(o, mousePos);
        }
    }

    // if (!fields->spawnedModLoaded) {
    //     fields->spawnGroupSimple(LevelKeys::modLoaded);
    //     fields->spawnedModLoaded = true;
    // }
}

// PLAYLAYER START!
void MyBaseLayer::resetLevelVariables() {
    GJBaseGameLayer::resetLevelVariables();

    for (const auto& o : m_fields->cursorFollowObjects) {
        if(o) {
            o->setLastPosition(o->getPosition());
        }
    }
    for (const auto& [touchId, touchVec] : m_fields->touchFollowObjects) {
        for (const auto& o : touchVec) {
            if(o) {
                o->setLastPosition(o->getPosition());
            }
        }
    }
    Log.i("gjbl", "Reset level variables, reset cursor follow objects' last position");
}

void MyBaseLayer::spawnModLoadedGroups(float) {

    Log.i("gjbgl", "Spawning mod load groupsm started: {}", m_started);
    auto fields = m_fields.self();
    fields->spawnGroupSimple(LevelKeys::modLoaded);

#if defined(GEODE_IS_DESKTOP)
    fields->spawnGroupSimple(LevelKeys::modLoadedPC);
#endif

#if defined(GEODE_IS_MOBILE)
    fields->spawnGroupSimple(LevelKeys::modLoadedMobile);
#endif

    auto windowSize = m_uiLayer->getContentSize();

    fields->updateItemIdWithSimpleKey(this, LevelKeys::windowWidth, std::lrint(windowSize.width));
    fields->updateItemIdWithSimpleKey(this, LevelKeys::windowHeight, std::lrint(windowSize.height));
}


// true if correctly registered atleast one keybind
bool MyBaseLayer::setupTextLabelKeys_step1() {
    auto f = m_fields.self();

    // parse all labels
    for (const auto& obj : m_objects->asExt<GameObject*>()) {
        if (obj->m_objectID == 914) {
            std::string_view t = static_cast<TextGameObject*>(obj)->m_text;
            if (auto parsed = parseObjectString(t)) {
                if (KeyAction* label = std::get_if<KeyAction>(&*parsed)) {
                    f->keyMap[KeyActionMapKey{label->key, label->keyDown}].insert(label->group);
                } else if (SimpleKeyAction* label = std::get_if<SimpleKeyAction>(&*parsed)) {
                    f->simpleKeyMap[label->key].insert(label->group);
                } else if (ClickAction* action = std::get_if<ClickAction>(&*parsed)) {
                    f->clickActionAddQueue.push_back(std::move(*action));
                } else if (TouchAction* action = std::get_if<TouchAction>(&*parsed)) {
                    f->touchActions.insert({action->touch_id, std::move(*action)});
                }
            } else {
                Log.e("gjbgl", "Failed to parse label: {}", static_cast<TextGameObject*>(obj)->m_text);
            }
        }
    }

    
    auto it = f->simpleKeyMap.find(LevelKeys::cursor);
    if (it != f->simpleKeyMap.end()) {
        f->cursorFollowGroupIds = it->second;
        PlatformToolbox::toggleLockCursor(false);
        f->cursorFollowObjects.clear();
        for (auto g : f->cursorFollowGroupIds) {
            Log.i("gjbgl", "SETTING UP CURSOR GROUP {}", g);
        }
    }

    boost::unordered_flat_map<int, boost::unordered_flat_set<int>> touchIdToFollowGroupIds;
    for(const auto& [touchId, action] : f->touchActions) {
        touchIdToFollowGroupIds[touchId].insert(action.groupIdLockObjectsToTouch);
    }
    
    struct TempOrderObj {
        GameObject* obj;
        int cursorfollowgroup;
    };

    std::vector<TempOrderObj> cursorFollowObjectsTemp;

    for (const auto& obj : m_objects->asExt<GameObject*>()) {
        if (obj->m_objectID == 1816) {
            for (const auto& clickaction : f->clickActionAddQueue) {
                if (clickaction.collisionBlockId == ((EffectGameObject*)(obj))->m_itemID) {
                    // Log.i("gjbgl", "{}", obj);
                    Log.i("gjbgl", "{}", obj);
                    f->clickActions.emplace_back(((EffectGameObject*)(obj)), std::move(clickaction));
                }
            }
        }
        for (auto cursorGroupId : f->cursorFollowGroupIds) {
            if (hasGroup(obj, cursorGroupId)) {
                cursorFollowObjectsTemp.push_back({obj, cursorGroupId});    
            }
        }

    }

    //sort first by cursorfollowgroup, then by m_hasGroupParentsString (true first)
    std::sort(cursorFollowObjectsTemp.begin(), cursorFollowObjectsTemp.end(), [](const TempOrderObj& a, const TempOrderObj& b) {
        if (a.cursorfollowgroup != b.cursorfollowgroup) {
            return a.cursorfollowgroup < b.cursorfollowgroup;
        }
        return a.obj->m_hasGroupParentsString > b.obj->m_hasGroupParentsString;
    });



    int lastGroup = -1;
    for(const auto& tempObj : cursorFollowObjectsTemp) {
        if(lastGroup != -1 && tempObj.cursorfollowgroup != lastGroup) {
            f->cursorFollowObjects.push_back(nullptr); // add a null object to separate groups
        }
        lastGroup = tempObj.cursorfollowgroup;
        f->cursorFollowObjects.push_back(tempObj.obj);
    }

    // Build touch follow object vectors with group-parent ordering
    f->touchFollowObjects.clear();
    for (const auto& [touchId, groupIds] : touchIdToFollowGroupIds) {
        std::vector<TempOrderObj> tempTouchObjects;
        for (const auto& obj : m_objects->asExt<GameObject*>()) {
            for (const auto& groupId : groupIds) {
                if (hasGroup(obj, groupId)) {
                    tempTouchObjects.push_back({obj, groupId});
                }
            }
        }
        std::sort(tempTouchObjects.begin(), tempTouchObjects.end(), [](const TempOrderObj& a, const TempOrderObj& b) {
            if (a.cursorfollowgroup != b.cursorfollowgroup) {
                return a.cursorfollowgroup < b.cursorfollowgroup;
            }
            return a.obj->m_hasGroupParentsString > b.obj->m_hasGroupParentsString;
        });
        std::vector<GameObject*>& touchVec = f->touchFollowObjects[touchId];
        int lastGroup = -1;
        for (const auto& tempObj : tempTouchObjects) {
            if (lastGroup != -1 && tempObj.cursorfollowgroup != lastGroup) {
                touchVec.push_back(nullptr);
            }
            lastGroup = tempObj.cursorfollowgroup;
            touchVec.push_back(tempObj.obj);
        }
    }

    for(const auto& obj : f->cursorFollowObjects) {
        if(!obj) {
            Log.i("gjbgl", "Cursor follow object: nullptr (group separator)");
            continue;
        }
        Log.i("gjbgl", "Cursor follow object: {}, hasGroupParentsString: {}", obj, obj->m_hasGroupParentsString);
    }

    f->clickActionAddQueue.clear();


    Log.i("gjbgl", "Added {} down keys", f->keyMap.size());
    Log.i("gjbgl", "Added {} simple keys", f->simpleKeyMap.size());
    Log.i("gjbgl", "Added {} button actions", f->clickActions.size());
    Log.i("gjbgl", "Added {} touch actions", f->touchActions.size());
    if (!f->cursorFollowGroupIds.empty()) {
        for (auto g : f->cursorFollowGroupIds) {
            Log.i("gjbgl", "Cursor Group: {}", g);
        }
    }
    // Log.i("gjbgl", "Wheel Up Group: {}", fields->wheelUpGroup);
    // Log.i("gjbgl", "Wheel Down Group: {}", fields->wheelDownGroup);

    f->addedAtleastOneKey =
            !f->keyMap.empty() || !f->simpleKeyMap.empty() || !f->clickActions.empty() || !f->touchActions.empty();

    return f->addedAtleastOneKey;
}

bool MyBaseLayer::isModActive() { return m_fields->active; }

void MyBaseLayer::setupKeybinds_step0(float) {
    if (!setupTextLabelKeys_step1()) {
        Log.e("gjbgl", "not parsed any labels");
        return;
    }

    auto fields = m_fields.self();

    fields->layer = this;


    fields->active = true;

    schedule(schedule_selector(MyBaseLayer::updateLoop));

    if (m_isEditor) {
        scheduleOnce(schedule_selector(MyBaseLayer::spawnModLoadedGroups), 0);
    }

    if (fields->simpleKeyMap.contains(LevelKeys::deltaX) || fields->simpleKeyMap.contains(LevelKeys::deltaY) ||
        fields->simpleKeyMap.contains(LevelKeys::mouseX) || fields->simpleKeyMap.contains(LevelKeys::mouseY)) {
        schedule(schedule_selector(MyBaseLayer::updateMouseDeltaKeys));
        PlatformToolbox::toggleLockCursor(false);
    }
}


void MyBaseLayer::Fields::updateItemIdWithSimpleKey(MyBaseLayer* layer, LevelKeys key, int value) {
    auto it = simpleKeyMap.find(key);
    if (it != simpleKeyMap.end()) {
        for (auto itemId : it->second) {
            layer->updateItemId(itemId, value);
        }
    }
}


void MyBaseLayer::updateMouseDeltaKeys(float) {
    auto fields = m_fields.self();
    auto mousePos = getMousePos();

    if (mousePos == fields->lastMousePos) {
        if (fields->shouldStopUpdatingMousePos) {
            return;
        }
        // Log.i("gjbgl", "Mouse stopped moving, resetting delta keys to 0");
        fields->updateItemIdWithSimpleKey(this, LevelKeys::deltaX, 0);
        fields->updateItemIdWithSimpleKey(this, LevelKeys::deltaY, 0);
        fields->shouldStopUpdatingMousePos = true;
    } else {
        fields->shouldStopUpdatingMousePos = false;
    }

    auto delta = mousePos - fields->lastMousePos;
    fields->lastMousePos = mousePos;

    // The delta is usually a very small float, so we multiply it to get a more reasonable integer value (cuz the item
    // are ints)
    int dx = static_cast<int>(delta.x * 10);
    int dy = static_cast<int>(delta.y * 10);

    fields->updateItemIdWithSimpleKey(this, LevelKeys::deltaX, std::lrint(dx));
    fields->updateItemIdWithSimpleKey(this, LevelKeys::deltaY, std::lrint(dy));
    fields->updateItemIdWithSimpleKey(this, LevelKeys::mouseX, std::lrint(mousePos.x));
    fields->updateItemIdWithSimpleKey(this, LevelKeys::mouseY, std::lrint(mousePos.y));
    // Log.i("gjbgl", "Mouse moved, updating item ids");
}

void MyBaseLayer::handleScroll(float x, float y) {
    if (y == 0)
        return;
    m_fields->spawnGroupSimple(y > 0 ? LevelKeys::wheelUp : LevelKeys::wheelDown);
}

void MyBaseLayer::nh_handleKeypress(LevelKeys key, bool down) {
    Log.d("gjbgl", "Handle key press");
    m_fields->spawnGroupKeys({key, down});
}


cocos2d::CCPoint MyBaseLayer::screenToGame(const cocos2d::CCPoint& screenPos) {
    auto cameraPos = m_gameState.m_cameraPosition;
    auto cameraScale = m_gameState.m_cameraZoom;
    auto cameraAngle = m_gameState.m_cameraAngle;

    // Rotate the position around the camera angle
    auto angle = cameraAngle * std::numbers::pi / 180;

    auto rotateVector = [](const cocos2d::CCPoint& vector, double angle) {
        auto x = vector.x * cos(angle) - vector.y * sin(angle);
        auto y = vector.x * sin(angle) + vector.y * cos(angle);
        return ccp(x, y);
    };

    auto rotatedPos = rotateVector(screenPos, angle);

    auto scaledPos = ccp(rotatedPos.x / cameraScale, rotatedPos.y / cameraScale);

    // Add the camera position
    auto point = ccp(cameraPos.x + scaledPos.x, cameraPos.y + scaledPos.y);
    return point;
}


void MyBaseLayer::spawnGroup(groupId id) {
    Log.i("gjbgl", "spawn group: {}", id);
    GJBaseGameLayer::spawnGroup(id, false, 0, gd::vector<int>(), 0, 0);
}


void MyBaseLayer::updateItemId(int itemId, int newValue) {
    m_effectManager->updateCountForItem(itemId, newValue);
    updateCounters(itemId, newValue);
}



class $modify(PlayLayer) {
    void resetLevel() {
        PlayLayer::resetLevel();
        scheduleOnce(schedule_selector(MyBaseLayer::spawnModLoadedGroups), 0);
    }



    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        auto uiwrapper = MyUIWrapper::create((MyBaseLayer*)this, nullptr);
        ((MyBaseLayer*)this)->m_fields->uiWrapper = uiwrapper;
        this->addChild(uiwrapper, 99);
        return true;
    }
};