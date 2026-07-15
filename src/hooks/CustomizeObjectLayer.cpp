#include <Geode/modify/CustomizeObjectLayer.hpp>
#include "EditorUI.hpp"

class $modify(CustomizeObjectLayer)
{
    void onClose(CCObject* sender) {
        CustomizeObjectLayer::onClose(sender);
        static_cast<MyEditorUI*>(EditorUI::get())->m_fields->editingNormalTextObject = false;
    }
};