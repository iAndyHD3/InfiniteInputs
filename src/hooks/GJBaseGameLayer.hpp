#pragma once
#include <Geode/binding/EffectGameObject.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../LevelKeys.hpp"
#include "Geode/utils/cocos.hpp"


// #include <alphalaneous.alphas-ui-pack/include/touch/Touch.hpp>
// #include "alphalaneous.alphas-ui-pack/include/touch/TouchDispatcher.hpp"
//  #include <alphalaneous.alphas-ui-pack/include/nodes/scroll/AdvancedScrollDelegate.hpp>
//  #include <alphalaneous.alphas-ui-pack/include/nodes/scroll/ScrollDispatcher.hpp>


#include <Geode/modify/GJBaseGameLayer.hpp>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "../TextParsing.hpp"


using namespace geode::prelude;

using groupId = int;

// struct MyClickDelegate : public CCNode, alpha::dispatcher::TouchDelegate {
//     bool clickBegan(alpha::dispatcher::TouchEvent* touch) override;
// 	void clickEnded(alpha::dispatcher::TouchEvent* touch) override;
// };

// struct MyScrollDelegate : public CCNode, alpha::dispatcher::AdvancedScrollDelegate {
//     void scroll(float x, float y) override;
// };


class $modify(MyBaseLayer, GJBaseGameLayer) {
    struct Fields {

        struct KeyActionMapKey {
            LevelKeys key;
            bool keyDown;

            bool operator==(const KeyActionMapKey& other) const { return key == other.key && keyDown == other.keyDown; }
        };

        struct KeyActionMapKeyHash {
            std::size_t operator()(const KeyActionMapKey& k) const {
                return std::hash<LevelKeys>()(k.key) ^ std::hash<bool>()(k.keyDown);
            }
        };

        std::unordered_map<KeyActionMapKey, groupId, KeyActionMapKeyHash> keyMap;

        // SIMPLE KEY MAP: wheelUp, wheelDown, cursorFollow
        std::unordered_map<LevelKeys, groupId> simpleKeyMap;

        std::vector<std::pair<GameObject*, ClickAction>> clickActionObjects;

        // only used as queue during initialization
        std::vector<ClickAction> clickActionAddQueue;

        bool spawnedModLoaded = false;
        bool active = false;
        bool addedAtleastOneKey = false;
        bool oldFormatFound = false;

        GJBaseGameLayer* layer = nullptr;

        std::vector<GameObject*> cursorFollowObjects;

        /*SPECIAL ONLY ONE GROUP ID!!!*/
        int cursorFollowGroupId = -1;


        void addKeyBind(LevelKeys key, bool down, int groupId);
        void addClickAction(EffectGameObject* collision, ClickAction action);

        // TODO: unify this with overloads or something
        std::optional<groupId> getGroupId(const KeyActionMapKey&);


        void spawnGroupKeys(const KeyActionMapKey&);
        void spawnGroupSimple(LevelKeys key);
    };


    $override bool init();

    $override void update(float);


    void delayedInit(float);

    static std::vector<std::string_view> getAllTextsFromLabels(GJBaseGameLayer* pl);

    void editorActiveHandlerLoop(float);

    // void handleClick(alpha::dispatcher::TouchEvent* touch, bool down);

    void updateLoop(float);


    void resetLevelVariables();

    void setupLevelStart(LevelSettingsObject* p0);

    void setupText(std::string_view t);

    // TODO: unify all of this
    void setupCursorGroup();

    // true if correctly registered (TODO: or will register) atleast one keybind
    bool setupTextLabelKeys_step1();

    // TODO: check this
    bool isModActive();

    void setupKeybinds_step0(float);

    void handleScroll(float x, float y);

    void nh_handleKeypress(LevelKeys key, bool down);

    cocos2d::CCPoint screenToGame(const cocos2d::CCPoint& screenPos);
};
