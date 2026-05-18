#if defined(GEODE_IS_WINDOWS)

#include <Geode/modify/CCEGLView.hpp>
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

#elif defined(GEODE_IS_MACOS)

#include <Geode/modify/PlatformToolbox.hpp>
#include "GJBaseGameLayer.hpp"

class $modify(PlatformToolbox)
{
    void toggleLockCursor(bool isLocked) {
        //this is way easier and simpler than manually toggling it everywhere.
        //its actually called before playlayer is initialized, so at the start it gets called manually in gjbgl
        auto mylayer = reinterpret_cast<MyBaseLayer*>(PlayLayer::get());
        if(mylayer && mylayer->m_fields->hasAnyMouseKeyActive()) {
            PlatformToolbox::toggleLockCursor(false);
            return;
        }
        PlatformToolbox::toggleLockCursor(isLocked);
    }
};

#endif