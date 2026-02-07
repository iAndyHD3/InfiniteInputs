#include "GJBaseGameLayer.hpp"
#include <Geode/binding/EffectGameObject.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/TextGameObject.hpp>
#include <Geode/cocos/CCDirector.h>
#include <enchantum/enchantum.hpp>
#include <numbers>
#include <scn/scan.h>
#include "../LogVar.hpp"
#include "../TextParsing.hpp"
#include "Geode/loader/Loader.hpp"
#include "Geode/loader/Log.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/utils/cocos.hpp"
#include "LevelKeys.hpp"


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

void MyBaseLayer::Fields::addClickAction(EffectGameObject* collision, ClickAction action) {
    clickActionObjects.push_back({collision, std::move(action)});
    addedAtleastOneKey = true;
}


std::optional<groupId> MyBaseLayer::Fields::getGroupId(const KeyActionMapKey& key) {
    auto groupid = keyMap.find(key);
    return groupid != keyMap.end() ? std::optional<groupId>(groupid->second) : std::nullopt;
}


void MyBaseLayer::Fields::spawnGroupKeys(const KeyActionMapKey& key) {
    if (auto group = getGroupId(key)) {
        log::info("KEY: {}, {}, GROUP: {}", enchantum::to_string(key.key), key.keyDown ? "down" : "up", *group);
        layer->spawnGroup(*group, false, 0, {}, 0, 0);
    }
}

void MyBaseLayer::Fields::spawnGroupSimple(LevelKeys key) {
    if (auto group = simpleKeyMap.find(key); group != simpleKeyMap.end()) {
        log::info("[SIMPLE] KEY: {}, GROUP: {}", enchantum::to_string(key), group->second);
        layer->spawnGroup(group->second, false, 0, {}, 0, 0);
    }
}

$override bool MyBaseLayer::init() {
    if (!GJBaseGameLayer::init())
        return false;

    scheduleOnce(schedule_selector(MyBaseLayer::delayedInit), 0);
    return true;
}

$override void MyBaseLayer::update(float dt) {
    auto fields = m_fields.self();
    // LOGI(fields->shouldRunUpdateLoop);
    if (!isModActive()) {
        return GJBaseGameLayer::update(dt);
    }

    updateLoop(dt);

    GJBaseGameLayer::update(dt);
}


void MyBaseLayer::delayedInit(float) {

    if (!m_isEditor) {
        return scheduleOnce(schedule_selector(MyBaseLayer::setupKeybinds_step0), 0);
    }

    for (auto obj : CCArrayExt<GameObject*>(m_objects)) {
        if (obj->m_objectID != 914)
            continue;

        auto text = static_cast<TextGameObject*>(obj)->m_text;

        if (!foundOldFormatString(text))
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
                        auto text = label->m_text;
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
        log::info("stop playtest?");
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

    if (!fields->spawnedModLoaded) {
        fields->spawnGroupKeys({LevelKeys::modLoaded, false});
        fields->spawnedModLoaded = true;
    }
}

void MyBaseLayer::resetLevelVariables() {
    GJBaseGameLayer::resetLevelVariables();
    log::info("resetting");

    for (const auto& o : m_fields->cursorFollowObjects) {
        o->setLastPosition(o->getPosition());
    }
}

void MyBaseLayer::setupLevelStart(LevelSettingsObject* p0) {
    GJBaseGameLayer::setupLevelStart(p0);
    m_fields->spawnedModLoaded = false;
}

void MyBaseLayer::setupCursorGroup() {
    auto fields = m_fields.self();

    auto it = fields->simpleKeyMap.find(LevelKeys::cursor);
    if(it == fields->simpleKeyMap.end()) return;
    int cursorGroupId = it->second;


    log::info("SETTING UP CURSOR GROUP {}", cursorGroupId);
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
                    fields->keyMap.emplace(Fields::KeyActionMapKey{label->key, label->keyDown}, label->group);
                } else if (SimpleKeyAction* label = std::get_if<SimpleKeyAction>(&*parsed)) {
                    fields->simpleKeyMap.emplace(label->key, label->group);
                } else if (ClickAction* action = std::get_if<ClickAction>(&*parsed)) {
                    m_fields->clickActionAddQueue.push_back(std::move(*action));
                }
            } else {
                log::error("Failed to parse label: {}", static_cast<TextGameObject*>(obj)->m_text);
            }
        }
    }

    for (const auto& obj : m_objects->asExt<EffectGameObject*>()) {
        if (obj->m_objectID == 1816) {
            for (const auto& clickaction : fields->clickActionAddQueue) {
                if (clickaction.collisionBlockId == obj->m_itemID) {
                    fields->clickActionObjects.emplace_back(obj, std::move(clickaction));
                }
            }
        }
    }


    log::info("Added {} down keys", fields->keyMap.size());
    log::info("Added {} simple keys", fields->simpleKeyMap.size());
    log::info("Button Objects: {}", fields->clickActionObjects.size());
    log::info("Cursor Group: {}", fields->cursorFollowGroupId);
    //log::info("Wheel Up Group: {}", fields->wheelUpGroup);
    //log::info("Wheel Down Group: {}", fields->wheelDownGroup);

    fields->addedAtleastOneKey =
            !fields->keyMap.empty() || !fields->simpleKeyMap.empty() || !fields->clickActionObjects.empty();

    return fields->addedAtleastOneKey;
}

bool MyBaseLayer::isModActive() {
    if (!m_isEditor && !reinterpret_cast<PlayLayer*>(this)->m_started) {
        return false;
    }
    return m_fields->active;
}

void MyBaseLayer::setupKeybinds_step0(float) {
    if (!setupTextLabelKeys_step1()) {
        log::error("not parsed any labels");
        return;
    }


    auto fields = m_fields.self();

    fields->layer = this;

    setupCursorGroup();


    fields->active = true;
}


void MyBaseLayer::handleScroll(float x, float y) {
    if (y == 0)
        return;
    m_fields->spawnGroupSimple(y > 0 ? LevelKeys::wheelUp : LevelKeys::wheelDown);
}

void MyBaseLayer::nh_handleKeypress(LevelKeys key, bool down) {
    log::debug("Handle key press");
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
