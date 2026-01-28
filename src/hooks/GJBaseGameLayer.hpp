#pragma once
#include "../LevelKeys.hpp"
#include <Geode/binding/LevelEditorLayer.hpp>
#include "Geode/utils/cocos.hpp"
#include <Geode/modify/GJBaseGameLayer.hpp>

//#include <alphalaneous.alphas-ui-pack/include/touch/Touch.hpp>
//#include "alphalaneous.alphas-ui-pack/include/touch/TouchDispatcher.hpp"
// #include <alphalaneous.alphas-ui-pack/include/nodes/scroll/AdvancedScrollDelegate.hpp>
// #include <alphalaneous.alphas-ui-pack/include/nodes/scroll/ScrollDispatcher.hpp>


#include <unordered_map>
#include <vector>
#include <string_view>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

using groupId = int;

// struct MyClickDelegate : public CCNode, alpha::dispatcher::TouchDelegate {
//     bool clickBegan(alpha::dispatcher::TouchEvent* touch) override;
// 	void clickEnded(alpha::dispatcher::TouchEvent* touch) override;
// };

// struct MyScrollDelegate : public CCNode, alpha::dispatcher::AdvancedScrollDelegate {
//     void scroll(float x, float y) override;
// };

class $modify(MyBaseLayer, GJBaseGameLayer)
{
    struct Fields {
        std::unordered_map<LevelKeys, groupId> upKeyMap;
        std::unordered_map<LevelKeys, groupId> downKeyMap;
        //MyClickDelegate* touchDelegate = nullptr;
        //MyScrollDelegate* scrollDelegate = nullptr;

        bool spawnedModLoaded = false;
        bool active = false;
        bool addedAtleastOneKey = false;
        GJBaseGameLayer* layer = nullptr;
        int i = 0;

        //needs to be ccarray because moveObjects takes CCArray* and converting from vector to ccarray would be stupid
        CCArrayExt<GameObject*> cursorFollowObjects;

        /*SPECIAL ONLY ONE GROUP ID!!!*/
        int cursorFollowGroupId = -1;
        int wheelUpGroup = -1;
        int wheelDownGroup = -1;
        
        void clear();
        auto& getKeysMap(bool down);

        void addKeyBind(LevelKeys key, bool down, int groupId);

        std::optional<groupId> getGroupId(LevelKeys k, bool down);


        void spawnGroupIfDefined(LevelKeys k, bool down);

        ~Fields();
    };


    $override
    bool init();

    $override
    void update(float);


    void delayedInit(float);

    static std::vector<std::string_view> getAllTextsFromLabels(GJBaseGameLayer* pl);

    void editorActiveHandlerLoop(float);

    //void handleClick(alpha::dispatcher::TouchEvent* touch, bool down);

    void updateLoop(float);


    void resetLevelVariables();

    void setupLevelStart(LevelSettingsObject* p0);

    void setupText(std::string_view t);

    void setupCursorGroup(int cursorGroupId);

    //true if correctly registered atleast one keybind
    bool setupTextLabelKeys_step1();

    bool isModActive();

    void setupKeybinds_step0(float);

    void handleScroll(float x, float y);

    void nh_handleKeypress(LevelKeys key, bool down);

    cocos2d::CCPoint screenToGame(const cocos2d::CCPoint& screenPos);

};