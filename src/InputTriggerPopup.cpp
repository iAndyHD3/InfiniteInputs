#include "InputTriggerPopup.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/TextGameObject.hpp>
#include <algorithm>
#include <arc/prelude.hpp>
#include <enchantum/enchantum.hpp>
#include <fmt/format.h>
#include "Geode/cocos/cocoa/CCGeometry.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/loader/Loader.hpp"
#include "Geode/ui/General.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/utils/ZStringView.hpp"
#include "Geode/utils/general.hpp"
#include "LevelKeys.hpp"
#include "LogVar.hpp"
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
    log::info("init with string: {}", object->m_text);
    getKeyboardData().m_keyAction.key = LevelKeys::empty;
    getKeyboardData().m_keyAction.keyDown = true;
    getMouseData().keyDown = true;

    m_noElasticity = true;
    m_object = object;
    m_closeBtn->removeFromParent();

    auto title = CCLabelBMFont::create("Input Settings", "bigFont.fnt");
    title->setScale(0.5f);
    title->setPosition({m_mainLayer->getContentWidth() * 0.5f, m_mainLayer->getContentHeight() - 14});
    title->setID("title-label");

    m_mainLayer->addChild(title);
    m_mainLayer->setID("main-layer");

    auto getHelpFn = [this]() -> std::string {
        switch (m_currentActionTab.get()) {
            case Tab::Keyboard:
                return R"(Activates a <cg>Group ID</c> based on a <cy>key</c> action. This trigger works similarly to the UI trigger; it is processed at the start of the level, and cannot be spawned or toggled.
<cp>Trigger On Release</c> activates the group on key or mouse release instead of press.)";
            case Tab::Mouse: return "Mouse";
            case Tab::Touch: return "Touch";
            default: return "Unknown Tab";
        }
    };

    auto helpBtn = CCMenuItemExt::createSpriteExtraWithFrameName(
            "GJ_infoIcon_001.png", 1.f, [getHelpFn](CCMenuItemSpriteExtra* self) {
                geode::createQuickPopup("Help", getHelpFn(), "OK", nullptr, nullptr);
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
    log::info("KEY BEFORE 3: {}", enchantum::to_string(getKeyboardData().m_keyAction.key));

    tabsMenu->addChild(createTabToggler("Keyboard", Tab::Keyboard));
    tabsMenu->addChild(createTabToggler("Mouse", Tab::Mouse));
    tabsMenu->addChild(createTabToggler("Touch", Tab::Touch));

    tabsMenu->updateLayout();
    log::info("KEY BEFORE 2: {}", enchantum::to_string(getKeyboardData().m_keyAction.key));

    log::info("BEFORE BEFORE IF");

    log::info("BEFORE IF");

    Tab selectedTab;
    if (auto parsed = KeyAction::parse(object->m_text)) {
        log::info("PASED KEY: {}", enchantum::to_string((*parsed).key));

        if (isSpecialMouseKeyboardKey(parsed->key)) {
            m_mouseTabData.key = reaction::var(parsed->key);
            m_mouseTabData.group = parsed->group;
            log::info("setting group: {}", m_mouseTabData.group);
            m_mouseTabData.keyDown = parsed->keyDown;
            selectedTab = Tab::Mouse;
        } else {
            m_keyboardTabData.m_keyAction = *parsed;
            selectedTab = Tab::Keyboard;
        }
    } else if (auto parsed = SimpleKeyAction::parse(object->m_text)) {
        m_mouseTabData.m_simpleKeyAction = std::move(*parsed);
        m_mouseTabData.group = m_mouseTabData.m_simpleKeyAction.group;
        m_mouseTabData.key = reaction::var(parsed->key);
        selectedTab = (Tab::Mouse);
        log::info("parsed mouse input");
    } else if (auto parsed = ClickAction::parse(object->m_text)) {
        m_touchTabData.m_clickAction = std::move(*parsed);
        selectedTab = (Tab::Touch);
        log::info("parsed touch input");
    } else {
        selectedTab = (Tab::Keyboard);
        m_keyboardTabData.m_keyAction.key = LevelKeys::empty;
        log::info(
                "Could not parse any input, defaulting to keyboard. key: {}",
                enchantum::to_string(getKeyboardData().m_keyAction.key));
    }
    m_currentActionTab.value(selectedTab);

    log::info("BEFORE VALUE");
    log::info("KEY BEFORE: {}", enchantum::to_string(getKeyboardData().m_keyAction.key));

    // KEYBOARD

    auto keyboardContainer = CCNode::create();
    keyboardContainer->setContentSize(m_mainLayer->getContentSize());
    keyboardContainer->setAnchorPoint({0, 0});
    keyboardContainer->ignoreAnchorPointForPosition(false);
    keyboardContainer->setID("keyboard-container");
    m_keyboardToggler.tabNodes.push_back(keyboardContainer);

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

    auto m_onReleaseToggle =
            CCMenuItemExt::createTogglerWithStandardSprites(0.7f, [&kbdata](CCMenuItemToggler* toggler) {
                kbdata.m_keyAction.keyDown = !kbdata.m_keyAction.keyDown;
                log::info("release toggle");
            });

    m_onReleaseToggle->toggle(!kbdata.m_keyAction.keyDown);
    m_onReleaseToggle->setPosition({m_mainLayer->getContentWidth() - 120, 55});
    m_onReleaseToggle->setID("on-release-toggle");

    m_buttonMenu->addChild(m_onReleaseToggle);
    m_keyboardToggler.tabNodes.push_back(m_onReleaseToggle);

    auto onReleaseLabel = CCLabelBMFont::create("Trigger On\nRelease", "bigFont.fnt");
    onReleaseLabel->setScale(0.35f);
    onReleaseLabel->setAnchorPoint({0.f, 0.5f});
    onReleaseLabel->setPosition(
            {m_onReleaseToggle->getPositionX() + m_onReleaseToggle->getContentWidth() * 0.5f + 10,
             m_onReleaseToggle->getPositionY()});
    onReleaseLabel->setID("on-release-label");

    keyboardContainer->addChild(onReleaseLabel);

    m_mainLayer->addChild(keyboardContainer);

    log::info("KEY AFTER: {}", enchantum::to_string(getKeyboardData().m_keyAction.key));

    // MOUSE

    auto mouseContainer = CCNode::create();
    mouseContainer->setContentSize(m_mainLayer->getContentSize());
    mouseContainer->setAnchorPoint({0.f, 0.f});
    mouseContainer->ignoreAnchorPointForPosition(false);
    mouseContainer->setID("mouse-container");
    m_mainLayer->addChild(mouseContainer);


    mouseContainer->addChild(createIntegerInput("Group ID: ", &m_mouseTabData.group, {50, 20}));

    m_mouseToggler.tabNodes.push_back(mouseContainer);

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
    innerContainer->addChild(createMouseToggler("Mod Loaded", LevelKeys::modLoaded));
    innerContainer->updateLayout();
    // NO more:
    mouseContainer->addChild(innerContainer);
    // instead:
    auto mouseOnReleaseToggle =
            CCMenuItemExt::createTogglerWithStandardSprites(0.7f, [this](CCMenuItemToggler* toggler) {
                m_mouseTabData.keyDown = !m_mouseTabData.keyDown;
                log::info("release toggle");
            });

    LOGI(m_mouseTabData.keyDown);
    mouseOnReleaseToggle->toggle(!m_mouseTabData.keyDown);

    mouseOnReleaseToggle->setID("mouse-on-release-toggle");

    auto mouseOnReleaseToggleMenu = CCMenu::create();
    mouseOnReleaseToggleMenu->setPosition({m_mainLayer->getContentWidth() - 120, 55});
    mouseOnReleaseToggleMenu->addChild(mouseOnReleaseToggle);
    mouseContainer->addChild(mouseOnReleaseToggleMenu);
    // m_mouseToggler.tabNodes.push_back(mouseOnReleaseToggle);


    auto mouseOnReleaseLabel = CCLabelBMFont::create("Trigger On\nRelease", "bigFont.fnt");
    mouseOnReleaseLabel->setScale(0.35f);
    mouseOnReleaseLabel->setAnchorPoint({0.f, 0.5f});
    mouseOnReleaseLabel->setPosition(
            {mouseOnReleaseToggle->getPositionX() + mouseOnReleaseToggle->getContentWidth() * 0.5f + 10,
             mouseOnReleaseToggle->getPositionY()});
    mouseOnReleaseLabel->setID("mouse-on-release-label");

    mouseOnReleaseToggle->setVisible(false);
    mouseOnReleaseLabel->setVisible(false);

    mouseOnReleaseToggleMenu->addChild(mouseOnReleaseLabel);

    m_mouseToggler.tabNodes.push_back(mouseOnReleaseToggleMenu);
    reaction::action([mouseOnReleaseLabel, mouseOnReleaseToggle, this]() {
        LevelKeys newkey = m_mouseTabData.key();
        log::info("CALLED WITH: {}", enchantum::to_string(newkey));
        bool isSpecialKey = isSpecialMouseKeyboardKey(newkey);
        LOGI(isSpecialKey);
        mouseOnReleaseLabel->setVisible(isSpecialKey);
        mouseOnReleaseToggle->setVisible(isSpecialKey);
        LOGI(mouseOnReleaseToggle->isVisible());
    });

    auto touchTabContainer = CCNode::create();
    touchTabContainer->setContentSize(m_mainLayer->getContentSize());
    touchTabContainer->setAnchorPoint({0.f, 0.f});
    touchTabContainer->ignoreAnchorPointForPosition(false);
    touchTabContainer->setID("touch-container");

    float ROW_LEFT_X = touchTabContainer->getContentWidth() / 4 - 15;
    float ROW_RIGHT_X = ROW_LEFT_X + 150;
    constexpr float Y_START = 120;
    constexpr float Y_DIFFERENCE = 55;

    auto row1Left = createIntegerInput(
            "Collision: ", &m_touchTabData.m_clickAction.collisionBlockId,
            {ROW_LEFT_X, touchTabContainer->getContentHeight() - Y_START});

    auto row2Left = createIntegerInput(
            "Group Down: ", &m_touchTabData.m_clickAction.groupIdCursorDown,
            {ROW_LEFT_X, touchTabContainer->getContentHeight() - Y_START - Y_DIFFERENCE});
    auto row2Right = createIntegerInput(
            "Group Up: ", &m_touchTabData.m_clickAction.groupIdCursorUp,
            {ROW_RIGHT_X, touchTabContainer->getContentHeight() - Y_START - Y_DIFFERENCE});

    auto row3Left = createIntegerInput(
            "Group Enter: ", &m_touchTabData.m_clickAction.groupIdCursorEnter,
            {ROW_LEFT_X, touchTabContainer->getContentHeight() - Y_START - (Y_DIFFERENCE * 2)});
    auto row3Right = createIntegerInput(
            "Group Exit: ", &m_touchTabData.m_clickAction.groupIdCursorExit,
            {ROW_RIGHT_X, touchTabContainer->getContentHeight() - Y_START - (Y_DIFFERENCE * 2)});

    touchTabContainer->addChild(row1Left);
    touchTabContainer->addChild(row2Left);
    touchTabContainer->addChild(row2Right);
    touchTabContainer->addChild(row3Left);
    touchTabContainer->addChild(row3Right);

    auto boolsMenu = CCMenu::create();
    boolsMenu->ignoreAnchorPointForPosition(false);
    boolsMenu->setAnchorPoint({0.f, 0.f});
    boolsMenu->setContentSize({m_mainLayer->getContentWidth(), 60.f});
    boolsMenu->setPosition({0.f, 30.f});
    boolsMenu->setID("bools-menu");

    auto stealTouchesToggle =
            CCMenuItemExt::createTogglerWithStandardSprites(0.7f, [&, this](CCMenuItemToggler* toggler) {
                m_touchTabData.m_clickAction.stealTouches = toggler->isToggled();
                log::info("steal touches toggle");
            });
    stealTouchesToggle->toggle(m_touchTabData.m_clickAction.stealTouches);
    stealTouchesToggle->setPosition({m_mainLayer->getContentWidth() * 0.25f - 40.f, 30.f});
    stealTouchesToggle->setID("steal-touches-toggle");

    auto stealTouchesLabel = CCLabelBMFont::create("Steal\nTouches", "bigFont.fnt");
    stealTouchesLabel->setScale(0.35f);
    stealTouchesLabel->setAnchorPoint({0.f, 0.5f});
    stealTouchesLabel->setPosition({m_mainLayer->getContentWidth() * 0.25f - 10.f, 30.f});
    stealTouchesLabel->setID("steal-touches-label");

    boolsMenu->addChild(stealTouchesToggle);
    boolsMenu->addChild(stealTouchesLabel);

    auto allowStealFromToggle =
            CCMenuItemExt::createTogglerWithStandardSprites(0.7f, [&, this](CCMenuItemToggler* toggler) {
                m_touchTabData.m_clickAction.allowStealFrom = toggler->isToggled();
                log::info("allow steal from toggle");
            });
    allowStealFromToggle->toggle(m_touchTabData.m_clickAction.allowStealFrom);
    allowStealFromToggle->setPosition({m_mainLayer->getContentWidth() * 0.75f - 10.f, 30.f});
    allowStealFromToggle->setID("allow-steal-from-toggle");

    auto allowStealFromLabel = CCLabelBMFont::create("Allow Steal\nFrom Others", "bigFont.fnt");
    allowStealFromLabel->setScale(0.35f);
    allowStealFromLabel->setAnchorPoint({0.f, 0.5f});
    allowStealFromLabel->setPosition({m_mainLayer->getContentWidth() * 0.75f + 20.f, 30.f});
    allowStealFromLabel->setID("allow-steal-from-label");

    boolsMenu->addChild(allowStealFromToggle);
    boolsMenu->addChild(allowStealFromLabel);

    touchTabContainer->addChild(boolsMenu);

    m_mainLayer->addChild(touchTabContainer);
    m_touchToggler.tabNodes.push_back(touchTabContainer);
    m_touchToggler.tabNodes.push_back(boolsMenu);

    reaction::action([this]() {
        Tab toggledTab = m_currentActionTab();
        log::info("REACTION TOGGLED TAB: {}", enchantum::to_string(toggledTab));

        for (auto& toggler : {std::ref(m_keyboardToggler), std::ref(m_mouseToggler), std::ref(m_touchToggler)}) {
            for (const auto& nodes : toggler.get().tabNodes) {
                nodes->setVisible(false);
            }

            if (toggledTab != Tab::Keyboard && &toggler.get() == &m_keyboardToggler) {
                toggler.get().toggler->toggle(false);
                toggler.get().toggler->setEnabled(true);
            } else if (toggledTab != Tab::Mouse && &toggler.get() == &m_mouseToggler) {
                toggler.get().toggler->toggle(false);
                toggler.get().toggler->setEnabled(true);
            } else if (toggledTab != Tab::Touch && &toggler.get() == &m_touchToggler) {
                toggler.get().toggler->toggle(false);
                toggler.get().toggler->setEnabled(true);
            }
        }

        TabToggler& activeToggler = getToggler(toggledTab);
        for (const auto& nodes : activeToggler.tabNodes) {
            LOGI(nodes->getID());
            nodes->setVisible(true);
        }

        log::info("clickable false on: {}", activeToggler.toggler);
        activeToggler.toggler->setEnabled(false);
        activeToggler.toggler->toggle(true);
    });

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

    auto& mouseData = getMouseData();
    auto toggler = CCMenuItemExt::createTogglerWithStandardSprites(0.7f, [&mouseData, key](CCMenuItemToggler* self) {
        updateButtonPressed(&mouseData.pressed, self);
        if (!self->isToggled()) {
            mouseData.key.value(key);
        } else {
            mouseData.key.value(LevelKeys::empty);
        }
    });
    if (key == mouseData.key.get()) {
        toggler->toggle(true);
        mouseData.pressed = toggler;
    }

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

    getToggler(tab).toggler = toggler;
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

    auto& kbData = getKeyboardData();
    auto toggler = CCMenuItemExt::createToggler(onSpr, offSpr, [&kbData, key](CCMenuItemToggler* self) {
        updateButtonPressed(&kbData.pressed, self);
        kbData.m_keyAction.key = key;
        log::info("toggled key button: {}", enchantum::to_string(key));
    });


    if (kbData.m_keyAction.key == key) {
        toggler->toggle(true);
        kbData.pressed = toggler;
    }


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

bool InputTriggerPopup::isSpecialMouseKeyboardKey(LevelKeys key) {
    using enum LevelKeys;
    switch (key) {
        default: return false;
        case leftMouse:
        case rightMouse:
        case middleMouse:
        case mouse3:
        case mouse4: return true;
    }
}

CCNode* InputTriggerPopup::createIntegerInput(const char* labelText, int* valuePtr, CCPoint pos) {

    auto groupInputContainer = CCNode::create();
    groupInputContainer->setContentSize({100, 60});
    groupInputContainer->ignoreAnchorPointForPosition(false);
    groupInputContainer->setAnchorPoint({0, 0});
    groupInputContainer->setPosition(pos);
    groupInputContainer->setID("group-input-container");

    auto groupInputBG = CCScale9Sprite::create("square02_small.png");
    groupInputBG->setScale(0.8f);
    groupInputBG->setContentSize({70, 30});
    groupInputBG->setZOrder(-1);
    groupInputBG->setOpacity(100);
    groupInputContainer->addChild(groupInputBG);

    auto groupInputLabel = CCLabelBMFont::create(labelText, "goldFont.fnt");
    groupInputLabel->setScale(0.56f);
    groupInputLabel->setAnchorPoint({0.5f, 0.f});
    groupInputContainer->addChild(groupInputLabel);

    auto input = geode::TextInput::create(48, "Num");
    if (*valuePtr > 0) {
        input->setString(geode::utils::numToString(*valuePtr));
    }


    input->setCommonFilter(geode::CommonFilter::Int);
    input->setMaxCharCount(4);
    input->hideBG();

    input->setCallback([valuePtr](const std::string& str) {
        if (!str.empty()) {
            *valuePtr = geode::utils::numFromString<int>(str).unwrapOr(0);
        }
    });

    float centerX = groupInputContainer->getContentWidth() * 0.5f;
    input->setPosition({centerX, input->getContentHeight() * 0.5f});
    groupInputBG->setPosition(input->getPosition());
    groupInputLabel->setPosition({centerX, input->getPositionY() + input->getContentHeight() * 0.5f + 5});

    groupInputContainer->addChild(input);

    auto inputBounds = input->boundingBox();

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

void InputTriggerPopup::updateButtonPressed(CCMenuItemToggler** oldToggled, CCMenuItemToggler* newpressed) {
    bool wasToggled = newpressed->isToggled();
    if (wasToggled && *oldToggled == newpressed) {
        return;
    }

    if (*oldToggled) {
        (*oldToggled)->toggle(false);
    }

    *oldToggled = newpressed;
}

void InputTriggerPopup::onClose(CCObject* sender) {

    std::string label;

    switch (m_currentActionTab.get()) {
        case Tab::Keyboard: label = getKeyboardData().m_keyAction.getLabel(); break;
        case Tab::Mouse: {
            auto& md = getMouseData();
            if (isSpecialMouseKeyboardKey(md.key.get())) {
                auto& kbd = getKeyboardData();
                kbd.m_keyAction.group = md.group;
                kbd.m_keyAction.key = md.key.get();
                kbd.m_keyAction.keyDown = md.keyDown;
                label = kbd.m_keyAction.getLabel();
            } else {
                md.m_simpleKeyAction.group = md.group;
                md.m_simpleKeyAction.key = md.key.get();
                label = md.m_simpleKeyAction.getLabel();
            }
            break;
        }
        case Tab::Touch: label = getTouchData().m_clickAction.getLabel(); break;
    }
    log::info("saving: {}", label);
    m_object->updateTextObject(label, false);
    Popup::onClose(sender);
}
