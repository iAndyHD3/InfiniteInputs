
#include "Geode/loader/Event.hpp"
#include <Geode/Geode.hpp>

#if defined(GEODE_IS_WINDOWS)
#include "LevelKeys.hpp"
#include <Geode/modify/CCEGLView.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <enchantum/enchantum.hpp>


extern void CROSSPLATFORM_ON_KEY_CALLBACK(LevelKeys key, bool down);


using namespace geode::prelude;

//for the mod reviewer reading this that will say that this could be done only in UILayer hooks:
//Know what? You're actually right, UILayer::handleKeyPress works great
//The problem ofcourse is that I assumed custom keybinds was a good mod and wouldn't fuck things up there.
//It swallows jump touches for no reason AND it sends the wrong keys for everything else.
//So no, I won't be using UILayer hooks even tho I really wanted to, that said I hope custom keybinds gets deleted one day :)

$execute {
    
    new EventListener<EventFilter<KeyboardInputEvent>>(+[](KeyboardInputEvent* event){

        if(event->action == KeyboardInputEvent::Action::Repeat) return geode::ListenerResult::Propagate;        
        
        CROSSPLATFORM_ON_KEY_CALLBACK(CocosKeyCodeToLevelKey(event->key), event->action == KeyboardInputEvent::Action::Press);
        return ListenerResult::Propagate;
    });

    new geode::EventListener<geode::EventFilter<MouseInputEvent>>(+[](MouseInputEvent* event){
        
        CROSSPLATFORM_ON_KEY_CALLBACK(GeodeMouseToLevelKeys(event->button), event->action == MouseInputEvent::Action::Press);
        return ListenerResult::Propagate;
    });

}

#endif