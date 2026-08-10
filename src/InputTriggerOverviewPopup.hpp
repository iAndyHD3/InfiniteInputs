#pragma once

#include <Geode/ui/Popup.hpp>
#include "TextParsing.hpp"

class InputTriggerOverviewPopup : public geode::Popup {
public:
    static InputTriggerOverviewPopup* create();

protected:
    bool init() override;
    void onClose(CCObject* sender) override;
    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;

    void collectData();

    struct MergedKey {
        LevelKeys key;
        std::optional<int> downGroup;
        std::optional<int> upGroup;
    };

    std::vector<MergedKey> m_mergedKeys;
    std::vector<SimpleKeyAction> m_variables;
    std::vector<ClickAction> m_buttons;
    std::vector<TouchAction> m_touches;
};
