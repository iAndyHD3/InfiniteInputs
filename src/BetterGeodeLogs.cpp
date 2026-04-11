#include "BetterGeodeLogs.hpp"
#include "Geode/loader/SettingV3.hpp"

$on_mod(Loaded) {
    geode::listenForSettingChanges<bool>("enable-logs", [](bool value) {
        BetterGeodeLogs::logsEnabled = value;
    });
}