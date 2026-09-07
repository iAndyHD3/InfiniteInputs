geode build -p android64 --config RelWithDebInfo -- -DANDROID_STL=c++_shared
adb push build-android64\iandyhd3.infinite_inputs.geode /storage/emulated/0/Android/media/com.geode.launcher/game/geode/mods
adb shell "am force-stop com.geode.launcher && am start -n com.geode.launcher/com.geode.launcher.GeometryDashActivity"