#include "InputTriggerPopup.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/TextGameObject.hpp>
#include <algorithm>
#include <arc/prelude.hpp>
#include <enchantum/enchantum.hpp>
#include <fmt/format.h>
#include "BetterGeodeLogs.hpp"
#include "GlobalOverviewPopup.hpp"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/cocoa/CCGeometry.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/ZStringView.hpp"
#include "Geode/utils/general.hpp"
#include "LevelKeys.hpp"
#include "TextParsing.hpp"
#include "hooks/EditorUI.hpp"

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

    Log.i("popup", "init with string: {}", object->m_text);
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
            case Tab::Button: return "Button";
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

    auto overviewSpr = CCSprite::createWithSpriteFrameName("GJ_longBtn06_001.png");
    auto overviewLabel = CCLabelBMFont::create("Overview", "bigFont.fnt");
    overviewLabel->setScale(0.35f);
    overviewLabel->setPosition(overviewSpr->getContentSize() * 0.5f);
    overviewSpr->addChild(overviewLabel);


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
    Log.i("popup", "KEY BEFORE 3: {}", enchantum::to_string(getKeyboardData().m_keyAction.key));

    tabsMenu->addChild(createTabToggler("Keyboard", Tab::Keyboard));
    tabsMenu->addChild(createTabToggler("Mouse", Tab::Mouse));
    tabsMenu->addChild(createTabToggler("Button", Tab::Button));
    tabsMenu->addChild(createTabToggler("Touch", Tab::Touch));

    tabsMenu->updateLayout();
    Log.i("popup", "KEY BEFORE 2: {}", enchantum::to_string(getKeyboardData().m_keyAction.key));

    Log.i("popup", "BEFORE BEFORE IF");

    Log.i("popup", "BEFORE IF");

    Tab selectedTab;
    if (auto parsed = KeyAction::parse(object->m_text)) {
        Log.i("popup", "PASED KEY: {}", enchantum::to_string((*parsed).key));

        if (isSpecialMouseKeyboardKey(parsed->key)) {
            m_mouseTabData.key = reaction::var(parsed->key);
            m_mouseTabData.group = parsed->group;
            Log.i("popup", "setting group: {}", m_mouseTabData.group);
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
        Log.i("popup", "parsed mouse input");
    } else if (auto parsed = ClickAction::parse(object->m_text)) {
        m_buttonTabData.m_clickAction = std::move(*parsed);
        selectedTab = (Tab::Button);
        Log.i("popup", "parsed button input");
    } else if (auto parsed = TouchAction::parse(object->m_text)) {
        m_touchTabData.m_touchAction = std::move(*parsed);
        selectedTab = (Tab::Touch);
        Log.i("popup", "parsed touch input");
    } else {
        selectedTab = (Tab::Keyboard);
        m_keyboardTabData.m_keyAction.key = LevelKeys::empty;
        log::info(
                "Could not parse any input, defaulting to keyboard. key: {}",
                enchantum::to_string(getKeyboardData().m_keyAction.key));
    }
    m_currentActionTab.value(selectedTab);

    Log.i("popup", "BEFORE VALUE");
    Log.i("popup", "KEY BEFORE: {}", enchantum::to_string(getKeyboardData().m_keyAction.key));

    // KEYBOARD

    auto keyboardContainer = CCNode::create();
    keyboardContainer->setContentSize(m_mainLayer->getContentSize());
    keyboardContainer->setAnchorPoint({0, 0});
    keyboardContainer->ignoreAnchorPointForPosition(false);
    keyboardContainer->setID("keyboard-container");
    m_keyboardToggler.tabNodes.push_back(keyboardContainer);

    auto& kbdata = getKeyboardData();
    keyboardContainer->addChild(createIntegerInput("Group ID:", &kbdata.m_keyAction.group, {50, 20}));

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

    auto sep = CCNode::create();
    sep->setContentSize({20,0});
    row5Menu->addChild(sep);

    row5Menu->addChild(createKeyboardToggler(LevelKeys::upArrow, 40, "", "edit_upBtn_001.png", false, false));

    sep = CCNode::create();
    sep->setContentSize({24, 0});
    row5Menu->addChild(sep);
    auto layout = (RowLayout*)row5Menu->getLayout();
    layout->setAxisAlignment(AxisAlignment::End);
    layout->setCrossAxisAlignment(AxisAlignment::End);


    row5Menu->updateLayout();
    keyboardContainer->addChild(row5Menu);

    auto row6Menu = createKeyboardMenu(2.f, 200, 6);
    layout = (RowLayout*)row6Menu->getLayout();
    layout->setAxisAlignment(AxisAlignment::End);
    layout->setCrossAxisAlignment(AxisAlignment::End);

    row6Menu->addChild(createKeyboardToggler(LevelKeys::leftCtrl, 50, "Ctrl"));
    row6Menu->addChild(createKeyboardToggler(LevelKeys::leftAlt, 50, "Alt"));
    row6Menu->addChild(createKeyboardToggler(LevelKeys::space, 240, "Space"));
    sep = CCNode::create();
    sep->setContentSize({20,0});
    row6Menu->addChild(sep);

    row6Menu->addChild(createKeyboardToggler(LevelKeys::leftArrow, 40, "", "edit_leftBtn_001.png", false, false));
    row6Menu->addChild(createKeyboardToggler(LevelKeys::downArrow, 40, "", "edit_downBtn_001.png", false, false));
    row6Menu->addChild(createKeyboardToggler(LevelKeys::rightArrow, 40, "", "edit_rightBtn_001.png", false, false));

    row6Menu->updateLayout();
    keyboardContainer->addChild(row6Menu);

    auto m_onReleaseToggle =
            CCMenuItemExt::createTogglerWithStandardSprites(0.7f, [&kbdata](CCMenuItemToggler* toggler) {
                kbdata.m_keyAction.keyDown = !kbdata.m_keyAction.keyDown;
                Log.i("popup", "release toggle");
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

    Log.i("popup", "KEY AFTER: {}", enchantum::to_string(getKeyboardData().m_keyAction.key));

    // MOUSE TAB

    auto mouseContainer = CCNode::create();
    mouseContainer->setContentSize(m_mainLayer->getContentSize());
    mouseContainer->setAnchorPoint({0.f, 0.f});
    mouseContainer->ignoreAnchorPointForPosition(false);
    mouseContainer->setID("mouse-container");
    m_mainLayer->addChild(mouseContainer);

    auto mouseGroupIDInput = createIntegerInput("Group ID:", &m_mouseTabData.group, {50, 20});
    mouseContainer->addChild(mouseGroupIDInput);

    reaction::action([mouseGroupIDInput, this]() {
        auto tab = m_currentActionTab();
        LevelKeys key = m_mouseTabData.key();

        if(tab != Tab::Mouse) return;

        if(auto label = typeinfo_cast<CCLabelBMFont*>(mouseGroupIDInput->getChildByID("group-input-label"))) {
            label->setString(SimpleKeyAction::isItemIdKey(key) ? "Item ID:" : "Group ID:");          
        }
    });

    m_mouseToggler.tabNodes.push_back(mouseContainer);

    auto innerContainer = CCNode::create();
    innerContainer->setAnchorPoint({0.5f, 1.f});
    innerContainer->ignoreAnchorPointForPosition(false);
    innerContainer->setPosition(
            {mouseContainer->getContentWidth() * 0.5f + 10, mouseContainer->getContentHeight() - 90.f});
    innerContainer->setContentHeight(140.f);
    innerContainer->setID("inner-container");
    mouseContainer->addChild(innerContainer);

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
    innerContainer->addChild(createMouseToggler("Mouse X", LevelKeys::mouseX));
    innerContainer->addChild(createMouseToggler("Mouse Y", LevelKeys::mouseY));
    innerContainer->addChild(createMouseToggler("Mouse Delta X", LevelKeys::deltaX));
    innerContainer->addChild(createMouseToggler("Mouse Delta Y", LevelKeys::deltaY));
    innerContainer->addChild(createMouseToggler("Window Width", LevelKeys::windowWidth));
    innerContainer->addChild(createMouseToggler("Window Height", LevelKeys::windowHeight));
    innerContainer->addChild(createMouseToggler("Mod Loaded", LevelKeys::modLoaded));
    innerContainer->addChild(createMouseToggler("Mod Loaded Mobile", LevelKeys::modLoadedMobile));
    innerContainer->addChild(createMouseToggler("Mod Loaded PC", LevelKeys::modLoadedPC));
    innerContainer->updateLayout();


    auto mouseOnReleaseToggle =
            CCMenuItemExt::createTogglerWithStandardSprites(0.7f, [this](CCMenuItemToggler* toggler) {
                m_mouseTabData.keyDown = !m_mouseTabData.keyDown;
                Log.i("popup", "release toggle");
            });

    Log.i("popup", "{}", m_mouseTabData.keyDown);
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
        Log.i("popup", "CALLED WITH: {}", enchantum::to_string(newkey));
        bool isSpecialKey = isSpecialMouseKeyboardKey(newkey);
        Log.i("popup", "is special: {}", isSpecialKey);
        mouseOnReleaseLabel->setVisible(isSpecialKey);
        mouseOnReleaseToggle->setVisible(isSpecialKey);

        Log.i("popup", "mouse release toggle is visible: {}", mouseOnReleaseToggle->isVisible());
    });

    // BUTTON TAB

    auto buttonTabContainer = CCNode::create();
    buttonTabContainer->setContentSize(m_mainLayer->getContentSize());
    buttonTabContainer->setAnchorPoint({0.f, 0.f});
    buttonTabContainer->ignoreAnchorPointForPosition(false);
    buttonTabContainer->setID("button-container");

    float B_ROW_LEFT_X = buttonTabContainer->getContentWidth() / 4 - 15;
    float B_ROW_RIGHT_X = B_ROW_LEFT_X + 150;
    constexpr float B_Y_START = 120;
    constexpr float B_Y_DIFFERENCE = 55;

    auto& bd = getButtonData().m_clickAction;

    auto bRow1Left = createIntegerInput(
            "Collision:", &bd.collisionBlockId,
            {B_ROW_LEFT_X, buttonTabContainer->getContentHeight() - B_Y_START});

    auto bRow2Left = createIntegerInput(
            "Press Group:", &bd.groupIdCursorDown,
            {B_ROW_LEFT_X, buttonTabContainer->getContentHeight() - B_Y_START - B_Y_DIFFERENCE});
    auto bRow2Right = createIntegerInput(
            "Release Group:", &bd.groupIdCursorUp,
            {B_ROW_RIGHT_X, buttonTabContainer->getContentHeight() - B_Y_START - B_Y_DIFFERENCE});

    auto bRow3Left = createIntegerInput(
            "Enter Group:", &bd.groupIdCursorEnter,
            {B_ROW_LEFT_X, buttonTabContainer->getContentHeight() - B_Y_START - (B_Y_DIFFERENCE * 2)});
    auto bRow3Right = createIntegerInput(
            "Exit Group:", &bd.groupIdCursorExit,
            {B_ROW_RIGHT_X, buttonTabContainer->getContentHeight() - B_Y_START - (B_Y_DIFFERENCE * 2)});

    buttonTabContainer->addChild(bRow1Left);
    buttonTabContainer->addChild(bRow2Left);
    buttonTabContainer->addChild(bRow2Right);
    buttonTabContainer->addChild(bRow3Left);
    buttonTabContainer->addChild(bRow3Right);

    auto bBoolsMenu = CCMenu::create();
    bBoolsMenu->ignoreAnchorPointForPosition(false);
    bBoolsMenu->setAnchorPoint({0.f, 0.f});
    bBoolsMenu->setContentSize({m_mainLayer->getContentWidth(), 60.f});
    bBoolsMenu->setPosition({0.f, 30.f});
    bBoolsMenu->setID("button-bools-menu");

    auto stealTouchesToggle =
            CCMenuItemExt::createTogglerWithStandardSprites(0.7f, [&, this](CCMenuItemToggler* toggler) {
                getButtonData().m_clickAction.stealTouches = !toggler->isToggled();
                Log.i("popup", "steal touches toggle");
            });
    stealTouchesToggle->toggle(bd.stealTouches);
    stealTouchesToggle->setPosition({m_mainLayer->getContentWidth() * 0.25f - 40.f, 30.f});
    stealTouchesToggle->setID("steal-touches-toggle");

    auto stealTouchesLabel = CCLabelBMFont::create("Claim Touch", "bigFont.fnt");
    stealTouchesLabel->setScale(0.35f);
    stealTouchesLabel->setAnchorPoint({0.f, 0.5f});
    stealTouchesLabel->setPosition({m_mainLayer->getContentWidth() * 0.25f - 10.f, 30.f});
    stealTouchesLabel->setID("steal-touches-label");

    bBoolsMenu->addChild(stealTouchesToggle);
    bBoolsMenu->addChild(stealTouchesLabel);

    auto allowStealFromToggle =
            CCMenuItemExt::createTogglerWithStandardSprites(0.7f, [&, this](CCMenuItemToggler* toggler) {
                getButtonData().m_clickAction.allowStealFrom = !toggler->isToggled();
                Log.i("popup", "allow steal from toggle");
            });
    allowStealFromToggle->toggle(bd.allowStealFrom);
    allowStealFromToggle->setPosition({m_mainLayer->getContentWidth() * 0.25f - 40.f, 0.f});
    allowStealFromToggle->setID("allow-steal-from-toggle");

    auto allowStealFromLabel = CCLabelBMFont::create("Allow Claiming\nFrom Others", "bigFont.fnt");
    allowStealFromLabel->setScale(0.35f);
    allowStealFromLabel->setAnchorPoint({0.f, 0.5f});
    allowStealFromLabel->setPosition({m_mainLayer->getContentWidth() * 0.25f - 10.f, 0.f});
    allowStealFromLabel->setID("allow-steal-from-label");

    bBoolsMenu->addChild(allowStealFromToggle);
    bBoolsMenu->addChild(allowStealFromLabel);

    auto ignoreInputToggle =
            CCMenuItemExt::createTogglerWithStandardSprites(0.7f, [&, this](CCMenuItemToggler* toggler) {
                getButtonData().m_clickAction.ignoreInput = !toggler->isToggled();
                Log.i("popup", "ignore input toggle");
            });
    ignoreInputToggle->toggle(bd.ignoreInput);
    ignoreInputToggle->setPosition({m_mainLayer->getContentWidth() * 0.75f - 10.f, 30.f});
    ignoreInputToggle->setID("ignore-input-toggle");

    auto ignoreInputLabel = CCLabelBMFont::create("Ignore Input", "bigFont.fnt");
    ignoreInputLabel->setScale(0.35f);
    ignoreInputLabel->setAnchorPoint({0.f, 0.5f});
    ignoreInputLabel->setPosition({m_mainLayer->getContentWidth() * 0.75f + 20.f, 30.f});
    ignoreInputLabel->setID("ignore-input-label");

    bBoolsMenu->addChild(ignoreInputLabel);
    bBoolsMenu->addChild(ignoreInputToggle);

    auto jumpInputToggle =
            CCMenuItemExt::createTogglerWithStandardSprites(0.7f, [&, this](CCMenuItemToggler* toggler) {
                bool jumpNewState = !toggler->isToggled();
                if(jumpNewState && getButtonData().m_clickAction.ignoreInput) {
                    getButtonData().m_clickAction.ignoreInput = false;
                }
                getButtonData().m_clickAction.jump = jumpNewState;
                Log.i("popup", "ignore input toggle");
            });
    jumpInputToggle->toggle(bd.jump);
    jumpInputToggle->setPosition({m_mainLayer->getContentWidth() * 0.75f - 10.f, 0});
    jumpInputToggle->setID("ignore-input-toggle");

    auto jumpInputLabel = CCLabelBMFont::create("P1 Force jump", "bigFont.fnt");
    jumpInputLabel->setScale(0.35f);
    jumpInputLabel->setAnchorPoint({0.f, 0.5f});
    jumpInputLabel->setPosition({m_mainLayer->getContentWidth() * 0.75f + 20.f, 0});
    jumpInputLabel->setID("ignore-input-label");

    bBoolsMenu->addChild(jumpInputToggle);
    bBoolsMenu->addChild(jumpInputLabel);

    buttonTabContainer->addChild(bBoolsMenu);

    m_mainLayer->addChild(buttonTabContainer);
    m_buttonToggler.tabNodes.push_back(buttonTabContainer);

    // TOUCH TAB

    auto touchTabContainer = CCNode::create();
    touchTabContainer->setContentSize(m_mainLayer->getContentSize());
    touchTabContainer->setAnchorPoint({0.f, 0.f});
    touchTabContainer->ignoreAnchorPointForPosition(false);
    touchTabContainer->setID("touch-container");

    float LEFT_X = 25;
    float RIGHT_LEFT_X = 190;
    float RIGHT_RIGHT_X = 320;
    constexpr float Y_START = 140;
    constexpr float Y_DIFFERENCE = 52;

    auto& td = getTouchData().m_touchAction;

    auto touchIdInput = createIntegerInput(
            "Touch ID:", &td.touch_id,
            {LEFT_X, touchTabContainer->getContentHeight() - Y_START});
    auto lockGroupInput = createIntegerInput(
            "Lock Group:", &td.groupIdLockObjectsToTouch,
            {LEFT_X, touchTabContainer->getContentHeight() - Y_START - Y_DIFFERENCE});

    auto downGroupInput = createIntegerInput(
            "Press Group:", &td.groupIdTouchDown,
            {RIGHT_LEFT_X, touchTabContainer->getContentHeight() - Y_START});
    auto upGroupInput = createIntegerInput(
            "Release Group:", &td.groupIdTouchUp,
            {RIGHT_RIGHT_X, touchTabContainer->getContentHeight() - Y_START});

    auto itemXInput = createIntegerInput(
            "Item ID X:", &td.itemId_x,
            {RIGHT_LEFT_X, touchTabContainer->getContentHeight() - Y_START - Y_DIFFERENCE});
    auto itemYInput = createIntegerInput(
            "Item ID Y:", &td.itemId_y,
            {RIGHT_RIGHT_X, touchTabContainer->getContentHeight() - Y_START - Y_DIFFERENCE});

    auto deltaXInput = createIntegerInput(
            "Delta X:", &td.itemId_deltaX,
            {RIGHT_LEFT_X, touchTabContainer->getContentHeight() - Y_START - (Y_DIFFERENCE * 2)});
    auto deltaYInput = createIntegerInput(
            "Delta Y:", &td.itemId_deltaY,
            {RIGHT_RIGHT_X, touchTabContainer->getContentHeight() - Y_START - (Y_DIFFERENCE * 2)});

    touchTabContainer->addChild(touchIdInput);
    touchTabContainer->addChild(lockGroupInput);
    touchTabContainer->addChild(downGroupInput);
    touchTabContainer->addChild(upGroupInput);
    touchTabContainer->addChild(itemXInput);
    touchTabContainer->addChild(itemYInput);
    touchTabContainer->addChild(deltaXInput);
    touchTabContainer->addChild(deltaYInput);

    m_mainLayer->addChild(touchTabContainer);
    m_touchToggler.tabNodes.push_back(touchTabContainer);

    reaction::action([this]() {
        Tab toggledTab = m_currentActionTab();
        Log.i("popup", "REACTION TOGGLED TAB: {}", enchantum::to_string(toggledTab));

        for (auto& toggler : {std::ref(m_keyboardToggler), std::ref(m_mouseToggler), std::ref(m_buttonToggler), std::ref(m_touchToggler)}) {
            for (const auto& nodes : toggler.get().tabNodes) {
                nodes->setVisible(false);
            }

            if (toggledTab != Tab::Keyboard && &toggler.get() == &m_keyboardToggler) {
                toggler.get().toggler->toggle(false);
                toggler.get().toggler->setEnabled(true);
            } else if (toggledTab != Tab::Mouse && &toggler.get() == &m_mouseToggler) {
                toggler.get().toggler->toggle(false);
                toggler.get().toggler->setEnabled(true);
            } else if (toggledTab != Tab::Button && &toggler.get() == &m_buttonToggler) {
                toggler.get().toggler->toggle(false);
                toggler.get().toggler->setEnabled(true);
            } else if (toggledTab != Tab::Touch && &toggler.get() == &m_touchToggler) {
                toggler.get().toggler->toggle(false);
                toggler.get().toggler->setEnabled(true);
            }
        }

        TabToggler& activeToggler = getToggler(toggledTab);
        for (const auto& nodes : activeToggler.tabNodes) {
            Log.i("popup", "nodeID: {}", nodes->getID());
            nodes->setVisible(true);
        }

        Log.i("popup", "clickable false on: {}", activeToggler.toggler);
        activeToggler.toggler->setEnabled(false);
        activeToggler.toggler->toggle(true);
    });

    return true;
}


CCNode* InputTriggerPopup::createMouseToggler(const char* label, LevelKeys key, float buttonScale, float labelScale) {
    auto checkboxContainer = CCNode::create();
    checkboxContainer->ignoreAnchorPointForPosition(false);
    checkboxContainer->setAnchorPoint({0.f, 0.5f});
    checkboxContainer->setID("checkbox-container");

    auto checkboxMenu = CCMenu::create();
    checkboxMenu->ignoreAnchorPointForPosition(false);
    checkboxMenu->setAnchorPoint({0.f, 0.5f});
    checkboxMenu->setID("checkbox-menu");

    auto& mouseData = getMouseData();
    auto toggler = CCMenuItemExt::createTogglerWithStandardSprites(buttonScale, [&mouseData, key](CCMenuItemToggler* self) {
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
    checkboxMenu->setPosition({0.f, checkboxContainer->getContentHeight() / 2});

    toggler->setPosition(checkboxMenu->getContentSize() / 2);

    checkboxContainer->addChild(checkboxMenu);

    auto checkboxLabel = CCLabelBMFont::create(label, "bigFont.fnt");

    checkboxLabel->setScale(labelScale);
    checkboxLabel->setAnchorPoint({0.f, 0.5f});
    checkboxLabel->setPosition({checkboxMenu->getContentWidth() + 10.f, checkboxContainer->getContentHeight() / 2});

    checkboxContainer->addChild(checkboxLabel);

    checkboxContainer->setContentWidth(
            checkboxMenu->getScaledContentWidth() + 10.f + checkboxLabel->getScaledContentWidth() + 10.f);

    toggler->setUserObject("key"_spr, ObjWrapper<LevelKeys>::create(key));
    toggler->setID(fmt::format("{}-toggle", enchantum::to_string(key)));


    return checkboxContainer;
}

CCMenuItemToggler* InputTriggerPopup::createTabToggler(const char* label, Tab tab) {
    auto onSpr = CCScale9Sprite::create("GJ_button_02.png");
    auto offSpr = CCScale9Sprite::create("GJ_button_04.png");

    auto keyLabelOn = CCLabelBMFont::create(label, "bigFont.fnt");
    keyLabelOn->setScale(0.5f);

    onSpr->setContentSize({keyLabelOn->getScaledContentWidth() + 20, keyLabelOn->getScaledContentHeight() + 10});
    onSpr->addChild(keyLabelOn);

    keyLabelOn->setPosition(onSpr->getContentSize() * 0.5f + CCPoint{0.f, 1.f});

    auto keyLabelOff = CCLabelBMFont::create(label, "bigFont.fnt");
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
InputTriggerPopup::createKeyboardToggler(LevelKeys key, float width, const std::string& labelOverride, const char* spr, bool flipX, bool flipY) {
    auto onSpr = CCScale9Sprite::create("GJ_button_02.png");
    onSpr->setContentSize({width, 40});
    auto offSpr = CCScale9Sprite::create("GJ_button_04.png");
    offSpr->setContentSize({width, 40});

    std::string keyStr = labelOverride.empty() ? std::string(fixKeyName(enchantum::to_string(key))) : std::move(labelOverride);

    CCSprite* alternativeSpr = spr ? CCSprite::createWithSpriteFrameName(spr) : nullptr;
    if (alternativeSpr) {
        if (flipX) {
            alternativeSpr->setFlipX(true);
        }
        if (flipY) {
            alternativeSpr->setFlipY(true);
        }
    }

    CCNode* spriteOnTop = alternativeSpr ? alternativeSpr : (CCNode*)CCLabelBMFont::create(keyStr.c_str(), "bigFont.fnt");
    if(!alternativeSpr) spriteOnTop->setScale(0.5f);
    spriteOnTop->setPosition(onSpr->getContentSize() * 0.5f);

    onSpr->setScale(0.6f);
    onSpr->addChild(spriteOnTop);


    CCSprite* alternativeSpr2 = spr ? CCSprite::createWithSpriteFrameName(spr) : nullptr;
    if (alternativeSpr2) {
        if (flipX) {
            alternativeSpr2->setFlipX(true);
        }
        if (flipY) {
            alternativeSpr2->setFlipY(true);
        }
    }


    auto spriteOnTop2 = alternativeSpr2 ? alternativeSpr2 : (CCNode*)CCLabelBMFont::create(keyStr.c_str(), "bigFont.fnt");
    
    if(!alternativeSpr2) spriteOnTop2->setScale(0.5f);
    spriteOnTop2->setPosition(offSpr->getContentSize() * 0.5f);

    offSpr->setScale(0.6f);
    offSpr->addChild(spriteOnTop2);

    auto& kbData = getKeyboardData();
    auto toggler = CCMenuItemExt::createToggler(onSpr, offSpr, [&kbData, key](CCMenuItemToggler* self) {
        updateButtonPressed(&kbData.pressed, self);
        kbData.m_keyAction.key = key;
        Log.i("popup", "toggled key button: {}", enchantum::to_string(key));
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
    groupInputBG->setID("group-input-bg");
    groupInputContainer->addChild(groupInputBG);

    auto groupInputLabel = CCLabelBMFont::create(labelText, "goldFont.fnt");
    groupInputLabel->setScale(0.56f);
    groupInputLabel->setAnchorPoint({0.5f, 0.f});
    groupInputLabel->setID("group-input-label");
    groupInputContainer->addChild(groupInputLabel);

    auto input = geode::TextInput::create(48, "Num");
    input->setID("group-input");
    if (*valuePtr > 0) {
        input->setString(geode::utils::numToString(*valuePtr));
    }


    input->setCommonFilter(geode::CommonFilter::Int);
    input->setMaxCharCount(5);
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
        input->setString(geode::utils::numToString(*valuePtr));
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
    menu->setID("integer-input-menu");
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
        case Tab::Button: label = getButtonData().m_clickAction.getLabel(); break;
        case Tab::Touch: label = getTouchData().m_touchAction.getLabel(); break;
    }
    Log.i("popup", "saving: {}", label);

    m_object->updateTextObject(label, false);


    Popup::onClose(sender);
}
