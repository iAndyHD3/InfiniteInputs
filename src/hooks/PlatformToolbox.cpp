#include <Geode/modify/CCEGLView.hpp>
#include "BetterGeodeLogs.hpp"
#include "GJBaseGameLayer.hpp"
#include "Geode/cocos/platform/win32/CCEGLView.h"

using namespace geode::prelude;

class $modify(CCEGLView)
{
    void toggleLockCursor(bool isLocked) {
        //this is way easier and simpler than manually toggling it everywhere.
        //its actually called before playlayer is initialized, so at the start it gets called manually in gjbgl
        auto mylayer = reinterpret_cast<MyBaseLayer*>(PlayLayer::get());
        if(mylayer && mylayer->m_fields->hasAnyMouseKeyActive()) {
            CCEGLView::toggleLockCursor(false);
            return;
        }
        CCEGLView::toggleLockCursor(isLocked);
    }
};



