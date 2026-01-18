#include "GJBaseGameLayer.hpp"
#include "../TextParsing.hpp"
#include "../LogVar.hpp"
#include "Geode/utils/cocos.hpp"
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <scn/scan.h>
#include <enchantum/enchantum.hpp>
#include <Geode/cocos/CCDirector.h>
#include <numbers>


#define ETOSTRING(k) enchantum::to_string(k)

static gd::vector<short> getGroupIDs(GameObject* obj) {
    gd::vector<short> res;

    if (obj->m_groups && obj->m_groups->at(0))
        for (auto i = 0; i < obj->m_groupCount; i++)
            res.push_back(obj->m_groups->at(i));
    return res;
}

static bool hasGroup(GameObject* obj, int group)
{
    for (const auto& g : getGroupIDs(obj))
    {
        if (g == group)
            return true;
    }
    return false;
}

void MyBaseLayer::Fields::clear() {
    upKeyMap.clear();
    downKeyMap.clear();
    if(touchDelegate) {
        CCTouchDispatcher::get()->removeDelegate(touchDelegate);
        touchDelegate = nullptr;
    }
    if(scrollDelegate) {
        alpha::dispatcher::ScrollDispatcher::get()->unregisterScroll(scrollDelegate);
        scrollDelegate = nullptr;
    }
    controlPressed = false;
    altPressed = false;
    shiftPressed = false;
    active = false;
    layer = nullptr;
    //no clear() in Ext
    cursorFollowObjects.inner()->removeAllObjects();
    cursorFollowGroupId = -1;
    wheelUpGroup = -1;
    wheelDownGroup = -1;
}

auto& MyBaseLayer::Fields::getKeysMap(bool down) {
    return down ? downKeyMap : upKeyMap;
}

void MyBaseLayer::Fields::addKeyBind(LevelKeys key, bool down, int groupId) {

    switch(key) {
        case LevelKeys::cursor:     cursorFollowGroupId =   groupId; break;
        case LevelKeys::wheelUp:    wheelUpGroup        =   groupId; break;
        case LevelKeys::wheelDown:  wheelDownGroup      =   groupId; break;

        default: getKeysMap(down).emplace(key, groupId);
    }
    addedAtleastOneKey = true;
}

std::optional<groupId> MyBaseLayer::Fields::getGroupId(LevelKeys k, bool down) {
    log::debug("FINDING: {}", ETOSTRING(k));
    if(k == LevelKeys::unknown) return std::nullopt;

    auto map = getKeysMap(down);
    auto it = map.find(k);
    if(it != map.end()) {
        log::debug("{} found in map", enchantum::to_string(k));
        return it->second;
    }

    switch(k) {
        case LevelKeys::cursor: return cursorFollowGroupId;
        case LevelKeys::wheelDown: return wheelDownGroup;
        case LevelKeys::wheelUp: return wheelUpGroup;
        default: return std::nullopt;
    }
}




void MyBaseLayer::Fields::spawnGroupIfDefined(LevelKeys k, bool down) {
    if(auto group = getGroupId(k, down)) {
        log::info("KEY: {}, {}, GROUP: {}", enchantum::to_string(k), down ? "down" : "up", *group);
        layer->spawnGroup(*group, false, 0, {}, 0, 0);
        return;
    }
}

MyBaseLayer::Fields::~Fields() {
    if(touchDelegate) {
        CCTouchDispatcher::get()->removeDelegate(touchDelegate);
    }
    if(scrollDelegate) {
        alpha::dispatcher::ScrollDispatcher::get()->unregisterScroll(scrollDelegate);
    }
}

$override
bool MyBaseLayer::init() {
    if (!GJBaseGameLayer::init()) return false;

    scheduleOnce(schedule_selector(MyBaseLayer::delayedInit), 0);
    return true;
}

$override
void MyBaseLayer::update(float dt)
{
    auto fields = m_fields.self();
    //LOGI(fields->shouldRunUpdateLoop);
    if(!isModActive()) {
        return GJBaseGameLayer::update(dt);
    }

    updateLoop(dt);

    GJBaseGameLayer::update(dt);
}


void MyBaseLayer::delayedInit(float) {
    if(m_isEditor) {
        schedule(schedule_selector(MyBaseLayer::editorActiveHandlerLoop), 0);
        return;
    }

    scheduleOnce(schedule_selector(MyBaseLayer::setupKeybinds_step0), 0);

    //start directly in playlayer
}

std::vector<std::string_view> MyBaseLayer::getAllTextsFromLabels(GJBaseGameLayer* pl) {
    std::vector<std::string_view> texts;

    for(const auto& obj : CCArrayExt<GameObject*>(pl->m_objects)) {
        if(obj->m_objectID != 914) continue;
        texts.emplace_back(static_cast<TextGameObject*>(obj)->m_text);
    }
    return texts;
}

void MyBaseLayer::editorActiveHandlerLoop(float) {
    auto fields = m_fields.self();
    auto editor = reinterpret_cast<LevelEditorLayer*>(this);
    bool inPlaytest = m_playbackMode == PlaybackMode::Playing;

    if(!fields->active && inPlaytest) {
        fields->active = true;
        setupKeybinds_step0(0);
    }
    else if(fields->active && !inPlaytest) {
        fields->active = false;
        fields->clear();
    }
}

void MyBaseLayer::handleClick(alpha::dispatcher::TouchEvent* touch, bool down) {
    LevelKeys btn = AlphaMouseButtonToLevelKeys(touch->getButton());
    log::info("{} {}", enchantum::to_string(touch->getButton()), enchantum::to_string(btn));
    nh_handleKeypress(btn, down);
}

void MyBaseLayer::updateLoop(float) {
    auto kb = CCDirector::get()->getKeyboardDispatcher();
    auto fields = m_fields.self();
    bool control = kb->getControlKeyPressed();
    if(control != fields->controlPressed) {
        fields->controlPressed = control;
        nh_handleKeypress(LevelKeys::leftCtrl, control);
    }
    bool shift = kb->getShiftKeyPressed();
    if(shift != fields->shiftPressed) {
        fields->shiftPressed = shift;
        nh_handleKeypress(LevelKeys::leftShift, shift);
    }
    bool alt = kb->getAltKeyPressed();
    if(alt != fields->altPressed) {
        fields->altPressed = alt;
        nh_handleKeypress(LevelKeys::leftAlt, alt);
    }

    for(const auto& o : fields->cursorFollowObjects) {
        //LOGI(o->m_objectID, o->m_isUIObject, o->getRealPosition());

        CCPoint layerObjPos;
        CCPoint layerMousePos;

        if(o->m_isUIObject) {
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
        fields->spawnGroupIfDefined(LevelKeys::modLoaded, false);
        fields->spawnedModLoaded = true;
    }
}

void MyBaseLayer::resetLevelVariables() {
    GJBaseGameLayer::resetLevelVariables();
    log::info("resetting");

    for(const auto& o : m_fields->cursorFollowObjects) {
        o->setLastPosition(o->getPosition());
    }
}

void MyBaseLayer::setupLevelStart(LevelSettingsObject* p0) {
    GJBaseGameLayer::setupLevelStart(p0);
    m_fields->spawnedModLoaded = false;
}

void MyBaseLayer::setupText(std::string_view t) {
    log::debug("parsing {}", t);
    if(!t.starts_with("inf_inp:")) return;
    auto parsed_opt = getTupleFromLabel(t);
    if(!parsed_opt) {
        return;
    }
    auto& parsed = *parsed_opt;
    m_fields->addKeyBind(parsed.key, parsed.keyDown, parsed.group);
}

void MyBaseLayer::setupCursorGroup(int cursorGroupId) {
    log::info("SETTING UP CURSOR GROUP {}", cursorGroupId);
    m_fields->cursorFollowGroupId = cursorGroupId;
    m_fields->cursorFollowObjects.inner()->removeAllObjects();
    for(const auto& o : CCArrayExt<GameObject*>(m_objects)) {
        if(hasGroup(o, cursorGroupId)) {
            m_fields->cursorFollowObjects.push_back(o);
        }
    }
}

//true if correctly registered atleast one keybind
bool MyBaseLayer::setupTextLabelKeys_step1() {
    auto fields = m_fields.self();
    auto texts = getAllTextsFromLabels(this);
    std::ranges::for_each(texts, [this](auto& t) {setupText(t); });

    log::info("Added {} down keys", fields->downKeyMap.size());
    log::info("Added {} up keys", fields->upKeyMap.size());
    log::info("Cursor Group: {}", fields->cursorFollowGroupId);
    log::info("Wheel Up Group: {}", fields->wheelUpGroup);
    log::info("Wheel Down Group: {}", fields->wheelDownGroup);
    return fields->addedAtleastOneKey;
}

bool MyBaseLayer::isModActive() {
    if(!m_isEditor && !reinterpret_cast<PlayLayer*>(this)->m_started) {
        return false;
    }
    return m_fields->active;
}

void MyBaseLayer::setupKeybinds_step0(float) {
    if(!setupTextLabelKeys_step1()) {
        log::error("not parsed any labels");
        return;
    }
    auto fields = m_fields.self();
    fields->layer = this;

    if(!fields->touchDelegate) {
        fields->touchDelegate = new MyClickDelegate;
        fields->touchDelegate->autorelease();
        fields->touchDelegate->setID("iandyhd.keyboardsupport/touch-delegate");
        fields->touchDelegate->setContentSize(this->getContentSize());
        addChild(fields->touchDelegate, INT_MAX);
        CCTouchDispatcher::get()->addTargetedDelegate(fields->touchDelegate, INT_MIN, false);
    }

    if(!fields->scrollDelegate) {
        fields->scrollDelegate = new MyScrollDelegate;
        fields->scrollDelegate->autorelease();
        fields->scrollDelegate->setID("iandyhd.keyboardsupport/scroll-delegate");
        fields->scrollDelegate->setContentSize(this->getContentSize());
        addChild(fields->scrollDelegate, INT_MAX);
        alpha::dispatcher::ScrollDispatcher::get()->registerScroll(fields->scrollDelegate);
    }
    if(fields->cursorFollowGroupId != -1) {
        setupCursorGroup(fields->cursorFollowGroupId);
    }

    fields->active = true;
}



void MyBaseLayer::handleScroll(float x, float y) {
    if(y != 0) {
        //scrolling up is minus apparently
        nh_handleKeypress(y < 0 ? LevelKeys::wheelUp : LevelKeys::wheelDown, /*this is ignored*/ false);
    }
}

void MyBaseLayer::nh_handleKeypress(LevelKeys key, bool down) {
    log::debug("Handle key press");
    m_fields->spawnGroupIfDefined(key, down);
}

bool MyClickDelegate::clickBegan(alpha::dispatcher::TouchEvent *touch) {
    auto bl = reinterpret_cast<MyBaseLayer*>(GJBaseGameLayer::get());
    if(!bl->isModActive()) return false;
    log::debug("click began {}", enchantum::to_string(touch->getButton()));
    bl->handleClick(touch, true);
    return true;
}

void MyClickDelegate::clickEnded(alpha::dispatcher::TouchEvent* touch) {
    log::debug("click ended {}", enchantum::to_string(touch->getButton()));
    reinterpret_cast<MyBaseLayer*>(GJBaseGameLayer::get())->handleClick(touch, false);
}

void MyScrollDelegate::scroll(float x, float y) {
    auto bl = reinterpret_cast<MyBaseLayer*>(GJBaseGameLayer::get());
    if(!bl->isModActive()) return;
    log::info("scroll {}, {}", x, y);
    bl->handleScroll(x, y);
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
