#pragma once

#include <Geode/modify/Modify.hpp>
#include <Geode/modify/TextGameObject.hpp>


class $modify(MyTextGameObject, TextGameObject) {

    void setupInputTrigger();
    void customObjectSetup(std::vector<std::string>& p0, std::vector<void*>& p1);
    void updateTextObject(std::string p0, bool p1);
};
