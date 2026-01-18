#include <Geode/binding/CreateMenuItem.hpp>
#include <Geode/modify/EditorUI.hpp>


class $modify(MyEditorUI, EditorUI) {
    void setupCreateMenu();
	void onCreateObject(int objectID);
    void clickOnPosition(cocos2d::CCPoint position);
    void editObject(cocos2d::CCObject* sender);
    CreateMenuItem* getCreateBtn(int id, int bg);

//     bool init(LevelEditorLayer* editorLayer);
//     GameObject* createObject(int objectID, cocos2d::CCPoint position);
    
};
