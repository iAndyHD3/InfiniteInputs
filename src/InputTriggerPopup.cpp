#include "InputTriggerPopup.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/TextGameObject.hpp>
#include <algorithm>
#include <enchantum/enchantum.hpp>
#include <fmt/format.h>
#include "Geode/cocos/cocoa/CCGeometry.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/ui/Popup.hpp"
#include "Geode/utils/ZStringView.hpp"
#include "TextParsing.hpp"

using namespace geode::prelude;

InputTriggerPopup* InputTriggerPopup::create(TextGameObject* object) {
    auto ret = new InputTriggerPopup();
    if (ret && ret->init(object)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}


bool InputTriggerPopup::init(TextGameObject* object) {
    if (!Popup::init(440.f, 310.f)) {
        return false;
    }

    log::info("text: {}", object->m_text);

    m_noElasticity = true;
    m_object = object;
    m_closeBtn->removeFromParent();

    auto title = CCLabelBMFont::create("Input Settings", "bigFont.fnt");
    title->setScale(0.5f);
    title->setPosition({m_mainLayer->getContentWidth() * 0.5f, m_mainLayer->getContentHeight() - 14});
    title->setID("title-label");

    m_mainLayer->addChild(title);
    m_mainLayer->setID("main-layer");

    auto helpBtn =
            CCMenuItemExt::createSpriteExtraWithFrameName("GJ_infoIcon_001.png", 1.f, [](CCMenuItemSpriteExtra* self) {
                geode::createQuickPopup(
                        "Help",
                        R"(Activates a <cg>Group ID</c> based on a <cy>key</c> or <cy>mouse</c> action. This trigger works similarly to the UI trigger; it is processed at the start of the level, and cannot be spawned or toggled.
<cb>On Mod Loaded</c> activates the group if this mod is installed.
<cp>Trigger On Release</c> activates the group on key or mouse release instead of press.
<co>Lock Cursor</c> locks the group to the cursor position instead of activating it.)",
                        "OK", nullptr, nullptr);
            });
    helpBtn->setID("info-button");

    helpBtn->setPosition({18, m_mainLayer->getContentHeight() - 18});

    m_buttonMenu->addChild(helpBtn);
    m_buttonMenu->setID("button-menu");

    auto okSpr = ButtonSprite::create("OK", 40, 0, 0.8, true, "goldFont.fnt", "GJ_button_01.png", 30.0);
    auto okBtn = CCMenuItemSpriteExtra::create(okSpr, this, menu_selector(InputTriggerPopup::onClose));
    okBtn->setPosition({m_mainLayer->getContentWidth() * 0.5f, 24});
    okBtn->setID("ok-button");

    m_buttonMenu->addChild(okBtn);

    auto tabsMenu = CCMenu::create();
    tabsMenu->ignoreAnchorPointForPosition(false);
    tabsMenu->setAnchorPoint({0.5f, 1.f});
    tabsMenu->setContentSize({m_mainLayer->getContentWidth(), 40});
    tabsMenu->setPosition({m_mainLayer->getContentWidth() * 0.5f, m_mainLayer->getContentHeight() - 32});
    tabsMenu->setID("tabs-menu");

    auto tabsLayout = RowLayout::create();

    tabsMenu->setLayout(tabsLayout);

    m_mainLayer->addChild(tabsMenu);

    tabsMenu->addChild(createTabToggler("Keyboard", Tab::Keyboard));
    tabsMenu->addChild(createTabToggler("Mouse", Tab::Mouse));
    tabsMenu->addChild(createTabToggler("Touch", Tab::Touch));

    tabsMenu->updateLayout();

    setupKeyboardTab();
    setupMouseTab();
    setupTouchTab();

    reaction::action([this](){
        Tab toggledTab = m_currentActionTab();
        log::info("tab: {}", (int)toggledTab);
        for(const auto& [tabEnum, data] : m_tabData) {
            //ADD NODES AND HIDE!!!!!!!!!!
            log::info("{}", data.common.nodeContainer);
            data.common.nodeContainer->setVisible(false);
            if(toggledTab != tabEnum) {
                data.common.toggler->toggle(false);
                data.common.toggler->setEnabled(true);
            }
        }
        m_tabData[toggledTab].common.nodeContainer->setVisible(true);

        auto& toggler = m_tabData[toggledTab].common.toggler;
        log::info("clickable false on: {}", toggler);
        toggler->setEnabled(false);
        toggler->toggle(true);
    });

    if (auto parsed = KeyAction::parse(object->m_text)) {
        m_tabData[Tab::Keyboard].specific = KeyboardTabData{.m_keyAction = std::move(*parsed)};
        m_currentActionTab.value(Tab::Keyboard);
    } else if (auto parsed = SimpleKeyAction::parse(object->m_text)) {
        m_tabData[Tab::Mouse].specific = MouseTabData{.m_simpleKeyAction = std::move(*parsed)};
        m_currentActionTab.value(Tab::Mouse);
    } else if (auto parsed = ClickAction::parse(object->m_text)) {
        m_tabData[Tab::Touch].specific = TouchTabData{.m_clickAction = std::move(*parsed)};
        m_currentActionTab.value(Tab::Touch);
    }
    else {
        m_currentActionTab.value(Tab::Keyboard);
        m_tabData[Tab::Keyboard].specific = KeyboardTabData{};
    }



    // m_onModLoadedToggle = CCMenuItemExt::createTogglerWithStandardSprites(0.7f, [this](CCMenuItemToggler* toggler) {
    //     // for (const auto& [toggle, tab] : m_toggles) {
    //     //     if (toggle == toggler)
    //     //         continue;
    //     //     toggle->toggle(false);
    //     // }
    //     // m_label.key = !toggler->isToggled() ? LevelKeys::modLoaded : LevelKeys::empty;
    //     // m_modifiedKey = true;
    // });

    // m_onModLoadedToggle->setPosition({m_mainLayer->getContentWidth() - 120, 25});

    // m_onModLoadedToggle->setUserObject("key"_spr, ObjWrapper<LevelKeys>::create(LevelKeys::modLoaded));
    // m_onModLoadedToggle->setID("on-mod-loaded-toggle");
    // m_toggles[m_onModLoadedToggle] = 0;

    // m_buttonMenu->addChild(m_onModLoadedToggle);

    // auto onModLoadedLabel = CCLabelBMFont::create("On Mod\nLoaded", "bigFont.fnt");
    // onModLoadedLabel->setScale(0.35f);
    // onModLoadedLabel->setAnchorPoint({0.f, 0.5f});
    // onModLoadedLabel->setPosition(
    //         {m_onModLoadedToggle->getPositionX() + m_onModLoadedToggle->getContentWidth() * 0.5f + 10,
    //          m_onModLoadedToggle->getPositionY()});
    // onModLoadedLabel->setID("on-mod-loaded-label");

    // m_mainLayer->addChild(onModLoadedLabel);


    return true;
}


CCNode* InputTriggerPopup::createMouseToggler(const std::string& label, LevelKeys key) {
    auto checkboxContainer = CCNode::create();
    checkboxContainer->ignoreAnchorPointForPosition(false);
    checkboxContainer->setAnchorPoint({0.f, 0.5f});
    checkboxContainer->setID("checkbox-container");

    auto checkboxMenu = CCMenu::create();
    checkboxMenu->ignoreAnchorPointForPosition(false);
    checkboxMenu->setAnchorPoint({0.f, 0.5f});
    checkboxMenu->setID("checkbox-menu");

    auto toggler = CCMenuItemExt::createTogglerWithStandardSprites(
            0.7f, [this, key](CCMenuItemToggler* self) { log::info("mouse toggle"); });


    checkboxMenu->addChild(toggler);
    checkboxMenu->setContentSize(toggler->getContentSize());

    checkboxContainer->setContentHeight(checkboxMenu->getContentHeight());
    checkboxMenu->setPosition({0.f, checkboxContainer->getContentHeight() * 0.5f});

    toggler->setPosition(checkboxMenu->getContentSize() * 0.5f);

    checkboxContainer->addChild(checkboxMenu);

    auto checkboxLabel = CCLabelBMFont::create(label.c_str(), "bigFont.fnt");

    checkboxLabel->setScale(0.35f);
    checkboxLabel->setAnchorPoint({0.f, 0.5f});
    checkboxLabel->setPosition({checkboxMenu->getContentWidth() + 10.f, checkboxContainer->getContentHeight() * 0.5f});

    checkboxContainer->addChild(checkboxLabel);

    checkboxContainer->setContentWidth(
            checkboxMenu->getScaledContentWidth() + 10.f + checkboxLabel->getScaledContentWidth() + 10.f);

    toggler->setUserObject("key"_spr, ObjWrapper<LevelKeys>::create(key));
    toggler->setID(fmt::format("{}-toggle", enchantum::to_string(key)));
    

    return checkboxContainer;
}

CCMenuItemToggler* InputTriggerPopup::createTabToggler(const std::string& label, Tab tab) {
    auto onSpr = CCScale9Sprite::create("GJ_button_02.png");
    auto offSpr = CCScale9Sprite::create("GJ_button_04.png");

    auto keyLabelOn = CCLabelBMFont::create(label.c_str(), "bigFont.fnt");
    keyLabelOn->setScale(0.5f);

    onSpr->setContentSize({keyLabelOn->getScaledContentWidth() + 20, keyLabelOn->getScaledContentHeight() + 10});
    onSpr->addChild(keyLabelOn);

    keyLabelOn->setPosition(onSpr->getContentSize() * 0.5f + CCPoint{0.f, 1.f});

    auto keyLabelOff = CCLabelBMFont::create(label.c_str(), "bigFont.fnt");
    keyLabelOff->setScale(0.5f);

    offSpr->setContentSize({keyLabelOff->getScaledContentWidth() + 20, keyLabelOff->getScaledContentHeight() + 10});
    offSpr->addChild(keyLabelOff);

    keyLabelOff->setPosition(offSpr->getContentSize() * 0.5f + CCPoint{0.f, 1.f});

    auto toggler = CCMenuItemExt::createToggler(onSpr, offSpr, [this, tab](CCMenuItemToggler* self) {
        bool wasToggled = self->isToggled();
        m_currentActionTab.value(tab);
    });

    toggler->setID(fmt::format("{}-toggle", label));

    m_tabData[tab].common.toggler = toggler;
    return toggler;
}

CCMenuItemToggler*
InputTriggerPopup::createKeyboardToggler(LevelKeys key, float width, const std::string& labelOverride) {
    auto onSpr = CCScale9Sprite::create("GJ_button_02.png");
    onSpr->setContentSize({width, 40});
    auto offSpr = CCScale9Sprite::create("GJ_button_04.png");
    offSpr->setContentSize({width, 40});

    std::string keyStr;
    if (labelOverride.empty()) {
        keyStr = std::string(fixKeyName(enchantum::to_string(key)));
        utils::string::toUpperIP(keyStr);
    } else {
        keyStr = labelOverride;
    }

    auto keyLabelOn = CCLabelBMFont::create(keyStr.c_str(), "bigFont.fnt");
    keyLabelOn->setScale(0.5f);
    keyLabelOn->setPosition(onSpr->getContentSize() * 0.5f);

    onSpr->setScale(0.6f);
    onSpr->addChild(keyLabelOn);

    auto keyLabelOff = CCLabelBMFont::create(keyStr.c_str(), "bigFont.fnt");
    keyLabelOff->setScale(0.5f);
    keyLabelOff->setPosition(offSpr->getContentSize() * 0.5f);

    offSpr->setScale(0.6f);
    offSpr->addChild(keyLabelOff);

    auto toggler = CCMenuItemExt::createToggler(onSpr, offSpr, [this, key](CCMenuItemToggler* self) {
        // for (const auto& [toggle, tab] : m_toggles) {
        //     if (toggle == self)
        //         continue;
        //     toggle->toggle(false);
        // }
        // m_label.key = !self->isToggled() ? key : LevelKeys::empty;
        // m_modifiedKey = true;

        // checkSpecialKey();
        log::info("toggler");
    });

    toggler->setUserObject("key"_spr, ObjWrapper<LevelKeys>::create(key));
    toggler->setID(fmt::format("{}-toggle", keyStr));

    return toggler;
}

CCMenu* InputTriggerPopup::createKeyboardMenu(float gap, float yOffset, int row) {
    CCMenu* menu = CCMenu::create();
    menu->setContentSize({380, 40});
    menu->ignoreAnchorPointForPosition(false);
    menu->setAnchorPoint({0.5f, 1.f});
    menu->setPosition({m_mainLayer->getContentWidth() * 0.5f, m_mainLayer->getContentHeight() - yOffset});
    menu->setID(fmt::format("keyboard-menu-{}", row));

    auto layout = RowLayout::create();
    layout->setAutoScale(false);
    layout->setGap(gap);
    menu->setLayout(layout);

    return menu;
}

void InputTriggerPopup::setupKeyboardTab() {

    auto keyboardContainer = CCNode::create();
    keyboardContainer->setContentSize(m_mainLayer->getContentSize());
    keyboardContainer->setAnchorPoint({0, 0});
    keyboardContainer->ignoreAnchorPointForPosition(false);
    keyboardContainer->setID("keyboard-container");
    m_tabData[Tab::Keyboard].common.nodeContainer = keyboardContainer;

    auto& kbdata = getKeyboardData();
    keyboardContainer->addChild(createIntegerInput("Group ID: ", &kbdata.m_keyAction.group, {50, 20}));

    auto row1Menu = createKeyboardMenu(2.f, 70, 1);

    row1Menu->addChild(createKeyboardToggler(LevelKeys::f1));
    row1Menu->addChild(createKeyboardToggler(LevelKeys::f2));
    row1Menu->addChild(createKeyboardToggler(LevelKeys::f3));
    row1Menu->addChild(createKeyboardToggler(LevelKeys::f4));
    row1Menu->addChild(createKeyboardToggler(LevelKeys::f5));
    row1Menu->addChild(createKeyboardToggler(LevelKeys::f6));
    row1Menu->addChild(createKeyboardToggler(LevelKeys::f7));
    row1Menu->addChild(createKeyboardToggler(LevelKeys::f8));
    row1Menu->addChild(createKeyboardToggler(LevelKeys::f9));
    row1Menu->addChild(createKeyboardToggler(LevelKeys::f10));
    row1Menu->addChild(createKeyboardToggler(LevelKeys::f11));
    row1Menu->addChild(createKeyboardToggler(LevelKeys::f12));

    row1Menu->updateLayout();
    keyboardContainer->addChild(row1Menu);

    auto row2Menu = createKeyboardMenu(2.f, 96, 2);

    row2Menu->addChild(createKeyboardToggler(LevelKeys::KEY_1));
    row2Menu->addChild(createKeyboardToggler(LevelKeys::KEY_2));
    row2Menu->addChild(createKeyboardToggler(LevelKeys::KEY_3));
    row2Menu->addChild(createKeyboardToggler(LevelKeys::KEY_4));
    row2Menu->addChild(createKeyboardToggler(LevelKeys::KEY_5));
    row2Menu->addChild(createKeyboardToggler(LevelKeys::KEY_6));
    row2Menu->addChild(createKeyboardToggler(LevelKeys::KEY_7));
    row2Menu->addChild(createKeyboardToggler(LevelKeys::KEY_8));
    row2Menu->addChild(createKeyboardToggler(LevelKeys::KEY_9));
    row2Menu->addChild(createKeyboardToggler(LevelKeys::KEY_0));

    row2Menu->updateLayout();
    keyboardContainer->addChild(row2Menu);

    auto row3Menu = createKeyboardMenu(2.f, 122, 3);

    row3Menu->addChild(createKeyboardToggler(LevelKeys::q));
    row3Menu->addChild(createKeyboardToggler(LevelKeys::w));
    row3Menu->addChild(createKeyboardToggler(LevelKeys::e));
    row3Menu->addChild(createKeyboardToggler(LevelKeys::r));
    row3Menu->addChild(createKeyboardToggler(LevelKeys::t));
    row3Menu->addChild(createKeyboardToggler(LevelKeys::y));
    row3Menu->addChild(createKeyboardToggler(LevelKeys::u));
    row3Menu->addChild(createKeyboardToggler(LevelKeys::i));
    row3Menu->addChild(createKeyboardToggler(LevelKeys::o));
    row3Menu->addChild(createKeyboardToggler(LevelKeys::p));

    row3Menu->updateLayout();
    keyboardContainer->addChild(row3Menu);

    auto row4Menu = createKeyboardMenu(2.f, 148, 4);

    row4Menu->addChild(createKeyboardToggler(LevelKeys::a));
    row4Menu->addChild(createKeyboardToggler(LevelKeys::s));
    row4Menu->addChild(createKeyboardToggler(LevelKeys::d));
    row4Menu->addChild(createKeyboardToggler(LevelKeys::f));
    row4Menu->addChild(createKeyboardToggler(LevelKeys::g));
    row4Menu->addChild(createKeyboardToggler(LevelKeys::h));
    row4Menu->addChild(createKeyboardToggler(LevelKeys::j));
    row4Menu->addChild(createKeyboardToggler(LevelKeys::k));
    row4Menu->addChild(createKeyboardToggler(LevelKeys::l));
    row4Menu->addChild(createKeyboardToggler(LevelKeys::enter, 90, "Enter"));

    row4Menu->updateLayout();
    keyboardContainer->addChild(row4Menu);

    auto row5Menu = createKeyboardMenu(2.f, 174, 5);

    row5Menu->addChild(createKeyboardToggler(LevelKeys::z));
    row5Menu->addChild(createKeyboardToggler(LevelKeys::x));
    row5Menu->addChild(createKeyboardToggler(LevelKeys::c));
    row5Menu->addChild(createKeyboardToggler(LevelKeys::v));
    row5Menu->addChild(createKeyboardToggler(LevelKeys::b));
    row5Menu->addChild(createKeyboardToggler(LevelKeys::n));
    row5Menu->addChild(createKeyboardToggler(LevelKeys::m));
    row5Menu->addChild(createKeyboardToggler(LevelKeys::leftShift, 80, "Shift"));

    row5Menu->updateLayout();
    keyboardContainer->addChild(row5Menu);

    auto row6Menu = createKeyboardMenu(2.f, 200, 6);

    row6Menu->addChild(createKeyboardToggler(LevelKeys::leftCtrl, 50, "Ctrl"));
    row6Menu->addChild(createKeyboardToggler(LevelKeys::leftAlt, 50, "Alt"));
    row6Menu->addChild(createKeyboardToggler(LevelKeys::space, 240, "Space"));

    row6Menu->updateLayout();
    keyboardContainer->addChild(row6Menu);

    auto m_onReleaseToggle = kbdata.m_onReleaseToggle;
    m_onReleaseToggle = CCMenuItemExt::createTogglerWithStandardSprites(0.7f, [this](CCMenuItemToggler* toggler) {
        // m_label.keyDown = toggler->isToggled();
        // m_modifiedRelease = true;
        log::info("release toggle");
    });

    m_onReleaseToggle->setPosition({m_mainLayer->getContentWidth() - 120, 55});
    m_onReleaseToggle->setID("on-release-toggle");

    
    m_buttonMenu->addChild(m_onReleaseToggle);
    keyboardContainer->addChild(m_onReleaseToggle);

    auto m_onReleaseLabel = kbdata.m_onReleaseLabel;
    m_onReleaseLabel = CCLabelBMFont::create("Trigger On\nRelease", "bigFont.fnt");
    m_onReleaseLabel->setScale(0.35f);
    m_onReleaseLabel->setAnchorPoint({0.f, 0.5f});
    m_onReleaseLabel->setPosition(
            {m_onReleaseToggle->getPositionX() + m_onReleaseToggle->getContentWidth() * 0.5f + 10,
             m_onReleaseToggle->getPositionY()});
    m_onReleaseLabel->setID("on-release-label");

    keyboardContainer->addChild(m_onReleaseLabel);

    m_mainLayer->addChild(keyboardContainer);
}

void InputTriggerPopup::setupMouseTab() {
    auto mouseContainer = CCNode::create();
    mouseContainer->setContentSize(m_mainLayer->getContentSize());
    mouseContainer->setAnchorPoint({0.f, 0.f});
    mouseContainer->ignoreAnchorPointForPosition(false);
    mouseContainer->setID("mouse-container");
    m_tabData[Tab::Mouse].common.nodeContainer = mouseContainer;

    auto innerContainer = CCNode::create();
    innerContainer->setAnchorPoint({0.5f, 1.f});
    innerContainer->ignoreAnchorPointForPosition(false);
    innerContainer->setPosition({mouseContainer->getContentWidth() * 0.5f, mouseContainer->getContentHeight() - 90.f});
    innerContainer->setContentHeight(105.f);
    innerContainer->setID("inner-container");

    auto innerContainerLayout = ColumnLayout::create();
    innerContainerLayout->setGrowCrossAxis(true);
    innerContainerLayout->setCrossAxisOverflow(true);
    innerContainerLayout->setAutoScale(false);
    innerContainerLayout->setAxisReverse(true);
    innerContainerLayout->setCrossAxisReverse(true);
    innerContainerLayout->setCrossAxisLineAlignment(AxisAlignment::Start);

    innerContainer->setLayout(innerContainerLayout);

    innerContainer->addChild(createMouseToggler("Left Click", LevelKeys::leftMouse));
    innerContainer->addChild(createMouseToggler("Right Click", LevelKeys::rightMouse));
    innerContainer->addChild(createMouseToggler("Middle Click", LevelKeys::middleMouse));
    innerContainer->addChild(createMouseToggler("Button 3 Click", LevelKeys::mouse3));
    innerContainer->addChild(createMouseToggler("Button 4 Click", LevelKeys::mouse4));

    innerContainer->addChild(createMouseToggler("Scroll Up", LevelKeys::wheelUp));
    innerContainer->addChild(createMouseToggler("Scroll Down", LevelKeys::wheelDown));

    innerContainer->addChild(createMouseToggler("Lock To Cursor", LevelKeys::cursor));

    innerContainer->updateLayout();
    mouseContainer->addChild(innerContainer);

    m_mainLayer->addChild(mouseContainer);

}

void InputTriggerPopup::setupTouchTab() {
    auto touchTabContainer = CCNode::create();
    touchTabContainer->setContentSize(m_mainLayer->getContentSize());
    touchTabContainer->setAnchorPoint({0.f, 0.f});
    touchTabContainer->ignoreAnchorPointForPosition(false);
    touchTabContainer->setID("touch-container");

    m_mainLayer->addChild(touchTabContainer);
    m_tabData[Tab::Touch].common.nodeContainer = touchTabContainer;

}
CCNode* InputTriggerPopup::createIntegerInput(const char* labelText, int* valuePtr, CCPoint pos) {

    auto groupInputContainer = CCNode::create();
    groupInputContainer->setContentSize({100, 60});
    groupInputContainer->ignoreAnchorPointForPosition(false);
    groupInputContainer->setAnchorPoint({0, 0});
    groupInputContainer->setPosition(pos);
    groupInputContainer->setID("group-input-container");

    // 1. Background Sprite
    auto groupInputBG = CCScale9Sprite::create("square02_small.png");
    groupInputBG->setScale(0.8f);
    groupInputBG->setContentSize({70, 30});
    groupInputBG->setZOrder(-1);
    groupInputBG->setOpacity(100);
    groupInputContainer->addChild(groupInputBG);

    // 2. Label (Parameterized)
    auto groupInputLabel = CCLabelBMFont::create(labelText, "goldFont.fnt");
    groupInputLabel->setScale(0.56f);
    groupInputLabel->setAnchorPoint({0.5f, 0.f});
    groupInputContainer->addChild(groupInputLabel);

    // 3. Text Input
    // We use the pointer to set the initial value
    std::string initialVal = std::to_string(*valuePtr);
    auto input = geode::TextInput::create(48, "Num");
    input->setString(initialVal);
    input->setCommonFilter(geode::CommonFilter::Int);
    input->setMaxCharCount(4);
    input->hideBG();

    input->setCallback([valuePtr](const std::string& str) {
        if (!str.empty()) {
            *valuePtr = std::stoi(str);
        }
    });

    // Positioning helper
    float centerX = groupInputContainer->getContentWidth() * 0.5f;
    input->setPosition({centerX, input->getContentHeight() * 0.5f});
    groupInputBG->setPosition(input->getPosition());
    groupInputLabel->setPosition({centerX, input->getPositionY() + input->getContentHeight() * 0.5f + 5});

    groupInputContainer->addChild(input);

    // 4. Buttons & Menu
    auto inputBounds = input->boundingBox();

    // Lambda to update both the pointer and the text visual
    auto updateUI = [input, valuePtr](int delta) {
        *valuePtr = std::clamp(*valuePtr + delta, 0, 9999);
        input->setString(std::to_string(*valuePtr));
    };

    auto decrBtn = CCMenuItemExt::createSpriteExtraWithFrameName(
            "edit_leftBtn_001.png", 1.f, [updateUI](auto) { updateUI(-1); });

    auto incrBtn = CCMenuItemExt::createSpriteExtraWithFrameName(
            "edit_rightBtn_001.png", 1.f, [updateUI](auto) { updateUI(1); });

    decrBtn->setPosition({inputBounds.getMinX() - 25, input->getPositionY()});
    incrBtn->setPosition({inputBounds.getMaxX() + 25, input->getPositionY()});

    auto menu = CCMenu::create();
    menu->setPosition({0, 0});
    menu->setContentSize(groupInputContainer->getContentSize());
    menu->addChild(decrBtn);
    menu->addChild(incrBtn);

    groupInputContainer->addChild(menu);

    return groupInputContainer;
}


void InputTriggerPopup::onClose(CCObject* sender) {

    // TODO :on close code
    Popup::onClose(sender);
}
