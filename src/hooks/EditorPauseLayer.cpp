#include <Geode/binding/EditorPauseLayer.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <GlobalOverviewPopup.hpp>


class $modify(MyEditorPause, EditorPauseLayer) {

    void onOverview(CCObject* sender) { GlobalOverviewPopup::create()->show(); }

    bool init(LevelEditorLayer* layer) {
        if (!EditorPauseLayer::init(layer))
            return false;

        auto menu = getChildByIDRecursive("guidelines-menu");

        geode::log::info("menu: {}", menu);
        if (!menu) {
            return true;
        }
        auto overviewBtn = CCMenuItemSpriteExtra::create(
                CCSprite::create("input_trigger.png"_spr), this,
                menu_selector(MyEditorPause::onOverview));

        menu->addChild(overviewBtn);
        menu->updateLayout();

        return true;
    }
};
