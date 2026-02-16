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

    static std::optional<ClickAction> parse(std::string_view text);
    std::string getLabel();
};

struct SimpleKeyAction {
    LevelKeys key;
    int group;

    static std::optional<SimpleKeyAction> parse(std::string_view text);
    std::string getLabel();
};

using II_ObjectAction = std::variant<KeyAction, SimpleKeyAction, ClickAction>;

std::optional<II_ObjectAction> parseObjectString(std::string_view t);


bool isOldFormatString(std::string_view t);
