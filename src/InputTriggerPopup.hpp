#pragma once

#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <unordered_map>
#include <vector>
#include <Geode/utils/cocos.hpp>
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "LevelKeys.hpp"
#include "TextParsing.hpp"
#include <reaction/reaction.h>


class InputTriggerPopup : public geode::Popup {
public:
    static InputTriggerPopup* create(TextGameObject* object);

protected:

    enum class Tab {
        Keyboard,
        Mouse,
        Touch, //(click action)
    };

    bool init(TextGameObject* textObject);
    void onClose(CCObject* sender) override;

    CCNode* createMouseToggler(const std::string& label, LevelKeys key);
    CCMenuItemToggler* createTabToggler(const std::string& label, Tab tab);
    CCMenuItemToggler* createKeyboardToggler(LevelKeys key, float width = 40, const std::string& labelOverride = "");
    CCMenu* createKeyboardMenu(float gap, float yOffset, int row);
    CCNode* createIntegerInput(const char* labelText, int* valuePtr, CCPoint position); 

    void setupKeyboardTab();
    void setupMouseTab();
    void setupTouchTab();

    struct KeyboardTabData {
        KeyAction m_keyAction;
        std::vector<geode::Ref<CCMenuItemToggler>> m_keyboardButtons;
        CCMenuItemToggler* m_onReleaseToggle;
        CCLabelBMFont* m_onReleaseLabel;
    };

    struct TouchTabData {
        ClickAction m_clickAction;
    };

    struct MouseTabData {
        SimpleKeyAction m_simpleKeyAction;
    };

    
    //this will get overriden later, but for the initialization it needs var()
    reaction::Var<Tab> m_currentActionTab = reaction::var(Tab::Keyboard);



    using TabSpecificData = std::variant<KeyboardTabData, MouseTabData, TouchTabData>;

    struct TabNodes {
        geode::Ref<CCMenuItemToggler> toggler;
        std::vector<geode::Ref<CCNode>> tabNodes;
    };
    struct TabData {
        TabNodes common;
        TabSpecificData specific;
    };

    KeyboardTabData& getKeyboardData() {
        return std::get<KeyboardTabData>(m_tabData[Tab::Keyboard].specific);
    }
    
    MouseTabData& getMouseData() {
        return std::get<MouseTabData>(m_tabData[Tab::Mouse].specific);
    }

    TouchTabData& getTouchData() {
        return std::get<TouchTabData>(m_tabData[Tab::Touch].specific);
    }

    std::unordered_map<Tab, TabData> m_tabData;

    geode::Ref<TextGameObject> m_object;
};
