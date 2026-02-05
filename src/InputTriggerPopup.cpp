#include "InputTriggerPopup.hpp"
#include <Geode/Geode.hpp>
#include <algorithm>
#include <enchantum/enchantum.hpp>
#include <fmt/format.h>
#include "Geode/ui/Popup.hpp"


using namespace geode::prelude;

InputTriggerPopup* InputTriggerPopup::create(std::vector<geode::Ref<GameObject>> objects) {
    auto ret = new InputTriggerPopup();
    if (ret && ret->init(objects)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}


bool InputTriggerPopup::init(std::vector<geode::Ref<GameObject>> objects) {

    if (!Popup::init(440.f, 310.f)) {
        return false;
    }
    m_noElasticity = true;
    m_objects = objects;
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

    auto groupInputContainer = CCNode::create();
    groupInputContainer->setAnchorPoint({0, 0});
    groupInputContainer->ignoreAnchorPointForPosition(false);
    groupInputContainer->setContentSize({100, 60});
    groupInputContainer->setPosition({50, 20});
    groupInputContainer->setID("group-input-container");

    auto groupInputBG = CCScale9Sprite::create("square02_small.png");
    groupInputBG->setScale(0.8f);
    groupInputBG->setContentSize({70, 30});
    groupInputBG->setZOrder(-1);
    groupInputBG->setOpacity(100);
    groupInputBG->setID("group-input-background");

    groupInputContainer->addChild(groupInputBG);

    auto groupInputLabel = CCLabelBMFont::create("Group ID:", "goldFont.fnt");
    groupInputLabel->setScale(0.56f);
    groupInputLabel->setAnchorPoint({0.5f, 0.f});
    groupInputLabel->setID("group-input-label");

    groupInputContainer->addChild(groupInputLabel);

    m_groupInput = geode::TextInput::create(48, "Num");
    m_groupInput->setCallback([this](const std::string& str) {
        auto numRes = geode::utils::numFromString<int>(str);
        m_label.group = numRes.unwrapOrDefault();
        m_modifiedGroup = true;
    });
    m_groupInput->setCommonFilter(geode::CommonFilter::Int);
    m_groupInput->setMaxCharCount(4);
    m_groupInput->hideBG();
    m_groupInput->setID("group-input");

    m_groupInput->setPosition({groupInputContainer->getContentWidth() * 0.5f, m_groupInput->getContentHeight() * 0.5f});
    groupInputBG->setPosition(m_groupInput->getPosition());
    groupInputLabel->setPosition(
            {groupInputContainer->getContentWidth() * 0.5f,
             m_groupInput->getPositionY() + m_groupInput->getContentHeight() * 0.5f + 5});

    groupInputContainer->addChild(m_groupInput);

    auto groupInputBounds = m_groupInput->boundingBox();

    auto decrGroupBtn = CCMenuItemExt::createSpriteExtraWithFrameName(
            "edit_leftBtn_001.png", 1.f, [this](CCMenuItemSpriteExtra* self) {
                m_label.group = std::clamp(m_label.group - 1, 0, 9999);
                m_groupInput->setString(geode::utils::numToString(m_label.group));
                m_modifiedGroup = true;
            });

    decrGroupBtn->setPosition({groupInputBounds.getMinX() - 25, m_groupInput->getPositionY()});
    decrGroupBtn->setID("decrement-group-button");

    auto incrGroupBtn = CCMenuItemExt::createSpriteExtraWithFrameName(
            "edit_rightBtn_001.png", 1.f, [this](CCMenuItemSpriteExtra* self) {
                m_label.group = std::clamp(m_label.group + 1, 0, 9999);
                m_groupInput->setString(geode::utils::numToString(m_label.group));
                m_modifiedGroup = true;
            });

    incrGroupBtn->setPosition({groupInputBounds.getMaxX() + 25, m_groupInput->getPositionY()});
    incrGroupBtn->setID("increment-group-button");

    auto groupMenu = CCMenu::create();
    groupMenu->ignoreAnchorPointForPosition(false);
    groupMenu->setAnchorPoint({0, 0});
    groupMenu->setContentSize(groupInputContainer->getContentSize());
    groupMenu->setPosition({0, 0});
    groupMenu->setID("group-input-menu");

    groupInputContainer->addChild(groupMenu);

    groupMenu->addChild(decrGroupBtn);
    groupMenu->addChild(incrGroupBtn);

    m_mainLayer->addChild(groupInputContainer);

    m_onReleaseToggle = CCMenuItemExt::createTogglerWithStandardSprites(0.7f, [this](CCMenuItemToggler* toggler) {
        m_label.keyDown = toggler->isToggled();
        m_modifiedRelease = true;
    });

    m_onReleaseToggle->setPosition({m_mainLayer->getContentWidth() - 120, 55});
    m_onReleaseToggle->setID("on-release-toggle");

    m_buttonMenu->addChild(m_onReleaseToggle);

    m_onReleaseLabel = CCLabelBMFont::create("Trigger On\nRelease", "bigFont.fnt");
    m_onReleaseLabel->setScale(0.35f);
    m_onReleaseLabel->setAnchorPoint({0.f, 0.5f});
    m_onReleaseLabel->setPosition(
            {m_onReleaseToggle->getPositionX() + m_onReleaseToggle->getContentWidth() * 0.5f + 10,
             m_onReleaseToggle->getPositionY()});
    m_onReleaseLabel->setID("on-release-label");

    m_mainLayer->addChild(m_onReleaseLabel);

    auto tabsMenu = CCMenu::create();
    tabsMenu->ignoreAnchorPointForPosition(false);
    tabsMenu->setAnchorPoint({0.5f, 1.f});
    tabsMenu->setContentSize({m_mainLayer->getContentWidth(), 40});
    tabsMenu->setPosition({m_mainLayer->getContentWidth() * 0.5f, m_mainLayer->getContentHeight() - 32});
    tabsMenu->setID("tabs-menu");

    auto tabsLayout = RowLayout::create();

    tabsMenu->setLayout(tabsLayout);

    m_mainLayer->addChild(tabsMenu);

    tabsMenu->addChild(createTabToggler("Keyboard", 0));
    tabsMenu->addChild(createTabToggler("Mouse", 1));

    tabsMenu->updateLayout();

    m_onModLoadedToggle = CCMenuItemExt::createTogglerWithStandardSprites(0.7f, [this](CCMenuItemToggler* toggler) {
        for (const auto& [toggle, tab] : m_toggles) {
            if (toggle == toggler)
                continue;
            toggle->toggle(false);
        }
        m_label.key = !toggler->isToggled() ? LevelKeys::modLoaded : LevelKeys::empty;
        m_modifiedKey = true;
    });

    m_onModLoadedToggle->setPosition({m_mainLayer->getContentWidth() - 120, 25});

    m_onModLoadedToggle->setUserObject("key"_spr, ObjWrapper<LevelKeys>::create(LevelKeys::modLoaded));
    m_onModLoadedToggle->setID("on-mod-loaded-toggle");
    m_toggles[m_onModLoadedToggle] = 0;

    m_buttonMenu->addChild(m_onModLoadedToggle);

    auto onModLoadedLabel = CCLabelBMFont::create("On Mod\nLoaded", "bigFont.fnt");
    onModLoadedLabel->setScale(0.35f);
    onModLoadedLabel->setAnchorPoint({0.f, 0.5f});
    onModLoadedLabel->setPosition(
            {m_onModLoadedToggle->getPositionX() + m_onModLoadedToggle->getContentWidth() * 0.5f + 10,
             m_onModLoadedToggle->getPositionY()});
    onModLoadedLabel->setID("on-mod-loaded-label");

    m_mainLayer->addChild(onModLoadedLabel);

    setupKeyboardTab();
    setupMouseTab();

    for (const auto& tab : m_tabs) {
        tab->setVisible(false);
    }

    setupValues();

    return true;
}

void InputTriggerPopup::setupValues() {

    auto parsed_opt = getParsedKeyAction(static_cast<TextGameObject*>(m_objects[0].data())->m_text);

    if (parsed_opt) {
        auto parsed = *parsed_opt;

        bool mixedGroups = false;
        bool mixedKey = false;
        bool mixedKeyDown = false;
        for (const auto& obj : m_objects) {
            auto parsedObj_opt = getParsedKeyAction(static_cast<TextGameObject*>(obj.data())->m_text);
            auto parsedObj = *parsedObj_opt;

            if (parsed.group != parsedObj.group) {
                mixedGroups = true;
            }
            if (parsed.key != parsedObj.key) {
                mixedKey = true;
            }
            if (parsed.keyDown != parsedObj.keyDown) {
                mixedKeyDown = true;
            }
        }
        if (!mixedGroups) {
            m_groupInput->setString(geode::utils::numToString(parsed.group));
            m_label.group = parsed.group;
        }

        bool mainTab = true;

        if (!mixedKey) {
            m_label.key = parsed.key;
            for (const auto& [toggle, tab] : m_toggles) {
                auto key = static_cast<ObjWrapper<LevelKeys>*>(toggle->getUserObject("key"_spr));

                if (key->getValue() == parsed.key) {
                    toggle->toggle(true);
                    m_tabs[tab]->setVisible(true);
                    m_tabToggles[tab]->toggle(true);
                    mainTab = false;
                    break;
                }
            }
        }

        if (mainTab) {
            m_tabs[0]->setVisible(true);
            m_tabToggles[0]->toggle(true);
        }

        if (!mixedKeyDown) {
            m_onReleaseToggle->toggle(!parsed.keyDown);
            m_label.keyDown = parsed.keyDown;
        }
    }

    checkSpecialKey();
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

    auto toggler = CCMenuItemExt::createTogglerWithStandardSprites(0.7f, [this, key](CCMenuItemToggler* self) {
        for (const auto& [toggle, tab] : m_toggles) {
            if (toggle == self)
                continue;
            toggle->toggle(false);
        }
        m_label.key = !self->isToggled() ? key : LevelKeys::empty;
        m_modifiedKey = true;
        checkSpecialKey();
    });


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
    m_toggles[toggler] = 1;

    return checkboxContainer;
}

CCMenuItemToggler* InputTriggerPopup::createTabToggler(const std::string& label, int tab) {
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

        for (const auto& toggle : m_tabToggles) {
            toggle->toggle(false);
            self->setClickable(true);
        }
        for (const auto& tab : m_tabs) {
            tab->setVisible(false);
        }
        m_tabs[tab]->setVisible(true);

        if (!wasToggled) {
            self->toggle(true);
            self->setClickable(false);
        }
    });

    toggler->setID(fmt::format("{}-toggle", label));

    m_tabToggles.push_back(toggler);

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
    } else
        keyStr = labelOverride;

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
        for (const auto& [toggle, tab] : m_toggles) {
            if (toggle == self)
                continue;
            toggle->toggle(false);
        }
        m_label.key = !self->isToggled() ? key : LevelKeys::empty;
        m_modifiedKey = true;

        checkSpecialKey();
    });

    toggler->setUserObject("key"_spr, ObjWrapper<LevelKeys>::create(key));
    toggler->setID(fmt::format("{}-toggle", keyStr));
    m_toggles[toggler] = 0;

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
    m_tabs.push_back(keyboardContainer);

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

    m_mainLayer->addChild(keyboardContainer);
}

void InputTriggerPopup::setupMouseTab() {
    auto mouseContainer = CCNode::create();
    mouseContainer->setContentSize(m_mainLayer->getContentSize());
    mouseContainer->setAnchorPoint({0.f, 0.f});
    mouseContainer->ignoreAnchorPointForPosition(false);
    mouseContainer->setID("mouse-container");
    m_tabs.push_back(mouseContainer);

    auto innerContainer = CCNode::create();
    innerContainer->setAnchorPoint({0.5f, 1.f});
    innerContainer->ignoreAnchorPointForPosition(false);
    innerContainer->setPosition({m_mainLayer->getContentWidth() * 0.5f, m_mainLayer->getContentHeight() - 90.f});
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

void InputTriggerPopup::checkSpecialKey() {
    switch (m_label.key) {
        case LevelKeys::wheelUp:
        case LevelKeys::wheelDown:
        case LevelKeys::cursor:
        case LevelKeys::modLoaded:
            m_onReleaseToggle->toggle(false);
            m_onReleaseToggle->setVisible(false);
            m_onReleaseLabel->setVisible(false);
            break;
        default:
            m_onReleaseToggle->setVisible(true);
            m_onReleaseLabel->setVisible(true);
            break;
    }
}

void InputTriggerPopup::onClose(CCObject* sender) {

    for (const auto& obj : m_objects) {
        auto textObj = static_cast<TextGameObject*>(obj.data());
        auto text = textObj->m_text;
        auto parsed_opt = getParsedKeyAction(text);
        if (parsed_opt) {
            auto parsed = *parsed_opt;
            if (m_modifiedGroup) {
                parsed.group = std::clamp(m_label.group, 0, 9999);
            }
            if (m_modifiedKey) {
                parsed.key = m_label.key;
            }
            if (m_modifiedRelease) {
                parsed.keyDown = m_label.keyDown;
            }
            auto label = getLabelFromKeyAction(parsed);
            textObj->updateTextObject(label, false);
        } else {
            auto label = getLabelFromKeyAction(m_label);
            textObj->updateTextObject(label, false);
        }
    }
    Popup::onClose(sender);
}
