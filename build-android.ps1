geode build -p android64 --config Release -- -DANDROID_STL=c++_shared
adb -s 192.168.0.156:40447 push build-android64\iandyhd3.infinite_inputs.geode /storage/emulated/0/Android/media/com.geode.launcher/game/geode/mods
adb -s 192.168.0.156:40447 shell "am force-stop com.geode.launcher && am start -n com.geode.launcher/com.geode.launcher.GeometryDashActivity"