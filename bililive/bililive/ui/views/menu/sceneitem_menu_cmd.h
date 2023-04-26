#ifndef BILILIVE_BILILIVE_UI_VIEWS_MENU_SCENEITEM_MENU_CMD_H
#define BILILIVE_BILILIVE_UI_VIEWS_MENU_SCENEITEM_MENU_CMD_H

#include "base/memory/scoped_ptr.h"

struct SCENEITEMCMD_
{
    SCENEITEMCMD_(const string16 &scene_name, const string16 &item_name)
        : item_name_(item_name)
        , scene_name_(scene_name)
    {
    };
	SCENEITEMCMD_(const string16& scene_name, const string16& item_name, const string16& extend_parameter)
		: item_name_(item_name)
		, scene_name_(scene_name)
        , extend_parameter_(extend_parameter)
	{
	};
    ~SCENEITEMCMD_(){}

    string16 scene_name() const { return scene_name_; }
    string16 item_name() const { return item_name_; }
    const std::vector<string16>& scene_items() { return scene_items_; }

    bool no_popup = false;
    string16 scene_name_;
    string16 item_name_;
    string16 extend_parameter_;
    std::vector<string16> scene_items_;
};

typedef scoped_ptr<SCENEITEMCMD_> SceneItemCmd;

#endif // BILILIVE_BILILIVE_UI_VIEWS_MENU_SCENEITEM_MENU_CMD_H