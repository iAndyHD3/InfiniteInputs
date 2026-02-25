#include "GJBaseGameLayer.hpp"
#include <Geode/binding/EffectGameObject.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/TextGameObject.hpp>
#include <Geode/cocos/CCDirector.h>
#include <arc/prelude.hpp>
#include <arc/runtime/Runtime.hpp>
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


std::optional<groupId> MyBaseLayer::Fields::getGroupId(const KeyActionMapKey& key) {
    auto groupid = keyMap.find(key);
    return groupid != keyMap.end() ? std::optional<groupId>(groupid->second) : std::nullopt;
}


void MyBaseLayer::Fields::spawnGroupKeys(const KeyActionMapKey& key) {
    if (auto group = getGroupId(key)) {
        Log.i("gjbgl", "KEY: {}, {}, GROUP: {}", enchantum::to_string(key.key), key.keyDown ? "down" : "up", *group);
        layer->spawnGroup(*group);
    }
}

void MyBaseLayer::Fields::spawnGroupSimple(LevelKeys key) {
    if (auto group = simpleKeyMap.find(key); group != simpleKeyMap.end()) {
        Log.i("gjbgl", "[SIMPLE] KEY: {}, GROUP: {}", enchantum::to_string(key), group->second);
        layer->spawnGroup(group->second);
    } else {
        Log.e("gjbl", "Could not find group to spawn on key: {}, size: {}", ETOSTRING(key), simpleKeyMap.size());
    }
}

$override bool MyBaseLayer::init() {
    if (!GJBaseGameLayer::init())
        return false;

    scheduleOnce(schedule_selector(MyBaseLayer::delayedInit), 0);
    return true;
}

$override void MyBaseLayer::update(float dt) {
    // LOGI(fields->shouldRunUpdateLoop);
    GJBaseGameLayer::update(dt);
}


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
        fields->active = true;
        setupKeybinds_step0(0);
    }
    // stop playtest
    else if (fields->active && !inPlaytest) {
        Log.i("gjbgl", "stop playtest? resetting all fields!");
        // default values of all fields again (cleared)

        *fields = MyBaseLayer::Fields();
    }
}

void MyBaseLayer::updateLoop(float) {


    auto fields = m_fields.self();

    for (const auto& o : fields->cursorFollowObjects) {
        // LOGI(o->m_objectID, o->m_isUIObject, o->getRealPosition());

        CCPoint layerObjPos;
        CCPoint layerMousePos;

        if (o->m_isUIObject) {
            layerObjPos = o->getRealPosition();
            layerMousePos = getMousePos();
        } else {
            layerObjPos = this->convertToNodeSpace(o->getRealPosition());
            layerMousePos = screenToGame(getMousePos());
        }

        auto delta = layerMousePos - layerObjPos;
        moveObject(o, delta.x, delta.y, false);
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
        o->setLastPosition(o->getPosition());
    }
}

void MyBaseLayer::spawnModLoadedGroups(float) {

    Log.i("gjbgl", "Spawning mod load groupsm started: {}", m_started);
    auto fields = m_fields.self();
    fields->spawnGroupSimple(LevelKeys::modLoaded);

#if defined(GEODE_IS_DESKTOP)
    fields->spawnGroupSimple(LevelKeys::modLoadedPC);
#elif defined(GEODE_IS_MOBILE)
    fields->spawnGroupSimple(LevelKeys::modLoadedMobile);
#endif
}

void MyBaseLayer::setupCursorGroup() {
    auto fields = m_fields.self();

    auto it = fields->simpleKeyMap.find(LevelKeys::cursor);
    if (it == fields->simpleKeyMap.end())
        return;
    int cursorGroupId = it->second;


    Log.i("gjbgl", "SETTING UP CURSOR GROUP {}", cursorGroupId);
    fields->cursorFollowGroupId = cursorGroupId;
    fields->cursorFollowObjects.clear();
    for (const auto& o : CCArrayExt<GameObject*>(m_objects)) {
        if (hasGroup(o, cursorGroupId)) {
            fields->cursorFollowObjects.push_back(o);
        }
    }
}


// true if correctly registered atleast one keybind
bool MyBaseLayer::setupTextLabelKeys_step1() {
    auto fields = m_fields.self();

    // parse all labels
    for (const auto& obj : m_objects->asExt<GameObject*>()) {
        if (obj->m_objectID == 914) {
            std::string_view t = static_cast<TextGameObject*>(obj)->m_text;
            if (auto parsed = parseObjectString(t)) {
                if (KeyAction* label = std::get_if<KeyAction>(&*parsed)) {
                    fields->keyMap.emplace(KeyActionMapKey{label->key, label->keyDown}, label->group);
                } else if (SimpleKeyAction* label = std::get_if<SimpleKeyAction>(&*parsed)) {
                    fields->simpleKeyMap.emplace(label->key, label->group);
                } else if (ClickAction* action = std::get_if<ClickAction>(&*parsed)) {
                    m_fields->clickActionAddQueue.push_back(std::move(*action));
                }
            } else {
                Log.e("gjbgl", "Failed to parse label: {}", static_cast<TextGameObject*>(obj)->m_text);
            }
        }
    }

    for (const auto& obj : m_objects->asExt<CollisionBlock*>()) {
        if (obj->m_objectID == 1816) {
            for (const auto& clickaction : fields->clickActionAddQueue) {
                if (clickaction.collisionBlockId == obj->m_itemID) {
                    // Log.i("gjbgl", "{}", obj);
                    Log.i("gjbgl", "{}", obj);
                    fields->clickActions.emplace_back(obj, std::move(clickaction));
                }
            }
        }
    }
    fields->clickActionAddQueue.clear();


    Log.i("gjbgl", "Added {} down keys", fields->keyMap.size());
    Log.i("gjbgl", "Added {} simple keys", fields->simpleKeyMap.size());
    Log.i("gjbgl", "Added {} click keys", fields->clickActions.size());
    Log.i("gjbgl", "Button Objects: {}", fields->clickActions.size());
    Log.i("gjbgl", "Cursor Group: {}", fields->cursorFollowGroupId);
    // Log.i("gjbgl", "Wheel Up Group: {}", fields->wheelUpGroup);
    // Log.i("gjbgl", "Wheel Down Group: {}", fields->wheelDownGroup);

    fields->addedAtleastOneKey =
            !fields->keyMap.empty() || !fields->simpleKeyMap.empty() || !fields->clickActions.empty();

    return fields->addedAtleastOneKey;
}

bool MyBaseLayer::isModActive() { return m_fields->active; }

void MyBaseLayer::setupKeybinds_step0(float) {
    if (!setupTextLabelKeys_step1()) {
        Log.e("gjbgl", "not parsed any labels");
        return;
    }


    auto fields = m_fields.self();

    fields->layer = this;

    setupCursorGroup();

    fields->active = true;

    schedule(schedule_selector(MyBaseLayer::updateLoop));

    if (m_isEditor) {
        scheduleOnce(schedule_selector(MyBaseLayer::spawnModLoadedGroups), 0);
    }
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

class $modify(PlayLayer) {
    void resetLevel() {
        PlayLayer::resetLevel();
        scheduleOnce(schedule_selector(MyBaseLayer::spawnModLoadedGroups), 0);
    }
};
