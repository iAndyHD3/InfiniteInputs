#pragma once

#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/cocos.hpp>
#include <reaction/reaction.h>
#include <variant>
#include <vector>
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "LevelKeys.hpp"
#include "TextParsing.hpp"


class InputTriggerPopup : public geode::Popup {
public:
    static InputTriggerPopup* create(TextGameObject* object);
    static bool isSpecialMouseKeyboardKey(LevelKeys key);

protected:
    enum class Tab {
        Keyboard,
        Mouse,
        Button,
        Touch,
    };

    bool init(TextGameObject* textObject);
    void onClose(CCObject* sender) override;

    CCNode* createMouseToggler(const char* label, LevelKeys key, float buttonScale = 0.55f, float labelScale = 0.35f);
    CCMenuItemToggler* createTabToggler(const char* label, Tab tab);
    CCMenuItemToggler* createKeyboardToggler(LevelKeys key, float width = 40, const std::string& labelOverride = "");
    CCMenu* createKeyboardMenu(float gap, float yOffset, int row);
    CCNode* createIntegerInput(const char* labelText, int* valuePtr, CCPoint position);

    static void updateButtonPressed(CCMenuItemToggler** oldToggled, CCMenuItemToggler* newpressed);

    struct KeyboardTabData {
        KeyAction m_keyAction;
        std::vector<geode::Ref<CCMenuItemToggler>> m_keyboardButtons;
        CCMenuItemToggler* pressed = nullptr;
    };

    struct ButtonTabData {
        ClickAction m_clickAction;
    };

    struct TouchTabData {
        TouchAction m_touchAction;
    };

    struct MouseTabData {
        SimpleKeyAction m_simpleKeyAction;
        CCMenuItemToggler* pressed = nullptr;

        reaction::Var<LevelKeys> key = reaction::var(LevelKeys::empty);
        int group;
        bool keyDown = true;
    };

    reaction::Var<Tab> m_currentActionTab = reaction::var(Tab::Keyboard);

    struct TabToggler {
        geode::Ref<CCMenuItemToggler> toggler;
        std::vector<geode::Ref<CCNode>> tabNodes;
    };

    TabToggler m_keyboardToggler;
    TabToggler m_mouseToggler;
    TabToggler m_buttonToggler;
    TabToggler m_touchToggler;

    KeyboardTabData m_keyboardTabData;
    MouseTabData m_mouseTabData;
    ButtonTabData m_buttonTabData;
    TouchTabData m_touchTabData;

    KeyboardTabData& getKeyboardData() { return m_keyboardTabData; }

    MouseTabData& getMouseData() { return m_mouseTabData; }

    ButtonTabData& getButtonData() { return m_buttonTabData; }

    TouchTabData& getTouchData() { return m_touchTabData; }

    TabToggler& getToggler(Tab tab) {
        switch (tab) {
            case Tab::Keyboard: return m_keyboardToggler;
            case Tab::Mouse: return m_mouseToggler;
            case Tab::Button: return m_buttonToggler;
            case Tab::Touch: return m_touchToggler;
        }
    }

    geode::Ref<TextGameObject> m_object;
};
