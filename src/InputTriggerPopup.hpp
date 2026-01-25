#pragma once

#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/ui/TextInput.hpp>
#include <unordered_map>
#include <vector>
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "LevelKeys.hpp"
#include "TextParsing.hpp"

class InputTriggerPopup : public geode::Popup {
public:
    static InputTriggerPopup* create(std::vector<geode::Ref<GameObject>> objects);
protected:
    bool init(std::vector<geode::Ref<GameObject>> objects);
    void onClose(CCObject* sender) override;
    void setupValues();

    CCNode* createMouseToggler(const std::string& label, LevelKeys key);
    CCMenuItemToggler* createTabToggler(const std::string& label, int tab);
    CCMenuItemToggler* createKeyboardToggler(LevelKeys key, float width = 40, const std::string& labelOverride = "");
    CCMenu* createKeyboardMenu(float gap, float yOffset, int row);

    void setupKeyboardTab();
    void setupMouseTab();

    void checkSpecialKey();

    bool m_modifiedGroup = false;
    bool m_modifiedKey = false;
    bool m_modifiedRelease = false;

    ParsedTextLabel m_label;

    geode::TextInput* m_groupInput;
    CCMenuItemToggler* m_onModLoadedToggle;
    CCMenuItemToggler* m_onReleaseToggle;
    CCLabelBMFont* m_onReleaseLabel;
    std::unordered_map<geode::Ref<CCMenuItemToggler>, int> m_toggles;
    std::vector<geode::Ref<CCMenuItemToggler>> m_tabToggles;
    std::vector<geode::Ref<CCNode>> m_tabs;
    std::vector<geode::Ref<GameObject>> m_objects;
};