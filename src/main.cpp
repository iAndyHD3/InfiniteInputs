#include <Geode/utils/Keyboard.hpp>
#include <enchantum/enchantum.hpp>
#include "Geode/cocos/actions/CCActionInterval.h"
#include "Geode/loader/Event.hpp"
#include "LevelKeys.hpp"
#include "hooks/GJBaseGameLayer.hpp"


void onKeyInput(LevelKeys key, bool down) {
    if (auto layer = reinterpret_cast<MyBaseLayer*>(GJBaseGameLayer::get()); layer && layer->isModActive()) {
        layer->nh_handleKeypress(key, down);
    }
}

void onScrollInput(float x, float y) {
    if (auto layer = reinterpret_cast<MyBaseLayer*>(GJBaseGameLayer::get()); layer && layer->isModActive()) {
        layer->handleScroll(x, y);
    }
}

$execute {

    KeyboardInputEvent()
            .listen(+[](const geode::KeyboardInputData& event) {
                if (event.action == KeyboardInputData::Action::Repeat)
                    return geode::ListenerResult::Propagate;
                onKeyInput(CocosKeyCodeToLevelKey(event.key), event.action == KeyboardInputData::Action::Press);
                return ListenerResult::Propagate;
            })
            .leak();

    MouseInputEvent()
            .listen(+[](const geode::MouseInputData& event) {
                onKeyInput(GeodeMouseToLevelKeys(event.button), event.action == MouseInputData::Action::Press);
                return ListenerResult::Propagate;
            })
            .leak();

    ScrollWheelEvent()
            .listen(+[](double x, double y) {
                onScrollInput(x, y);
                return ListenerResult::Propagate;
            })
            .leak();
};
