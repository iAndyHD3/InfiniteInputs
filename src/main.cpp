#include "hooks/GJBaseGameLayer.hpp"

using namespace geode::prelude;

//called from windows.cpp or macos.mm (mac version is NOT currently being released, can be built locally)
void CROSSPLATFORM_ON_KEY_CALLBACK(LevelKeys key, bool down)
{
    if(auto layer = reinterpret_cast<MyBaseLayer*>(GJBaseGameLayer::get()); layer && layer->isActive()) {
        layer->nh_handleKeypress(key, down);
    } else {
        log::info("unhandled press");
    }
}