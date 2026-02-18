#include "TextParsing.hpp"
#include <Geode/modify/Modify.hpp>
#include <Geode/utils/casts.hpp>
#include <Geode/utils/string.hpp>
#include <cstdint>
#include <enchantum/enchantum.hpp>
#include <fmt/format.h>
#include <scn/scan.h>
#include "LevelKeys.hpp"
#include "InputTriggerPopup.hpp"

using namespace geode::prelude;


std::string KeyAction::getLabel() {
    return fmt::format(
            "inf_inp:1 {} {} {}", fixKeyName(enchantum::to_string(key)), static_cast<int>(keyDown),
            group);
}


std::optional<KeyAction> KeyAction::parse(std::string_view t) {

    if (auto result = scn::scan<std::string, int, int>(t, "inf_inp:1 {} {} {}")) {
        auto& [key, keyDownParsed, groupParsed] = result->values();
        if (auto enumval = keyLevelIdentifierToValue(key); enumval != LevelKeys::unknown) {
            return KeyAction{enumval, keyDownParsed == 1, groupParsed};
        }
    }
    return std::nullopt;
}

bool isOldFormatString(std::string_view t) {
    return scn::scan<std::string, int>(t, "inf_inp:{} = {}").has_value() ||
           scn::scan<std::string, char, int>(t, "inf_inp:{} {} = {}").has_value();
}

std::string ClickAction::getLabel() {
    return fmt::format(
            "inf_inp:3 {} {} {} {} {} {} {}", collisionBlockId, groupIdCursorEnter, groupIdCursorExit,
            groupIdCursorDown, groupIdCursorUp, static_cast<uint8_t>(stealTouches), static_cast<uint8_t>(allowStealFrom));
}


std::optional<ClickAction> ClickAction::parse(std::string_view t) {
    if (auto result = scn::scan<int, int, int, int, int, int, int>(t, "inf_inp:3 {} {} {} {} {} {} {}")) {
        auto& [collisionBlockId, groupIdCursorEnter, groupIdCursorExit, groupIdCursorDown, groupIdCursorUp,
               stealTouches, allowStealFrom] = result->values();

        return ClickAction{collisionBlockId, groupIdCursorEnter, groupIdCursorExit, groupIdCursorDown, groupIdCursorUp, static_cast<bool>(stealTouches), static_cast<bool>(allowStealFrom)};
    }
    log::error("Could not parse click action: {}", t);
    return std::nullopt;
}

std::string SimpleKeyAction::getLabel() {
    return fmt::format("inf_inp:2 {} {}", fixKeyName(enchantum::to_string(key)), group);
}

bool SimpleKeyAction::isSimpleKey(LevelKeys e)
{
    switch(e) {
        default: return false;
        case LevelKeys::wheelUp:
        case LevelKeys::wheelDown:
        case LevelKeys::cursor:
        case LevelKeys::modLoaded:
            return true;
    }
}

std::optional<SimpleKeyAction> SimpleKeyAction::parse(std::string_view t) {
    if (auto result = scn::scan<std::string, int>(t, "inf_inp:2 {} {}")) {
        auto& [key, group] = result->values();
        LevelKeys parsedKey = keyLevelIdentifierToValue(key);
        if(!isSimpleKey(parsedKey)) return std::nullopt;
        return SimpleKeyAction{parsedKey, group};
    }
    return std::nullopt;
}

std::optional<II_ObjectAction> parseObjectString(std::string_view t) {
    if (auto result = KeyAction::parse(t)) {
        return result;
    }
    if (auto result = SimpleKeyAction::parse(t)) {
        return result;
    }
    if (auto result = ClickAction::parse(t)) {
        return result;
    }
    return std::nullopt;
}







/*
OLD FORMAT

    // std::string keyStr;
    // bool keyDown = false;
    // int group = 0;

    // if (auto result = scn::scan<std::string, char, int>(t, "inf_inp:{} {} = {}")) {
    //     auto [key, keyDownParsed, groupParsed] = result->values();
    //     keyStr = std::move(key);

    //     if(auto keyDownOpt = praseKeyUpDownSpecifier(keyDownParsed)) {
    //         keyDown = *keyDownOpt;
    //     } else {
    //         geode::log::error("Not accepted key down/up specifier: {}", keyDownParsed);
    //         return std::nullopt;
    //     }

    //     group = groupParsed;
    // }
    // else if (auto result = scn::scan<std::string, int>(t, "inf_inp:{} = {}")) {
    //     auto [key, groupParsed] = result->values();
    //     keyStr = std::move(key);
    //     group = groupParsed;
    // }
    // else {
    //     return std::nullopt;
    // }

    // auto enumval = keyLevelIdentifierToValue(keyStr);
    // if (enumval == LevelKeys::unknown) {
    //     log::error("Unknown key name: {} (for group: {})", keyStr, group);
    //     return std::nullopt;
    // }

    // return KeyAction{enumval, keyDown, group};



*/