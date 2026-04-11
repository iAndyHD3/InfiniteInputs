#pragma once

#include <Geode/loader/Log.hpp>
#include <fmt/format.h>
#include <string_view>
#include <utility>


/**
 * Enhanced logging interface for Geode SDK.
 *
 * Provides Java/Android-style tagged logging with automatic [tag] prefixing.
 * Supports full fmtlib formatting (compile-time checked) and perfect forwarding of arguments.
 *
 * Usage examples:
 *   Log.i("MyMod", "Player position: {}", playerPos);
 *   Log.e("Network", "Failed with code {}: {}", errorCode, errorMsg);
 *   Log.d("Debug", "Simple message without args");
 *   Log.w("Config", "Missing key, using default: {}", defaultValue);
 *
 * Output format (in console / log files):
 *   [MyMod] Player position: {x: 123, y: 456}
 *
 * Supports both `Log::i(...)` and `Log.i(...)` syntax.
 */

// you need to set this in your build system
#if !defined(BETTER_GEODE_LOGS_VAR_NAME)
#define BETTER_GEODE_LOGS_VAR_NAME Log
#endif

namespace BetterGeodeLogs {

//default if there is no setting like this so its good
inline bool logsEnabled = geode::Mod::get()->getSettingValue<bool>("enable-logs");
struct Log {

    template <typename... Args>
    static void i(std::string_view tag, fmt::format_string<Args...> fmt_str, Args&&... args) {
        if(!logsEnabled) return;

        std::string full_fmt = fmt::format("[{}] {}", tag, fmt_str.get());
        geode::log::info(fmt::runtime(full_fmt), std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void e(std::string_view tag, fmt::format_string<Args...> fmt_str, Args&&... args) {
        if(!logsEnabled) return;

        std::string full_fmt = fmt::format("[{}] {}", tag, fmt_str.get());
        geode::log::error(fmt::runtime(full_fmt), std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void w(std::string_view tag, fmt::format_string<Args...> fmt_str, Args&&... args) {
        if(!logsEnabled) return;

        std::string full_fmt = fmt::format("[{}] {}", tag, fmt_str.get());
        geode::log::warn(fmt::runtime(full_fmt), std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void d(std::string_view tag, fmt::format_string<Args...> fmt_str, Args&&... args) {
        if(!logsEnabled) return;

        std::string full_fmt = fmt::format("[{}] {}", tag, fmt_str.get());
        geode::log::debug(fmt::runtime(full_fmt), std::forward<Args>(args)...);
    }
};

}

inline BetterGeodeLogs::Log BETTER_GEODE_LOGS_VAR_NAME;