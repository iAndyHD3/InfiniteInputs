#pragma once
#include <optional>
#include <string_view>
#include "LevelKeys.hpp"


struct KeyAction {
    LevelKeys key;
    bool keyDown;
    int group;

    static std::optional<KeyAction> parse(std::string_view text);
    std::string getLabel();
};

struct ClickAction {
    int collisionBlockId;
    int groupIdCursorEnter;
    int groupIdCursorExit;
    int groupIdCursorDown;
    int groupIdCursorUp;
    bool stealTouches;
    bool allowStealFrom;
    bool ignoreInput;

    static std::optional<ClickAction> parse(std::string_view text);
    std::string getLabel();
};

struct SimpleKeyAction {
    LevelKeys key;
    int group;

    static std::optional<SimpleKeyAction> parse(std::string_view text);
    static bool isSimpleKey(LevelKeys);
    static bool isItemIdKey(LevelKeys);
    std::string getLabel();
};

struct TouchAction {
    int touch_id;
    int groupIdLockObjectsToTouch;
    int groupIdTouchDown;
    int groupIdTouchUp;
    int itemId_x;
    int itemId_y;
    int itemId_deltaX;
    int itemId_deltaY;

    static std::optional<TouchAction> parse(std::string_view text);
    std::string getLabel();
};


using II_ObjectAction = std::variant<KeyAction, SimpleKeyAction, ClickAction, TouchAction>;

std::optional<II_ObjectAction> parseObjectString(std::string_view t);


bool isOldFormatString(std::string_view t);
