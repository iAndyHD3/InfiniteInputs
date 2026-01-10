#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/robtop/keyboard_dispatcher/CCKeyboardDelegate.h"
#include "Geode/modify/Modify.hpp"
#include "Geode/utils/cocos.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <unordered_map>
#include <vector>
#include <string_view>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/UILayer.hpp>
#include <scn/scan.h>
#include <enchantum/enchantum.hpp>
#include <alphalaneous.alphas-ui-pack/include/touch/Touch.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCEGLView.hpp>


using namespace geode::prelude;

static std::vector<std::string_view> getAllTexts(PlayLayer* pl)
{
	std::vector<std::string_view> texts;

	for(const auto& obj : CCArrayExt<GameObject*>(pl->m_objects))
	{
		if(obj->m_objectID != 914) continue;
		texts.emplace_back(static_cast<TextGameObject*>(obj)->m_text);
	}
	return texts;
}

enum class LevelKeys {
    f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
    KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
    q,w,e,r,t,z,u,i,o,p,
    a,s,d,f,g,h,j,k,l,
    y,x,c,v,b,n,m,
    space,
    leftCtrl,
    leftShift,
    leftAlt,
};

std::string_view fixKeyName(std::string_view name) {
    if (name.starts_with("KEY_")) [[unlikely]] {
        return name.substr(4);
    }
    return name;     
}

struct LevelKeyBinding
{
    enumKeyCodes key;
    int groupId;
};


enumKeyCodes LevelKeyToCocosKeyCode(LevelKeys key) {
    switch (key) {
        case LevelKeys::f1: return KEY_F1;
        case LevelKeys::f2: return KEY_F2;
        case LevelKeys::f3: return KEY_F3;
        case LevelKeys::f4: return KEY_F4;
        case LevelKeys::f5: return KEY_F5;
        case LevelKeys::f6: return KEY_F6;
        case LevelKeys::f7: return KEY_F7;
        case LevelKeys::f8: return KEY_F8;
        case LevelKeys::f9: return KEY_F9;
        case LevelKeys::f10: return KEY_F10;
        case LevelKeys::f11: return KEY_F11;
        case LevelKeys::f12: return KEY_F12;
        case LevelKeys::KEY_1: return KEY_One;
        case LevelKeys::KEY_2: return KEY_Two;
        case LevelKeys::KEY_3: return KEY_Three;
        case LevelKeys::KEY_4: return KEY_Four;
        case LevelKeys::KEY_5: return KEY_Five;
        case LevelKeys::KEY_6: return KEY_Six;
        case LevelKeys::KEY_7: return KEY_Seven;
        case LevelKeys::KEY_8: return KEY_Eight;
        case LevelKeys::KEY_9: return KEY_Nine;
        case LevelKeys::KEY_0: return KEY_Zero;
        case LevelKeys::q: return KEY_Q;
        case LevelKeys::w: return KEY_W;
        case LevelKeys::e: return KEY_E;
        case LevelKeys::r: return KEY_R;
        case LevelKeys::t: return KEY_T;
        case LevelKeys::z: return KEY_Z;
        case LevelKeys::u: return KEY_U;
        case LevelKeys::i: return KEY_I;
        case LevelKeys::o: return KEY_O;
        case LevelKeys::p: return KEY_P;
        case LevelKeys::a: return KEY_A;
        case LevelKeys::s: return KEY_S;
        case LevelKeys::d: return KEY_D;
        case LevelKeys::f: return KEY_F;
        case LevelKeys::g: return KEY_G;
        case LevelKeys::h: return KEY_H;
        case LevelKeys::j: return KEY_J;
        case LevelKeys::k: return KEY_K;
        case LevelKeys::l: return KEY_L;
        case LevelKeys::y: return KEY_Y;
        case LevelKeys::x: return KEY_X;
        case LevelKeys::c: return KEY_C;
        case LevelKeys::v: return KEY_V;
        case LevelKeys::b: return KEY_B;
        case LevelKeys::n: return KEY_N;
        case LevelKeys::m: return KEY_M;
        case LevelKeys::space: return KEY_Space;
        case LevelKeys::leftCtrl: return KEY_LeftControl;
        case LevelKeys::leftShift: return KEY_LeftShift;
        case LevelKeys::leftAlt: return KEY_LeftMenu;
        default: return KEY_Unknown;
    }
}

using groupId = int;


class MyUI;

struct MyClickDelegate : public CCNode, alpha::dispatcher::TouchDelegate {
    bool clickBegan(alpha::dispatcher::TouchEvent *touch) override;
};

struct MyKeyboardDelegate : public CCNode, CCKeyboardDelegate {
    virtual void keyDown(enumKeyCodes key) override {
        log::info("{}", enchantum::to_string(key));
    }

};


class $modify(MyUI, UILayer)
{
    static void onModify(auto& self) {
        if(!self.setHookPriority("UILayer::handleKeypress", INT_MIN)) {
            log::error("failed to set priority");
        }
    }


    struct Fields {
        std::unordered_map<enumKeyCodes, groupId> upKeyMap;
        std::unordered_map<enumKeyCodes, groupId> downKeyMap;
        MyClickDelegate* touchDelegate = nullptr;
        auto& getKeysMap(bool down) {
            return down ? downKeyMap : upKeyMap;
        }
        std::optional<groupId> getGroupId(enumKeyCodes k, bool down) {
            auto map = getKeysMap(down);
            auto it = map.find(k);
            if(it == map.end()) return std::nullopt;
            return it->second;
        }
        bool controlPressed = false;
        bool altPressed = false;
        bool shiftPressed = false;
        ~Fields() {
            if(touchDelegate) {
                CCTouchDispatcher::get()->removeDelegate(touchDelegate);
            }
        }
    };

    $override
    bool init(GJBaseGameLayer* layer) {
        if (!UILayer::init(layer)) return false;

        scheduleOnce(schedule_selector(MyUI::setupKeybinds), 0);
        return true;
    }

    void handleClick(alpha::dispatcher::TouchEvent *touch) {
        log::info("{}", enchantum::to_string(touch->getButton()));
    }


    void modifierKeysUpdate(float) {
        auto kb = CCDirector::get()->getKeyboardDispatcher();
        auto fields = m_fields.self();
        bool control = kb->getControlKeyPressed();
        if(control != fields->controlPressed) {
            handleKeypress(enumKeyCodes::KEY_LeftControl, control);
            fields->controlPressed = control;
        }
        bool shift = kb->getShiftKeyPressed();
        if(shift != fields->shiftPressed) {
            fields->shiftPressed = shift;
            handleKeypress(enumKeyCodes::KEY_LeftShift, shift);
        }
        bool alt = kb->getAltKeyPressed();
        if(alt != fields->altPressed) {
            fields->altPressed = alt;
            handleKeypress(enumKeyCodes::KEY_LeftMenu, alt);
        }
    }





    void addKeyBind(LevelKeys key, bool down, int groupId) {
        m_fields->getKeysMap(down).emplace(LevelKeyToCocosKeyCode(key), groupId);
    }

    
    void setupKeybinds(float) {
        
        schedule(schedule_selector(MyUI::modifierKeysUpdate), 0);
        m_fields->touchDelegate = new MyClickDelegate;

        CCTouchDispatcher::get()->addTargetedDelegate(m_fields->touchDelegate, -128, true);

        addChild(m_fields->touchDelegate);

        auto texts = getAllTexts(PlayLayer::get());
        for(const auto& t : texts) {
            log::debug("parsing {}", t);
            auto result = scn::scan<std::string, int, int>(t, "@{} {} = {}");
            if(!result) {
                if(t.front() == '@') {
                    log::error("'{}' Could not parse label with format @{{}} {{}} = {{}}, mind the spaces", t);
                }
                continue;
            }
            auto& [levelKeyName, keyDown, levelGroupId] = result->values();
            for(const auto& [keyVal, originalKeyName] : enchantum::entries_generator<LevelKeys>) {
                auto keyName = fixKeyName(originalKeyName);
                //log::debug("{} {}", keyName, levelKeyName);
                if(keyName == levelKeyName) {
                    addKeyBind(keyVal, keyDown == 1, levelGroupId);
                    log::debug("Added key {}: {} with group ID: {}", keyDown ? "down" : "up", enchantum::to_string(keyVal), levelGroupId);
                }
            }
        }

        CCDirector::get()->getKeyboardDispatcher();

        geode::log::info("Added {} down keys", m_fields->downKeyMap.size());
        geode::log::info("Added {} up keys", m_fields->upKeyMap.size());
    }


    $override
    void handleKeypress(enumKeyCodes key, bool down) {
        log::debug("Handle key press");
        auto fields = m_fields.self();
        auto groupopt = fields->getGroupId(key, down);
        if(groupopt) {
        log::info("KEY: {}, {}, GROUP: {}", (int)key, down ? "down" : "up", *groupopt);
            PlayLayer::get()->spawnGroup(*groupopt, false, 0, {}, 0, 0);
        }
        UILayer::handleKeypress(key, down);
    }
};


bool MyClickDelegate::clickBegan(alpha::dispatcher::TouchEvent *touch) {
    reinterpret_cast<MyUI*>(PlayLayer::get()->m_uiLayer)->handleClick(touch);
    return true;
}



