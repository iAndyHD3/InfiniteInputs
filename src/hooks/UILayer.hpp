#pragma once

#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/modify/UILayer.hpp>

using namespace geode::prelude;

class $modify(MyUILayer, UILayer) {

    struct Fields {
        bool allow = false;
    };

    bool init(GJBaseGameLayer* layer) {
        if (!UILayer::init(layer)) return false;
        return true;
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) {
        if(auto& allow = m_fields->allow; allow) {
            if(!m_gameLayer->m_isEditor) allow = false;
            return UILayer::ccTouchBegan(touch, event);
        }
        return false;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent* event) {
        if(auto& allow = m_fields->allow; allow) {
            if(!m_gameLayer->m_isEditor) allow = false;
            UILayer::ccTouchMoved(touch, event);
        }
    }

    void ccTouchEnded(CCTouch* touch, CCEvent* event) {
        if(auto& allow = m_fields->allow; allow) {
            if(!m_gameLayer->m_isEditor) allow = false;
            UILayer::ccTouchEnded(touch, event);
        }
    }
};