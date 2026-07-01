#pragma once
#include <Geode/binding/TextGameObject.hpp>
#include <Geode/ui/Popup.hpp>
#include <vector>
#include <string>
#include "TextParsing.hpp"

class GlobalOverviewPopup : public geode::Popup {
public:
    static GlobalOverviewPopup* create();

protected:
    bool init();

    void parseAllActions();
    void rebuildUI();
    void onPrev(CCObject*);
    void onNext(CCObject*);
    void onView(CCObject* sender);

    struct ActionEntry {
        TextGameObject* m_textObject;
        std::string tabName;
        std::vector<std::string> propertyNames;
        std::vector<std::string> values;
        std::vector<float> columnXPositions;
    };

    std::vector<ActionEntry> m_entries;
    int m_currentPage = 0;
    int const m_perPage = 12;

    CCNode* m_contentArea = nullptr;
    CCMenuItemSpriteExtra* m_prevBtn = nullptr;
    CCMenuItemSpriteExtra* m_nextBtn = nullptr;
    CCLabelBMFont* m_emptyLabel = nullptr;
};
