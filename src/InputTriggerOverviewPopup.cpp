#include "InputTriggerOverviewPopup.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/EditorUI.hpp>
#include <Geode/binding/TextGameObject.hpp>
#include <Geode/ui/ScrollLayer.hpp>

#include <enchantum/enchantum.hpp>
#include "LevelKeys.hpp"

using namespace geode::prelude;

static std::string keyDisplayName(LevelKeys key) {
    return std::string(fixKeyName(enchantum::to_string(key)));
}

InputTriggerOverviewPopup* InputTriggerOverviewPopup::create() {
    auto ret = new InputTriggerOverviewPopup();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

static void buildTransposedContent(
    ScrollLayer* scroll,
    const std::vector<std::vector<std::string>>& rows
) {
    constexpr float rowH = 18.f;
    auto content = scroll->m_contentLayer;
    auto totalH = rowH * rows.size() + 4.f;
    content->setContentSize({scroll->getContentWidth(), totalH});

    for (size_t i = 0; i < rows.size(); i++) {
        auto bg = CCLayerColor::create();
        bg->setContentSize({scroll->getContentWidth(), rowH - 1});
        bg->setAnchorPoint({0, 0});
        bg->setPosition({0, totalH - (i + 1) * rowH});
        bg->setColor({0, 0, 0});
        bg->setOpacity((i % 2 == 0) ? 40 : 15);
        content->addChild(bg);

        auto rowNode = CCNode::create();
        rowNode->setContentSize({scroll->getContentWidth(), 14});
        float x = 6.f;
        for (auto& val : rows[i]) {
            auto label = CCLabelBMFont::create(val.c_str(), "bigFont.fnt");
            label->setScale(0.3f);
            label->setAnchorPoint({0, 0});
            label->setPosition({x, 0});
            rowNode->addChild(label);
            x += 80.f;
        }
        rowNode->setPositionY(totalH - (i + 1) * rowH + rowH * 0.5f);
        content->addChild(rowNode);
    }

    scroll->moveToTop();
}

bool InputTriggerOverviewPopup::init() {
    if (!Popup::init(420.f, 310.f)) {
        return false;
    }

    m_noElasticity = true;
    m_closeBtn->removeFromParent();

    auto title = CCLabelBMFont::create("Input Overview", "bigFont.fnt");
    title->setScale(0.5f);
    title->setPosition({m_mainLayer->getContentWidth() * 0.5f, m_mainLayer->getContentHeight() - 14});
    m_mainLayer->addChild(title);

    auto okSpr = ButtonSprite::create("OK", 40, 0, 0.8f, true, "goldFont.fnt", "GJ_button_01.png", 30.0);
    auto okBtn = CCMenuItemSpriteExtra::create(okSpr, this, menu_selector(InputTriggerOverviewPopup::onClose));
    okBtn->setPosition({m_mainLayer->getContentWidth() * 0.5f, 22});
    m_buttonMenu->addChild(okBtn);

    collectData();

    float scrollW = 380.f;
    float scrollH = 150.f;
    float leftX = m_mainLayer->getContentWidth() * 0.5f - scrollW * 0.5f;

    bool hasAny = false;
    float y = m_mainLayer->getContentHeight() - 32.f;

    auto addKeyHeaders = [&](const std::vector<std::string>& names) {
        float x = leftX + 6.f;
        for (auto& name : names) {
            auto label = CCLabelBMFont::create(name.c_str(), "goldFont.fnt");
            label->setScale(0.3f);
            label->setAnchorPoint({0, 0});
            label->setPosition({x, y + 4.f});
            m_mainLayer->addChild(label);
            x += 80.f;
        }
    };

    auto addSection = [&](const char* titleStr,
                          const std::vector<std::string>& idents,
                          const std::vector<std::vector<std::string>>& dataRows,
                          const char* id) {
        hasAny = true;

        y -= 14.f;
        auto label = CCLabelBMFont::create(titleStr, "goldFont.fnt");
        label->setScale(0.4f);
        label->setAnchorPoint({0, 0});
        label->setPosition({leftX, y + 4.f});
        m_mainLayer->addChild(label);

        y -= 14.f;
        addKeyHeaders(idents);

        y -= scrollH;
        auto scroll = ScrollLayer::create({scrollW, scrollH});
        scroll->setAnchorPoint({0, 0});
        scroll->setPosition({leftX, y});
        scroll->enableScrollWheel(true);
        scroll->setID(id);
        m_mainLayer->addChild(scroll);

        buildTransposedContent(scroll, dataRows);

        y -= 2.f;
    };

    if (!m_mergedKeys.empty()) {
        std::vector<std::string> idents;
        for (auto& mk : m_mergedKeys)
            idents.push_back(keyDisplayName(mk.key));

        std::vector<std::vector<std::string>> rows;
        std::vector<std::string> downRow;
        for (auto& mk : m_mergedKeys)
            downRow.push_back(mk.downGroup ? std::to_string(*mk.downGroup) : "-");
        rows.push_back(downRow);

        std::vector<std::string> upRow;
        for (auto& mk : m_mergedKeys)
            upRow.push_back(mk.upGroup ? std::to_string(*mk.upGroup) : "-");
        rows.push_back(upRow);

        addSection("Keys", idents, rows, "keys-scroll");
    }

    if (!m_variables.empty()) {
        std::vector<std::string> idents;
        for (auto& v : m_variables)
            idents.push_back(keyDisplayName(v.key));

        std::vector<std::vector<std::string>> rows;
        std::vector<std::string> groupRow;
        for (auto& v : m_variables)
            groupRow.push_back(std::to_string(v.group));
        rows.push_back(groupRow);

        addSection("Variables", idents, rows, "var-scroll");
    }

    if (!m_buttons.empty()) {
        std::vector<std::string> idents;
        for (auto& b : m_buttons)
            idents.push_back(std::to_string(b.collisionBlockId));

        std::vector<std::vector<std::string>> rows;
        std::vector<std::string> enterRow;
        for (auto& b : m_buttons) enterRow.push_back(std::to_string(b.groupIdCursorEnter));
        rows.push_back(enterRow);

        std::vector<std::string> exitRow;
        for (auto& b : m_buttons) exitRow.push_back(std::to_string(b.groupIdCursorExit));
        rows.push_back(exitRow);

        std::vector<std::string> downRow;
        for (auto& b : m_buttons) downRow.push_back(std::to_string(b.groupIdCursorDown));
        rows.push_back(downRow);

        std::vector<std::string> upRow;
        for (auto& b : m_buttons) upRow.push_back(std::to_string(b.groupIdCursorUp));
        rows.push_back(upRow);

        std::vector<std::string> stealRow;
        for (auto& b : m_buttons) stealRow.push_back(b.stealTouches ? "Y" : "N");
        rows.push_back(stealRow);

        std::vector<std::string> allowRow;
        for (auto& b : m_buttons) allowRow.push_back(b.allowStealFrom ? "Y" : "N");
        rows.push_back(allowRow);

        addSection("Buttons", idents, rows, "btn-scroll");
    }

    if (!m_touches.empty()) {
        std::vector<std::string> idents;
        for (auto& t : m_touches)
            idents.push_back(std::to_string(t.touch_id));

        std::vector<std::vector<std::string>> rows;
        std::vector<std::string> lockRow;
        for (auto& t : m_touches) lockRow.push_back(std::to_string(t.groupIdLockObjectsToTouch));
        rows.push_back(lockRow);

        std::vector<std::string> downRow;
        for (auto& t : m_touches) downRow.push_back(std::to_string(t.groupIdTouchDown));
        rows.push_back(downRow);

        std::vector<std::string> upRow;
        for (auto& t : m_touches) upRow.push_back(std::to_string(t.groupIdTouchUp));
        rows.push_back(upRow);

        std::vector<std::string> itemXRow;
        for (auto& t : m_touches) itemXRow.push_back(std::to_string(t.itemId_x));
        rows.push_back(itemXRow);

        std::vector<std::string> itemYRow;
        for (auto& t : m_touches) itemYRow.push_back(std::to_string(t.itemId_y));
        rows.push_back(itemYRow);

        std::vector<std::string> deltaXRow;
        for (auto& t : m_touches) deltaXRow.push_back(std::to_string(t.itemId_deltaX));
        rows.push_back(deltaXRow);

        std::vector<std::string> deltaYRow;
        for (auto& t : m_touches) deltaYRow.push_back(std::to_string(t.itemId_deltaY));
        rows.push_back(deltaYRow);

        addSection("Touch Actions", idents, rows, "touch-scroll");
    }

    if (!hasAny) {
        auto noData = CCLabelBMFont::create("No input triggers found", "bigFont.fnt");
        noData->setScale(0.4f);
        noData->setPosition({m_mainLayer->getContentWidth() * 0.5f, m_mainLayer->getContentHeight() * 0.5f});
        noData->setAnchorPoint({0.5f, 0.5f});
        noData->setID("no-data-label");
        m_mainLayer->addChild(noData);
    }

    return true;
}

void InputTriggerOverviewPopup::onClose(CCObject* sender) {
    Popup::onClose(sender);
}

bool InputTriggerOverviewPopup::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    return false;
}

void InputTriggerOverviewPopup::collectData() {
    auto ui = EditorUI::get();
    if (!ui || !ui->m_editorLayer) return;

    std::vector<KeyAction> keys;

    for (auto obj : CCArrayExt<GameObject*>(ui->m_editorLayer->m_objects)) {
        if (obj->m_objectID != 914) continue;

        auto text = static_cast<TextGameObject*>(obj)->m_text;
        if (auto parsed = parseObjectString(text)) {
            if (auto ka = std::get_if<KeyAction>(&*parsed)) {
                keys.push_back(*ka);
            } else if (auto sa = std::get_if<SimpleKeyAction>(&*parsed)) {
                m_variables.push_back(*sa);
            } else if (auto ca = std::get_if<ClickAction>(&*parsed)) {
                m_buttons.push_back(*ca);
            } else if (auto ta = std::get_if<TouchAction>(&*parsed)) {
                m_touches.push_back(*ta);
            }
        }
    }

    std::unordered_map<LevelKeys, MergedKey> mergedMap;
    for (auto& k : keys) {
        auto& mk = mergedMap[k.key];
        mk.key = k.key;
        if (k.keyDown) {
            mk.downGroup = k.group;
        } else {
            mk.upGroup = k.group;
        }
    }

    for (auto& [_, mk] : mergedMap) {
        m_mergedKeys.push_back(mk);
    }
}
