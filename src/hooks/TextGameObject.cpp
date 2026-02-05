#include "TextGameObject.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/TextGameObject.hpp>
#include <enchantum/enchantum.hpp>
#include <string_view>
#include "../TextParsing.hpp"
#include "LevelKeys.hpp"


using namespace geode::prelude;

void MyTextGameObject::setupInputTrigger() {
    if (!Mod::get()->getSettingValue<bool>("trigger-ui")) {
        return;
    }

    auto view = std::string_view(m_text);
    if (!view.starts_with("inf_inp:"))
        return;

    // treat like trigger :3 (aka move out of batch node and not allow color to be set normally)
    m_addToNodeContainer = true;
    m_detailColor = nullptr;
    m_isTrigger = true;

    for (auto child : getChildrenExt()) {
        child->setVisible(false);
    }

    if (LevelEditorLayer::get()) {
        auto parsed_opt = getParsedKeyAction(m_text);
        std::string labelStr = "";

        if (parsed_opt) {
            auto parsed = *parsed_opt;
            if (parsed.key != LevelKeys::editorTab) {
                labelStr = utils::numToString(parsed.group);
            }
        }

        auto labelContainer = CCNodeRGBA::create();
        labelContainer->ignoreAnchorPointForPosition(false);
        labelContainer->setAnchorPoint({0.5f, 0.5f});
        labelContainer->setScale(0.5f);
        labelContainer->setZOrder(1);
        labelContainer->setCascadeOpacityEnabled(true);

        auto groupLabel = CCLabelBMFont::create(labelStr.c_str(), "bigFont.fnt");

        labelContainer->setContentSize(groupLabel->getContentSize());
        groupLabel->setPosition(labelContainer->getContentSize() * 0.5f);

        labelContainer->addChild(groupLabel);
        addChild(labelContainer);

        auto spr = CCSprite::create("input_trigger.png"_spr);

        setContentSize(spr->getContentSize());
        m_width = getContentWidth();
        m_height = getContentHeight();
        updateOrientedBox();

        spr->setPosition(getContentSize() * 0.5f);
        labelContainer->setPosition({getContentWidth() * 0.5f, 10.5f});

        addChild(spr);

        spr->setColor(getColor());
    }
}

void MyTextGameObject::customObjectSetup(gd::vector<gd::string>& p0, gd::vector<void*>& p1) {
    TextGameObject::customObjectSetup(p0, p1);
    if (!Mod::get()->getSettingValue<bool>("trigger-ui")) {
        return;
    }
    setupInputTrigger();
}

void MyTextGameObject::updateTextObject(gd::string p0, bool p1) {
    TextGameObject::updateTextObject(p0, p1);
    if (!Mod::get()->getSettingValue<bool>("trigger-ui")) {
        return;
    }
    setupInputTrigger();
}
