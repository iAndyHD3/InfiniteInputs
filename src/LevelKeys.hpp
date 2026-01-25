#pragma once
#include <cocos2d.h>
//#include <alphalaneous.alphas-ui-pack/include/touch/Touch.hpp>
using namespace cocos2d;

enum class LevelKeys {
    editorTab = -3,
    empty = -2,
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

    wheelUp,
    wheelDown,

    cursor,
    modLoaded
};

struct LevelKeyBinding
{
    enumKeyCodes key;
    int groupId;
};

// Function declarations
LevelKeys CocosKeyCodeToLevelKey(enumKeyCodes code);
LevelKeys glfwKeyToLevelKey(int glfwKey);
std::string_view fixKeyName(std::string_view name);
enumKeyCodes LevelKeyToCocosKeyCode(LevelKeys key);
//LevelKeys AlphaMouseButtonToLevelKeys(alpha::dispatcher::MouseButton button);
LevelKeys keyLevelIdentifierToValue(std::string_view levelkeyname);

