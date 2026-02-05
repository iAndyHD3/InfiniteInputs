#include "TextParsing.hpp"
#include <Geode/modify/Modify.hpp>
#include <Geode/utils/casts.hpp>
#include <Geode/utils/string.hpp>
#include <enchantum/enchantum.hpp>
#include <fmt/format.h>
#include <scn/scan.h>
#include "Geode/loader/Log.hpp"
#include "LevelKeys.hpp"


using namespace geode::prelude;


std::string getLabelFromKeyAction(const KeyAction& t) {
    return fmt::format(
            "inf_inp:1 1,{},2,{},3,{}", fixKeyName(enchantum::to_string(t.key)), static_cast<unsigned int>(t.keyDown),
            t.group);
}


std::optional<KeyAction> getParsedKeyAction(std::string_view t) {
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

    if (auto result = scn::scan<std::string, int, int>(t, "inf_inp:1 {} {} {}")) {
        auto& [key, keyDownParsed, groupParsed] = result->values();
        if (auto enumval = keyLevelIdentifierToValue(key); enumval != LevelKeys::unknown) {
            return KeyAction{enumval, keyDownParsed == 1, groupParsed};
        }
    }
    return std::nullopt;
}

bool foundOldFormatString(std::string_view t) {
    return scn::scan<std::string, int>(t, "inf_inp:{} = {}").has_value() ||
           scn::scan<std::string, char, int>(t, "inf_inp:{} {} = {}").has_value();
}

std::string getLabelFromClickAction(const ClickAction& t) {
    return fmt::format(
            "inf_inp:3 {} {} {} {} {}", t.collisionBlockId, t.groupIdCursorEnter, t.groupIdCursorExit,
            t.groupIdCursorDown, t.groupIdCursorUp);
}


std::optional<ClickAction> getClickActionFromLabel(std::string_view t) {
    if (auto result = scn::scan<int, int, int, int, int>(t, "inf_inp:3 {} {} {} {} {}")) {
        auto& [collisionBlockId, groupIdCursorEnter, groupIdCursorExit, groupIdCursorDown, groupIdCursorUp] =
                result->values();
        return ClickAction{collisionBlockId, groupIdCursorEnter, groupIdCursorExit, groupIdCursorDown, groupIdCursorUp};
    }
    return std::nullopt;
}

std::string getLabelFromSimpleKeyAction(const SimpleKeyAction& t) {
    return fmt::format("inf_inp:2 {} {}", t.group, fixKeyName(enchantum::to_string(t.key)));
}

std::optional<SimpleKeyAction> getSimpleKeyActionFromLabel(std::string_view t) {
    if (auto result = scn::scan<std::string, int>(t, "inf_inp:2 {} {}")) {
        auto& [key, group] = result->values();
        return SimpleKeyAction{keyLevelIdentifierToValue(key), group};
    }
    return std::nullopt;
}


std::optional<II_ObjectAction> parseObjectString(std::string_view t) {
    if (auto result = getParsedKeyAction(t)) {
        return result;
    }
    if (auto result = getSimpleKeyActionFromLabel(t)) {
        return result;
    }
    if (auto result = getClickActionFromLabel(t)) {
        return result;
    }
    return std::nullopt;
}
