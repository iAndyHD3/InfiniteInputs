#include "GlobalOverviewPopup.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/EditorUI.hpp>
#include <Geode/binding/EditorPauseLayer.hpp>
#include <Geode/binding/TextGameObject.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <algorithm>
#include <enchantum/enchantum.hpp>
#include <fmt/format.h>
#include "Geode/utils/general.hpp"
#include "LevelKeys.hpp"

using namespace geode::prelude;

GlobalOverviewPopup* GlobalOverviewPopup::create() {
    auto ret = new GlobalOverviewPopup();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool GlobalOverviewPopup::init() {
    if (!Popup::init(440.f, 280.f)) {
        return false;
    }

    m_noElasticity = true;

    // Title
    auto title = CCLabelBMFont::create(fmt::format("{} - Input Overview", LevelEditorLayer::get()->m_level->m_levelName).c_str(), "bigFont.fnt");
    title->setScale(0.5f);
    title->setPosition({m_mainLayer->getContentWidth() / 2, m_mainLayer->getContentHeight() - 14});
    title->setID("overview-title");
    m_mainLayer->addChild(title);

    // Parse all trigger actions from the level
    parseAllActions();

    // Content area for entries
    m_contentArea = CCNode::create();
    m_contentArea->setContentSize({m_mainLayer->getContentWidth() - 60, 260});
    m_contentArea->setAnchorPoint({0.5f, 1.f});
    m_contentArea->ignoreAnchorPointForPosition(false);
    m_contentArea->setPosition({m_mainLayer->getContentWidth() / 2, m_mainLayer->getContentHeight() - 30});
    m_contentArea->setID("overview-content");
    m_mainLayer->addChild(m_contentArea);

    // Empty label (hidden by default) — sibling of m_contentArea, not a child,
    // so removeAllChildren on m_contentArea won't release it.
    m_emptyLabel = CCLabelBMFont::create("No actions found.", "bigFont.fnt");
    m_emptyLabel->setScale(0.5f);
    m_emptyLabel->setPosition(m_mainLayer->getContentSize() / 2);
    m_emptyLabel->setID("overview-empty");
    m_mainLayer->addChild(m_emptyLabel);
    m_emptyLabel->setVisible(m_entries.empty());

    // Pagination: left arrow
    m_prevBtn = CCMenuItemExt::createSpriteExtraWithFrameName(
        "GJ_arrow_01_001.png", 0.75f, [this](CCMenuItemSpriteExtra*) {
            onPrev(nullptr);
        });
    m_prevBtn->setPosition({15, m_mainLayer->getContentHeight() / 2});
    m_prevBtn->setID("overview-prev-btn");
    m_buttonMenu->addChild(m_prevBtn);

    // Pagination: right arrow (flipped)
    auto nextSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    nextSpr->setFlipX(true);
    nextSpr->setScale(0.75f);
    m_nextBtn = CCMenuItemSpriteExtra::create(nextSpr, this, menu_selector(GlobalOverviewPopup::onNext));
    m_nextBtn->setPosition({m_mainLayer->getContentWidth() - 15, m_mainLayer->getContentHeight() / 2});
    m_nextBtn->setID("overview-next-btn");
    m_buttonMenu->addChild(m_nextBtn);

    // Build first page
    if (!m_entries.empty()) {
        rebuildUI();
    }

    return true;
}

void GlobalOverviewPopup::parseAllActions() {
    m_entries.clear();

    auto editor = LevelEditorLayer::get();
    if (!editor) return;

    for (auto obj : CCArrayExt<GameObject*>(editor->m_objects)) {
        if (obj->m_objectID != 914) continue;

        auto textObj = static_cast<TextGameObject*>(obj);
        std::string_view t = textObj->m_text;

        auto parsed = parseObjectString(t);
        if (!parsed) continue;

        ActionEntry entry;
        entry.m_textObject = textObj;

        if (auto keyAction = std::get_if<KeyAction>(&*parsed)) {
            entry.tabName = "Keyboard";
            entry.propertyNames = {"Key", "Down/Up", "Group"};
            entry.values = {
                std::string(fixKeyName(enchantum::to_string(keyAction->key))),
                keyAction->keyDown ? "Down" : "Up",
                geode::utils::numToString(keyAction->group)
            };
        } else if (auto simpleAction = std::get_if<SimpleKeyAction>(&*parsed)) {
            entry.tabName = "Mouse";
            entry.propertyNames = {"Key", "Group"};
            entry.values = {
                std::string(fixKeyName(enchantum::to_string(simpleAction->key))),
                geode::utils::numToString(simpleAction->group)
            };
        } else if (auto clickAction = std::get_if<ClickAction>(&*parsed)) {
            entry.tabName = "Button";
            entry.propertyNames = {
                "Collision", "Down", "Up", "Enter", "Exit", "Steal", "Allow"
            };
            entry.values = {
                geode::utils::numToString(clickAction->collisionBlockId),
                geode::utils::numToString(clickAction->groupIdCursorDown),
                geode::utils::numToString(clickAction->groupIdCursorUp),
                geode::utils::numToString(clickAction->groupIdCursorEnter),
                geode::utils::numToString(clickAction->groupIdCursorExit),
                clickAction->stealTouches ? "Yes" : "No",
                clickAction->allowStealFrom ? "Yes" : "No"
            };
        } else if (auto touchAction = std::get_if<TouchAction>(&*parsed)) {
            entry.tabName = "Touch";
            entry.propertyNames = {
                "TouchID", "Lock", "Down", "Up", "I.X", "I.Y", "D.X", "D.Y"
            };
            entry.values = {
                geode::utils::numToString(touchAction->touch_id),
                geode::utils::numToString(touchAction->groupIdLockObjectsToTouch),
                geode::utils::numToString(touchAction->groupIdTouchDown),
                geode::utils::numToString(touchAction->groupIdTouchUp),
                geode::utils::numToString(touchAction->itemId_x),
                geode::utils::numToString(touchAction->itemId_y),
                geode::utils::numToString(touchAction->itemId_deltaX),
                geode::utils::numToString(touchAction->itemId_deltaY)
            };
        }

        // Compute column X positions based on column count
        int numCols = (int)entry.propertyNames.size();
        float startX = 5;
        float spacing;
        if (numCols <= 2) spacing = 130;
        else if (numCols <= 3) spacing = 110;
        else if (numCols <= 7) spacing = 52;
        else spacing = 46;

        for (int i = 0; i < numCols; i++) {
            entry.columnXPositions.push_back(startX + i * spacing);
        }

        m_entries.push_back(std::move(entry));
    }

    // Sort entries: Keyboard → Mouse → Button → Touch
    auto tabPriority = [](const std::string& name) -> int {
        if (name == "Keyboard") return 0;
        if (name == "Mouse")    return 1;
        if (name == "Button")   return 2;
        if (name == "Touch")    return 3;
        return 4;
    };
    std::stable_sort(m_entries.begin(), m_entries.end(), [&](const ActionEntry& a, const ActionEntry& b) {
        return tabPriority(a.tabName) < tabPriority(b.tabName);
    });
}

void GlobalOverviewPopup::rebuildUI() {
    m_contentArea->removeAllChildren();
    m_emptyLabel->setVisible(false);

    if (m_entries.empty()) {
        m_emptyLabel->setVisible(true);
        m_prevBtn->setVisible(false);
        m_nextBtn->setVisible(false);
        return;
    }

    int totalPages = (int)std::ceil((float)m_entries.size() / m_perPage);
    if (m_currentPage >= totalPages) m_currentPage = totalPages - 1;
    if (m_currentPage < 0) m_currentPage = 0;

    int start = m_currentPage * m_perPage;
    int end = std::min(start + m_perPage, (int)m_entries.size());

    float yPos = m_contentArea->getContentHeight() - 5;
    std::string currentTabName;
    int rowIndex = 0; // for alternating row colours

    for (int i = start; i < end; i++) {
        auto& entry = m_entries[i];

        // Show gold name + property names only when the type changes
        if (entry.tabName != currentTabName) {
            currentTabName = entry.tabName;
            rowIndex = 0;

            // Gold name
            auto nameLabel = CCLabelBMFont::create(entry.tabName.c_str(), "goldFont.fnt");
            nameLabel->setScale(0.5f);
            nameLabel->setAnchorPoint({0, 1});
            nameLabel->setPosition({5, yPos});
            nameLabel->setID(fmt::format("action-name-{}", i));
            m_contentArea->addChild(nameLabel);
            yPos -= 16;

            // Property names (column headers)
            for (int j = 0; j < (int)entry.propertyNames.size(); j++) {
                auto propLabel = CCLabelBMFont::create(entry.propertyNames[j].c_str(), "bigFont.fnt");
                propLabel->setScale(0.3f);
                propLabel->setAnchorPoint({0, 1});
                propLabel->setPosition({entry.columnXPositions[j], yPos});
                propLabel->setColor({150, 150, 150});
                propLabel->setID(fmt::format("action-prop-{}-{}", i, j));
                m_contentArea->addChild(propLabel);
            }
            yPos -= 12;
        }

        // Alternating row background
        float rowH = 17;
        auto rowBg = CCLayerColor::create(
            rowIndex % 2 == 0 ? ccc4(255, 255, 255, 30) : ccc4(0, 0, 0, 20),
            m_contentArea->getContentWidth(), rowH
        );
        rowBg->setAnchorPoint({0, 0});
        rowBg->setPosition({0, yPos - rowH});
        rowBg->setZOrder(-1);
        rowBg->setID(fmt::format("action-bg-{}", i));
        m_contentArea->addChild(rowBg);
        rowIndex++;

        // View button at the right end of this row
        auto rowMenu = CCMenu::create();
        rowMenu->setPosition({0, 0});
        rowMenu->setContentSize(m_contentArea->getContentSize());
        rowMenu->setID(fmt::format("row-menu-{}", i));

        float valCenterY = yPos - rowH / 2;

        auto viewLabel = CCLabelBMFont::create("View", "bigFont.fnt");
        viewLabel->setScale(.35f);
        auto viewBtn = CCMenuItemSpriteExtra::create(viewLabel, this, menu_selector(GlobalOverviewPopup::onView));
        auto color = CCLayerColor::create({ 32, 67, 36, 99 }, viewBtn->getContentSize().width + 10, viewBtn->getContentSize().height + 4);
        color->setPosition({-5, -2});
        viewBtn->addChild(color, -5);


        viewBtn->setPosition({m_contentArea->getContentWidth() - 20, valCenterY});
        viewBtn->setUserObject("textObj"_spr, ObjWrapper<TextGameObject*>::create(entry.m_textObject));

        rowMenu->addChild(viewBtn);
        m_contentArea->addChild(rowMenu);

        // Values row (one per action)
        for (int j = 0; j < (int)entry.values.size(); j++) {
            auto valLabel = CCLabelBMFont::create(entry.values[j].c_str(), "bigFont.fnt");
            valLabel->setScale(0.3f);
            valLabel->setAnchorPoint({0, 0.5f});
            valLabel->setPosition({entry.columnXPositions[j], valCenterY});
            valLabel->setID(fmt::format("action-val-{}-{}", i, j));
            m_contentArea->addChild(valLabel);
        }
        yPos -= rowH;
    }

    // Page indicator

    // Arrow visibility
    m_prevBtn->setVisible(m_currentPage > 0);
    m_nextBtn->setVisible(m_currentPage < totalPages - 1);
}

void GlobalOverviewPopup::onPrev(CCObject*) {
    if (m_currentPage > 0) {
        m_currentPage--;
        rebuildUI();
    }
}

void GlobalOverviewPopup::onNext(CCObject*) {
    int totalPages = (int)std::ceil((float)m_entries.size() / m_perPage);
    if (m_currentPage < totalPages - 1) {
        m_currentPage++;
        rebuildUI();
    }
}

void GlobalOverviewPopup::onView(CCObject* sender) {
    auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
    if (auto wrapper = static_cast<ObjWrapper<TextGameObject*>*>(btn->getUserObject("textObj"_spr))) {
        auto obj = wrapper->getValue();
        if (auto editor = EditorUI::get()) {
            editor->deselectAll();
            editor->selectObject(obj, false);
            editor->centerCameraOnObject(obj);
        }
    }
    // Close this popup, then EditorPauseLayer if present
    keyBackClicked();
    auto pauseLayer = CCDirector::get()->getRunningScene()->getChildByIDRecursive("EditorPauseLayer");
    if (auto pl = typeinfo_cast<EditorPauseLayer*>(pauseLayer)) {
        pl->keyBackClicked();
    }
    EditorUI::get()->updateButtons();
}
