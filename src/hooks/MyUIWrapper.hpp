#pragma once

#include <cocos2d.h>
#include "GJBaseGameLayer.hpp"
#include "../LogVar.hpp"

struct MyUIWrapper : public CCLayer {

    MyBaseLayer* m_gameLayer;
    EditorUI* editorUI = nullptr;

    struct TouchHasher {
        std::size_t operator()(CCTouch* p) const { return p->getID(); }
    };
    struct TouchEquality {
        bool operator()(CCTouch* lhs, CCTouch* rhs) const { return lhs->getID() == rhs->getID(); }
    };

    boost::unordered_flat_map<CCTouch*, boost::unordered_flat_set<ClickActionData*>, TouchHasher, TouchEquality> claimedTouches;
    boost::unordered_flat_set<CCTouch*, TouchHasher, TouchEquality> ignoredTouches;
    bool processingCameraMove = false;
    uintptr_t clickActionsDataAtInsert = 0;

    virtual void registerWithTouchDispatcher();

    static bool isPointInOBB(const CCPoint& point, const std::array<CCPoint, 4>& corners);

    bool isTouchInsideBlock(CCTouch* touch, CollisionBlock* block);

    void updateNonUILayerTouches(float dt);

    void touchMoved(CCTouch* touch);

    virtual bool ccTouchBegan(CCTouch* touch, CCEvent* event);

    virtual void ccTouchMoved(CCTouch* touch, CCEvent* event);

    virtual void ccTouchEnded(CCTouch* touch, CCEvent* event);

    virtual bool init();

    static MyUIWrapper* create(MyBaseLayer* gameLayer, EditorUI* editorUI);

    bool originalBegan(CCTouch* touch, CCEvent* event);
    void originalMoved(CCTouch* touch, CCEvent* event);
    void originalEnded(CCTouch* touch, CCEvent* event);
};
