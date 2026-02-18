#include "EditorUI.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/CreateMenuItem.hpp>
#include <Geode/binding/EditButtonBar.hpp>
#include <Geode/binding/EditorUI.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/TextGameObject.hpp>
#include "Geode/loader/Mod.hpp"
#include "Geode/utils/cocos.hpp"
#include "InputTriggerPopup.hpp"


constexpr int INPUT_TRIGGER_ID = 14999; // just below Object Groups limit

using namespace geode::prelude;

void MyEditorUI::setupCreateMenu() {
    EditorUI::setupCreateMenu();

    if (!Mod::get()->getSettingValue<bool>("trigger-ui")) {
        return;
    }

    auto bar = CCArrayExt<EditButtonBar*>(m_createButtonBars)[12];

    auto btn = getCreateBtn(INPUT_TRIGGER_ID, 1);

    m_createButtonArray->addObject(btn);

    int idx = 0;
    for (auto btn : CCArrayExt<CreateMenuItem*>(bar->m_buttonArray)) {
        idx++;
        if (btn->m_objectID == 1595) {
            break;
        }
    }

    bar->m_buttonArray->insertObject(btn, idx);

    bar->reloadItems(GameManager::get()->getIntGameVariable("0049"), GameManager::get()->getIntGameVariable("0050"));
}

CreateMenuItem* MyEditorUI::getCreateBtn(int id, int bg) {
    if (!Mod::get()->getSettingValue<bool>("trigger-ui")) {
        return EditorUI::getCreateBtn(id, bg);
    }

    if (id == INPUT_TRIGGER_ID) {
        auto btn = getCreateBtn(1, 4);

        auto arr = CCArray::create();
        auto spr =
                spriteFromObjectString("1,914,31,aW5mX2lucDplZGl0b3JUYWIgPSAw", false, false, 0, arr, nullptr, nullptr);

        spr->setScale(std::min(32.f / spr->getContentHeight(), 32.f / spr->getContentWidth()));

        auto buttonSpr = static_cast<ButtonSprite*>(btn->getNormalImage());
        if (auto obj = buttonSpr->m_subSprite) {
            obj->setVisible(false);
        }
        buttonSpr->addChild(spr);
        spr->setPosition({20, 21});

        btn->m_objectID = INPUT_TRIGGER_ID;
        btn->setTag(INPUT_TRIGGER_ID);
        return btn;
    }
    return EditorUI::getCreateBtn(id, bg);
}

void MyEditorUI::onCreateObject(int objectID) {
    if (!Mod::get()->getSettingValue<bool>("trigger-ui")) {
        return EditorUI::onCreateObject(objectID);
    }

    if (objectID != INPUT_TRIGGER_ID)
        return EditorUI::onCreateObject(objectID);
    objectID = 914;

    EditorUI::onCreateObject(objectID);

    if (m_selectedObject) {
        auto textObj = static_cast<TextGameObject*>(m_selectedObject);
        textObj->updateTextObject("inf_inp:1 empty 1 = 0", false);
    }
}

void MyEditorUI::clickOnPosition(cocos2d::CCPoint position) {
    if (!Mod::get()->getSettingValue<bool>("trigger-ui")) {
        return EditorUI::clickOnPosition(position);
    }

    EditorUI::clickOnPosition(position);
    if (m_selectedObjectIndex != 914)
        return;
    if (m_selectedMode != 2)
        return;

    auto nodePos = m_editorLayer->m_objectLayer->convertToNodeSpace(position);

    auto obj = m_editorLayer->objectAtPosition(nodePos);
    auto textGameObject = typeinfo_cast<TextGameObject*>(obj);
    if (!textGameObject)
        return;

    std::string_view view = textGameObject->m_text;
    if (view.starts_with("inf_inp:")) {
        m_selectedObjectIndex = INPUT_TRIGGER_ID;
        updateCreateMenu(true);
    }
}

void MyEditorUI::editObject(cocos2d::CCObject* sender) {
    if (!Mod::get()->getSettingValue<bool>("trigger-ui")) {
        return EditorUI::editObject(sender);
    }

    if(!m_selectedObject) {
        return EditorUI::editObject(sender);
    }

    if(m_selectedObject->m_objectID == 914) {
        return InputTriggerPopup::create(static_cast<TextGameObject*>(m_selectedObject))->show();
    }

    EditorUI::editObject(sender);
}
