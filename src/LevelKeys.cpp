#include "LevelKeys.hpp"
#include <enchantum/enchantum.hpp>
#include "Geode/cocos/robtop/keyboard_dispatcher/CCKeyboardDelegate.h"



std::string_view fixKeyName(std::string_view name) {
    if (name.starts_with("KEY_")) [[unlikely]] {
        return name.substr(4);
    }
    return name;
}

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

LevelKeys keyLevelIdentifierToValue(std::string_view levelkeyname) {
    for (const auto& [keyVal, originalKeyName] : enchantum::entries_generator<LevelKeys>) {
        auto keyName = fixKeyName(originalKeyName);
        if (keyName == levelkeyname) {
            return keyVal;
        }
    }
    return LevelKeys::unknown;
}

LevelKeys CocosKeyCodeToLevelKey(enumKeyCodes code) {
    switch (code) {
        case KEY_F1: return LevelKeys::f1;
        case KEY_F2: return LevelKeys::f2;
        case KEY_F3: return LevelKeys::f3;
        case KEY_F4: return LevelKeys::f4;
        case KEY_F5: return LevelKeys::f5;
        case KEY_F6: return LevelKeys::f6;
        case KEY_F7: return LevelKeys::f7;
        case KEY_F8: return LevelKeys::f8;
        case KEY_F9: return LevelKeys::f9;
        case KEY_F10: return LevelKeys::f10;
        case KEY_F11: return LevelKeys::f11;
        case KEY_F12: return LevelKeys::f12;
        case KEY_One: return LevelKeys::KEY_1;
        case KEY_Two: return LevelKeys::KEY_2;
        case KEY_Three: return LevelKeys::KEY_3;
        case KEY_Four: return LevelKeys::KEY_4;
        case KEY_Five: return LevelKeys::KEY_5;
        case KEY_Six: return LevelKeys::KEY_6;
        case KEY_Seven: return LevelKeys::KEY_7;
        case KEY_Eight: return LevelKeys::KEY_8;
        case KEY_Nine: return LevelKeys::KEY_9;
        case KEY_Zero: return LevelKeys::KEY_0;
        case KEY_Q: return LevelKeys::q;
        case KEY_W: return LevelKeys::w;
        case KEY_E: return LevelKeys::e;
        case KEY_R: return LevelKeys::r;
        case KEY_T: return LevelKeys::t;
        case KEY_Z: return LevelKeys::z;
        case KEY_U: return LevelKeys::u;
        case KEY_I: return LevelKeys::i;
        case KEY_O: return LevelKeys::o;
        case KEY_P: return LevelKeys::p;
        case KEY_A: return LevelKeys::a;
        case KEY_S: return LevelKeys::s;
        case KEY_D: return LevelKeys::d;
        case KEY_F: return LevelKeys::f;
        case KEY_G: return LevelKeys::g;
        case KEY_H: return LevelKeys::h;
        case KEY_J: return LevelKeys::j;
        case KEY_K: return LevelKeys::k;
        case KEY_L: return LevelKeys::l;
        case KEY_Y: return LevelKeys::y;
        case KEY_X: return LevelKeys::x;
        case KEY_C: return LevelKeys::c;
        case KEY_V: return LevelKeys::v;
        case KEY_B: return LevelKeys::b;
        case KEY_N: return LevelKeys::n;
        case KEY_M: return LevelKeys::m;
        case KEY_Enter: return LevelKeys::enter;

        case KEY_Space: return LevelKeys::space;
        case KEY_LeftControl: return LevelKeys::leftCtrl;
        case KEY_LeftShift: return LevelKeys::leftShift;
        case KEY_LeftMenu: return LevelKeys::leftAlt;
        default: return LevelKeys::unknown;
    }
}

LevelKeys GeodeMouseToLevelKeys(geode::MouseInputData::Button button) {
    switch (button) {
        case geode::MouseInputData::Button::Left: return LevelKeys::leftMouse;
        case geode::MouseInputData::Button::Right: return LevelKeys::rightMouse;
        case geode::MouseInputData::Button::Middle: return LevelKeys::middleMouse;
        case geode::MouseInputData::Button::Button4: return LevelKeys::mouse3;
        case geode::MouseInputData::Button::Button5: return LevelKeys::mouse4; break;
    }
}
