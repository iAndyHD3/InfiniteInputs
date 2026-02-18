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
        Touch,
    };

    bool init(TextGameObject* textObject);
    void onClose(CCObject* sender) override;

    CCNode* createMouseToggler(const std::string& label, LevelKeys key);
    CCMenuItemToggler* createTabToggler(const std::string& label, Tab tab);
    CCMenuItemToggler* createKeyboardToggler(LevelKeys key, float width = 40, const std::string& labelOverride = "");
    CCMenu* createKeyboardMenu(float gap, float yOffset, int row);
    CCNode* createIntegerInput(const char* labelText, int* valuePtr, CCPoint position);

    static void updateButtonPressed(CCMenuItemToggler** oldToggled, CCMenuItemToggler* newpressed);

    struct KeyboardTabData {
        KeyAction m_keyAction;
        std::vector<geode::Ref<CCMenuItemToggler>> m_keyboardButtons;
        CCMenuItemToggler* pressed = nullptr;
    };

    struct TouchTabData {
        ClickAction m_clickAction;
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
    TabToggler m_touchToggler;

    KeyboardTabData m_keyboardTabData;
    MouseTabData m_mouseTabData;
    TouchTabData m_touchTabData;

    KeyboardTabData& getKeyboardData() { return m_keyboardTabData; }

    MouseTabData& getMouseData() { return m_mouseTabData; }

    TouchTabData& getTouchData() { return m_touchTabData; }

    TabToggler& getToggler(Tab tab) {
        switch (tab) {
            case Tab::Keyboard: return m_keyboardToggler;
            case Tab::Mouse: return m_mouseToggler;
            case Tab::Touch: return m_touchToggler;
        }
    }

    geode::Ref<TextGameObject> m_object;
};
