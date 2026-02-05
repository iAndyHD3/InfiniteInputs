#pragma once

#include <Geode/c++stl/aliastl.hpp>
#include <Geode/c++stl/string.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/modify/TextGameObject.hpp>


class $modify(MyTextGameObject, TextGameObject) {
    void setupInputTrigger();
    void customObjectSetup(gd::vector<gd::string>& p0, gd::vector<void*>& p1);
    void updateTextObject(gd::string p0, bool p1);
};
