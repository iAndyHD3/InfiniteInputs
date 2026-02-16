#include <Geode/binding/GJOptionsLayer.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/GJOptionsLayer.hpp>
#include "Geode/utils/cocos.hpp"

using namespace geode::prelude;

class $modify(GJOptionsLayer) {


    struct Fields {
        bool inEditorSettings = false;
        ~Fields() {
            Mod::get()->setSavedValue("inEditorSettings", inEditorSettings);
            
        }
    };
    
    void addGVToggle(char const* title, char const* variable, char const* description) {
        GJOptionsLayer::addGVToggle(title, variable, description);
        if (std::string_view{"Static Trace Arrows"} == title) {
            GJOptionsLayer::addGVToggle("hello world", "1979368571", nullptr);
            m_fields->inEditorSettings = true;

            

        }
    }

    

};