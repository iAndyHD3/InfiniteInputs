#pragma once

#include <Geode/binding/GJBaseGameLayer.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <string_view>
#include <vector>

#include <Geode/binding/EffectGameObject.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../LevelKeys.hpp"
#include "../TextParsing.hpp"


using namespace geode::prelude;

using groupId = int;
using CollisionBlock = EffectGameObject;

struct KeyActionMapKey {
    LevelKeys key;
    bool keyDown;
    bool operator==(const KeyActionMapKey& other) const { return key == other.key && keyDown == other.keyDown; }
};

namespace boost {
    template <>
    struct hash<ClickAction> {
        std::size_t operator()(const ClickAction& k) const {
            auto hasher = boost::hash<int>();
            return hasher(k.collisionBlockId) ^ hasher(k.groupIdCursorEnter) ^ hasher(k.groupIdCursorExit) ^
                   hasher(k.groupIdCursorUp);
        }
    };

    template <>
    struct hash<KeyActionMapKey> {
        std::size_t operator()(const KeyActionMapKey& k) const {
            return boost::hash<LevelKeys>()(k.key) ^ boost::hash<bool>()(k.keyDown);
        }
    };
} // namespace boost

struct ClickActionData {
    CollisionBlock* collblock = nullptr;
    ClickAction action;
    bool taken = false;
    bool calledEnter = true;
    bool calledExit = false;
};

class $modify(MyBaseLayer, GJBaseGameLayer) {
    using GJBaseGameLayer::spawnGroup;
    struct Fields {

        boost::unordered_flat_map<KeyActionMapKey, groupId> keyMap;

        // SIMPLE KEY MAP: wheelUp, wheelDown, cursorFollow
        boost::unordered_flat_map<LevelKeys, groupId> simpleKeyMap;

        // only used as queue during initialization
        std::vector<ClickActionData> clickActions;

        bool spawnedModLoaded = false;
        bool spawnedModLoadedPC = false;
        bool spawnedModLoadedMobile = false;
        bool active = false;
        bool addedAtleastOneKey = false;
        bool oldFormatFound = false;
        bool appliedWindowSize = false;

        // cache
        MyBaseLayer* layer = nullptr;

        std::vector<GameObject*> cursorFollowObjects;

        /*SPECIAL ONLY ONE GROUP ID!!!*/
        int cursorFollowGroupId = -1;

        std::vector<ClickAction> clickActionAddQueue;

        CCPoint lastMousePos;
        bool shouldStopUpdatingMousePos = false;


        void addKeyBind(LevelKeys key, bool down, int groupId);
        void addClickAction(CollisionBlock* collision, ClickAction action);

        std::optional<groupId> getGroupId(const KeyActionMapKey&);

        void spawnGroupKeys(const KeyActionMapKey&);
        void spawnGroupSimple(LevelKeys key);
        void updateItemIdWithSimpleKey(GJBaseGameLayer* layer, LevelKeys key, int itemId);

        bool hasAnyMouseKeyActive();
    };


    $override bool init();

    //$override void update(float);


    // void sortSectionVector();
    void delayedInit(float);

    static std::vector<std::string_view> getAllTextsFromLabels(GJBaseGameLayer* pl);

    void editorActiveHandlerLoop(float);

    // void handleClick(alpha::dispatcher::TouchEvent* touch, bool down);

    void updateLoop(float);

    void updateMouseDeltaKeys(float);

    void spawnModLoadedGroups(float);


    void resetLevelVariables();

    // void setupLevelStart(LevelSettingsObject* p0);

    void setupText(std::string_view t);

    void setupCursorGroup();

    // true if correctly registered (TODO: or will register) atleast one keybind
    bool setupTextLabelKeys_step1();

    bool isModActive();

    void setupKeybinds_step0(float);

    void handleScroll(float x, float y);

    void nh_handleKeypress(LevelKeys key, bool down);

    cocos2d::CCPoint screenToGame(const cocos2d::CCPoint& screenPos);

    void spawnGroup(groupId id);
};
