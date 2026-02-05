#pragma once
#include <optional>
#include <string_view>
#include "LevelKeys.hpp"


struct KeyAction {
    LevelKeys key;
    bool keyDown;
    int group;
};

struct ClickAction {
    int collisionBlockId;
    int groupIdCursorEnter;
    int groupIdCursorExit;
    int groupIdCursorDown;
    int groupIdCursorUp;
};

struct SimpleKeyAction {
    LevelKeys key;
    int group;
};

using II_ObjectAction = std::variant<KeyAction, SimpleKeyAction, ClickAction>;


std::string getLabelFromKeyAction(const KeyAction& t);
std::optional<KeyAction> getParsedKeyAction(std::string_view t);

std::string getLabelFromClickAction(const ClickAction& t);
std::optional<ClickAction> getClickActionFromLabel(std::string_view t);

std::string getLabelFromSimpleKeyAction(const SimpleKeyAction& t);
std::optional<SimpleKeyAction> getSimpleKeyActionFromLabel(std::string_view t);


std::optional<II_ObjectAction> parseObjectString(std::string_view t);


bool foundOldFormatString(std::string_view t);
