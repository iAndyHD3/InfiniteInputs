
#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/robtop/glfw/glfw3.h"
#include "Geode/modify/Modify.hpp"

#include <Geode/binding/LevelEditorLayer.hpp>
#include <alphalaneous.alphas-ui-pack/include/touch/Touch.hpp>
#include "Geode/utils/cocos.hpp"
#include "alphalaneous.alphas-ui-pack/include/touch/TouchDispatcher.hpp"

#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/CCEGLView.hpp>

#include <scn/scan.h>
#include <enchantum/enchantum.hpp>

#include <unordered_map>
#include <vector>
#include <string_view>

using namespace geode::prelude;



enum class LevelKeys {
    unknown = -1,
    f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
    KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
    q,w,e,r,t,z,u,i,o,p,
    a,s,d,f,g,h,j,k,l,
    y,x,c,v,b,n,m,
    enter,
    space,
    leftCtrl,
    leftShift,
    leftAlt,

    leftMouse,
    rightMouse,
    middleMouse,
    mouse3,
    mouse4,

    cursor
};


LevelKeys glfwKeyToLevelKey(int glfwKey) {
    switch (glfwKey) {
        // Function keys
        case GLFW_KEY_F1:  return LevelKeys::f1;
        case GLFW_KEY_F2:  return LevelKeys::f2;
        case GLFW_KEY_F3:  return LevelKeys::f3;
        case GLFW_KEY_F4:  return LevelKeys::f4;
        case GLFW_KEY_F5:  return LevelKeys::f5;
        case GLFW_KEY_F6:  return LevelKeys::f6;
        case GLFW_KEY_F7:  return LevelKeys::f7;
        case GLFW_KEY_F8:  return LevelKeys::f8;
        case GLFW_KEY_F9:  return LevelKeys::f9;
        case GLFW_KEY_F10: return LevelKeys::f10;
        case GLFW_KEY_F11: return LevelKeys::f11;
        case GLFW_KEY_F12: return LevelKeys::f12;

        // Number keys
        case GLFW_KEY_1: return LevelKeys::KEY_1;
        case GLFW_KEY_2: return LevelKeys::KEY_2;
        case GLFW_KEY_3: return LevelKeys::KEY_3;
        case GLFW_KEY_4: return LevelKeys::KEY_4;
        case GLFW_KEY_5: return LevelKeys::KEY_5;
        case GLFW_KEY_6: return LevelKeys::KEY_6;
        case GLFW_KEY_7: return LevelKeys::KEY_7;
        case GLFW_KEY_8: return LevelKeys::KEY_8;
        case GLFW_KEY_9: return LevelKeys::KEY_9;
        case GLFW_KEY_0: return LevelKeys::KEY_0;

        // Letter keys
        case GLFW_KEY_Q: return LevelKeys::q;
        case GLFW_KEY_W: return LevelKeys::w;
        case GLFW_KEY_E: return LevelKeys::e;
        case GLFW_KEY_R: return LevelKeys::r;
        case GLFW_KEY_T: return LevelKeys::t;
        case GLFW_KEY_Z: return LevelKeys::z;
        case GLFW_KEY_U: return LevelKeys::u;
        case GLFW_KEY_I: return LevelKeys::i;
        case GLFW_KEY_O: return LevelKeys::o;
        case GLFW_KEY_P: return LevelKeys::p;

        case GLFW_KEY_A: return LevelKeys::a;
        case GLFW_KEY_S: return LevelKeys::s;
        case GLFW_KEY_D: return LevelKeys::d;
        case GLFW_KEY_F: return LevelKeys::f;
        case GLFW_KEY_G: return LevelKeys::g;
        case GLFW_KEY_H: return LevelKeys::h;
        case GLFW_KEY_J: return LevelKeys::j;
        case GLFW_KEY_K: return LevelKeys::k;
        case GLFW_KEY_L: return LevelKeys::l;

        case GLFW_KEY_Y: return LevelKeys::y;
        case GLFW_KEY_X: return LevelKeys::x;
        case GLFW_KEY_C: return LevelKeys::c;
        case GLFW_KEY_V: return LevelKeys::v;
        case GLFW_KEY_B: return LevelKeys::b;
        case GLFW_KEY_N: return LevelKeys::n;
        case GLFW_KEY_M: return LevelKeys::m;

        // Special keys
        case GLFW_KEY_ENTER:        return LevelKeys::enter;
        case GLFW_KEY_SPACE:        return LevelKeys::space;
        case GLFW_KEY_LEFT_CONTROL: return LevelKeys::leftCtrl;
        case GLFW_KEY_LEFT_SHIFT:   return LevelKeys::leftShift;
        case GLFW_KEY_LEFT_ALT:     return LevelKeys::leftAlt;

        default: return LevelKeys::unknown;
    }
}

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

LevelKeys AlphaMouseButtonToLevelKeys(alpha::dispatcher::MouseButton button) {
    switch(button) {

    case alpha::dispatcher::MouseButton::LEFT: return LevelKeys::leftMouse;
    case alpha::dispatcher::MouseButton::MIDDLE: return LevelKeys::middleMouse;
    case alpha::dispatcher::MouseButton::RIGHT: return LevelKeys::rightMouse;
    case alpha::dispatcher::MouseButton::BUTTON3: return LevelKeys::mouse3;
    case alpha::dispatcher::MouseButton::BUTTON4: return LevelKeys::mouse4;
    case alpha::dispatcher::MouseButton::HOVER: return LevelKeys::unknown;
    default: return LevelKeys::unknown;
    }
}


static gd::vector<short> getGroupIDs(GameObject* obj) {
    gd::vector<short> res;

    if (obj->m_groups && obj->m_groups->at(0))
        for (auto i = 0; i < obj->m_groupCount; i++)
            res.push_back(obj->m_groups->at(i));
    return res;
}
static bool hasGroup(GameObject* obj, int group)
{
    for (const auto& g : getGroupIDs(obj))
    {
        if (g == group)
            return true;
    }
    return false;
}



using groupId = int;

struct MyClickDelegate : public CCNode, alpha::dispatcher::TouchDelegate {
    bool clickBegan(alpha::dispatcher::TouchEvent* touch) override;
	void clickEnded(alpha::dispatcher::TouchEvent* touch) override;
};

class $modify(MyBaseLayer, GJBaseGameLayer)
{
    struct Fields {
        std::unordered_map<LevelKeys, groupId> upKeyMap;
        std::unordered_map<LevelKeys, groupId> downKeyMap;
        MyClickDelegate* touchDelegate = nullptr;
        bool controlPressed = false;
        bool altPressed = false;
        bool shiftPressed = false;
        bool scheduledModifiedKeys = false;
        bool active = false;
        GJBaseGameLayer* layer = nullptr;

        //needs to be ccarray because moveObjects takes CCArray* and converting from vector to ccarray would be stupid
        CCArrayExt<GameObject*> cursorFollowObjects;

        int cursorFollowGroupId = -1;

        
        void clear() {
            upKeyMap.clear();
            downKeyMap.clear();
            if(touchDelegate) {
                CCTouchDispatcher::get()->removeDelegate(touchDelegate);
                touchDelegate = nullptr;
            }
            controlPressed = false;
            altPressed = false;
            shiftPressed = false;
            scheduledModifiedKeys = false;
            active = false;
            layer = nullptr;
            //no clear() in Ext
            cursorFollowObjects.inner()->removeAllObjects();
            cursorFollowGroupId = -1;
        }


        auto& getKeysMap(bool down) {
            return down ? downKeyMap : upKeyMap;
        }


        void addKeyBind(LevelKeys key, bool down, int groupId) {
            getKeysMap(down).emplace(key, groupId);
        }


        std::optional<groupId> getGroupId(LevelKeys k, bool down) {
            if(k == LevelKeys::unknown) return std::nullopt;

            auto map = getKeysMap(down);
            auto it = map.find(k);
            if(it == map.end()) return std::nullopt;

            return it->second;
        }

        void spawnGroupIfDefined(LevelKeys k, bool down) {
            if(auto group = getGroupId(k, down)) {
                log::info("KEY: {}, {}, GROUP: {}", enchantum::to_string(k), down ? "down" : "up", *group);
                layer->spawnGroup(*group, false, 0, {}, 0, 0);
            }
        }

        ~Fields() {
            if(touchDelegate) {
                CCTouchDispatcher::get()->removeDelegate(touchDelegate);
            }
        }
    };

    $override
    bool init() {
        if (!GJBaseGameLayer::init()) return false;

        scheduleOnce(schedule_selector(MyBaseLayer::delayedInit), 0);
        return true;
    }

    void delayedInit(float)
    {
        if(m_isEditor) {
            schedule(schedule_selector(MyBaseLayer::editorActiveHandlerLoop), 0);
            return;
        }

        scheduleOnce(schedule_selector(MyBaseLayer::setupKeybinds_step0), 0);

        //start directly in playlayer
    }


    static std::vector<std::string_view> getAllTextsFromLabels(GJBaseGameLayer* pl) {
	    std::vector<std::string_view> texts;

	    for(const auto& obj : CCArrayExt<GameObject*>(pl->m_objects)) {
	    	if(obj->m_objectID != 914) continue;
	    	texts.emplace_back(static_cast<TextGameObject*>(obj)->m_text);
	    }
	    return texts;
    }

    void editorActiveHandlerLoop(float) {
        auto fields = m_fields.self();
        auto editor = reinterpret_cast<LevelEditorLayer*>(this);
        bool inPlaytest = m_playbackMode == PlaybackMode::Playing;

        if(!fields->active && inPlaytest) {
            fields->active = true;
            setupKeybinds_step0(0);
        }
        else if(fields->active && !inPlaytest) {
            fields->active = false;
            fields->clear();
            unschedule(schedule_selector(MyBaseLayer::updateLoop));
        }
    }

    void handleClick(alpha::dispatcher::TouchEvent* touch, bool down) {
        LevelKeys btn = AlphaMouseButtonToLevelKeys(touch->getButton());
        log::info("{} {}", enchantum::to_string(touch->getButton()), enchantum::to_string(btn));
        nh_handleKeypress(btn, down);
    }


    void updateLoop(float) {
        auto kb = CCDirector::get()->getKeyboardDispatcher();
        auto fields = m_fields.self();
        bool control = kb->getControlKeyPressed();
        if(control != fields->controlPressed) {
            fields->controlPressed = control;
            nh_handleKeypress(LevelKeys::leftCtrl, control);
        }
        bool shift = kb->getShiftKeyPressed();
        if(shift != fields->shiftPressed) {
            fields->shiftPressed = shift;
            nh_handleKeypress(LevelKeys::leftShift, shift);
        }
        bool alt = kb->getAltKeyPressed();
        if(alt != fields->altPressed) {
            fields->altPressed = alt;
            nh_handleKeypress(LevelKeys::leftAlt, alt);
        }


    for(const auto& o : fields->cursorFollowObjects) {
        auto mousepos = getMousePos();
        // Convert mouse to the same coordinate space where the object's position is defined
        auto targetPos = o->getParent() ? o->getParent()->convertToNodeSpace(mousepos) : mousepos;
        auto currentPos = o->getPosition();
        auto delta = targetPos - currentPos;
        moveObject(o, delta.x, delta.y, false);
    }

    }

    void setupText(std::string_view t)
    {
        log::debug("parsing {}", t);
        auto result = scn::scan<std::string, int, int>(t, "@{} {} = {}");
        if(!result) {
            if(t.front() == '@') {
                log::error("'{}' Could not parse label with format @{{}} {{}} = {{}}, mind the spaces", t);
            }
            return;
        }
        auto& [levelKeyName, keyDown, levelGroupId] = result->values();
        for(const auto& [keyVal, originalKeyName] : enchantum::entries_generator<LevelKeys>) {
            auto keyName = fixKeyName(originalKeyName);
            //log::debug("{} {}", keyName, levelKeyName);
            if(keyName == levelKeyName) {
                if(keyVal == LevelKeys::cursor) {
                    setupCursorGroup(levelGroupId);
                    continue;
                }
                m_fields->addKeyBind(keyVal, keyDown == 1, levelGroupId);
                log::debug("Added key {}: {} with group ID: {}", keyDown ? "down" : "up", enchantum::to_string(keyVal), levelGroupId);
            }
        }
    }

    void setupCursorGroup(int cursorGroupId) {
        log::info("SETTING UP CURSOR GROUP {}", cursorGroupId);
        m_fields->cursorFollowGroupId = cursorGroupId;
        m_fields->cursorFollowObjects.inner()->removeAllObjects();
        for(const auto& o : CCArrayExt<GameObject*>(m_objects)) {
            if(hasGroup(o, cursorGroupId)) {
                m_fields->cursorFollowObjects.push_back(o);
            }
        }
    }

    //true if correctly registered atleast one keybind
    bool setupTextLabelKeys_step1() {
        auto fields = m_fields.self();
        auto texts = getAllTextsFromLabels(this);
        std::ranges::for_each(texts, [this](auto& t){setupText(t); });

        log::info("Added {} down keys", fields->downKeyMap.size());
        log::info("Added {} up keys", fields->upKeyMap.size());
        return !fields->downKeyMap.empty() || !fields->upKeyMap.empty(); 
    }

    bool isActive() {
        return m_fields->active;
    }

    void setupKeybinds_step0(float) {
        if(!setupTextLabelKeys_step1()) {
            log::error("not parsed any labels");
            return;
        }
        auto fields = m_fields.self();
        fields->layer = this;

        if(!fields->touchDelegate) {
            fields->touchDelegate = new MyClickDelegate;
            fields->touchDelegate->setID("iandyhd.keyboardsupport/touch-delegate");
            fields->touchDelegate->setContentSize(this->getContentSize());
            addChild(fields->touchDelegate, INT_MAX);
            CCTouchDispatcher::get()->addTargetedDelegate(fields->touchDelegate, INT_MIN, false);
        }

        if(!fields->scheduledModifiedKeys) {
            schedule(schedule_selector(MyBaseLayer::updateLoop), 0);
            fields->scheduledModifiedKeys = true;
        }

        fields->active = true;
    }


    void nh_handleKeypress(LevelKeys key, bool down) {
        log::debug("Handle key press");
        m_fields->spawnGroupIfDefined(key, down);
    }
};


//for the mod reviewer reading this that will say that this could be done only in UILayer hooks:
//Know what? You're actually right, UILayer::handleKeyPress works great
//The problem ofcourse is that I assumed custom keybinds was a good mod and wouldn't fuck things up there.
//It swallows jump touches for no reason AND it sends the wrong keys for everything else.
//So no, I won't be using UILayer hooks even tho I really wanted to, that said I hope custom keybinds gets deleted one day :)

class $modify(cocos2d::CCEGLView)
{
    //action == 0, up
    //action == 1, down
    //action == 2, hold
    void onGLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
        CCEGLView::onGLFWKeyCallback(window, key, scancode, action, mods);
        log::debug("key {}, scancode {}, action {}, mods {}", key, scancode, action, mods);
        if(action >= 2) return;

        if(auto layer = reinterpret_cast<MyBaseLayer*>(GJBaseGameLayer::get()); layer && layer->isActive()) {
            layer->nh_handleKeypress(glfwKeyToLevelKey(key), action == 1);
        }
	}
};



bool MyClickDelegate::clickBegan(alpha::dispatcher::TouchEvent *touch) {
    log::debug("click began {}", enchantum::to_string(touch->getButton()));
    reinterpret_cast<MyBaseLayer*>(GJBaseGameLayer::get())->handleClick(touch, true);
    return true;
}

void MyClickDelegate::clickEnded(alpha::dispatcher::TouchEvent* touch) {
    log::debug("click ended {}", enchantum::to_string(touch->getButton()));
    reinterpret_cast<MyBaseLayer*>(GJBaseGameLayer::get())->handleClick(touch, false);
}