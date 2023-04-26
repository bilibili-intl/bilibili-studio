#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_PRESET_MATERIAL_COMMON_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_PRESET_MATERIAL_COMMON_H_

#include "ui/views/view.h"
#include "ui/views/controls/image_view.h"
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/livehime/obs/source_creator.h"

namespace PresetMaterial
{
	enum class LiveMainType
	{
		LiveType_Game = 0,
		LiveType_Art,
		LiveType_Voice,
		LiveType_Virtual,
		LiveType_Learning
	};

	enum class MaterialType 
	{
		Type_PcGame = 0,
		Type_MobileGame,
		Type_Monitor,
		Type_Windows,
		Type_Screenshot,
		Type_ThirdPartyStreaming,
		Type_Camera,
		Type_Image,
		Type_Vtuber
	};

	enum PresetMaterialMsgType
	{
		MsgType_Skip_Enter_Main_View = 0,
		MsgType_Relayout,
		MsgType_UpdateCurSelectSourceType,
		MsgType_ShowScreenshotView,
		MsgType_StartBroadcast
	};

	struct PresetMaterialMsgInfo
	{
		PresetMaterialMsgInfo(PresetMaterialMsgType msgType) :msgType_(msgType) {};
		PresetMaterialMsgType msgType_;
		MaterialType material_type_;
	};

	std::string GetLiveMainTypeName(LiveMainType main_type);
	std::string GetLiveSubTypeName(MaterialType material_type);
	std::string GetMaterialTypeName(MaterialType material_type);
}


class PresetMaterialBridge
{
public:
    explicit PresetMaterialBridge() {};
    virtual ~PresetMaterialBridge() {};
	virtual void ProcessingPresetMaterialMsg(PresetMaterial::PresetMaterialMsgInfo msgInfo) = 0;
	virtual bool GoToStartBroadcasting(PresetMaterial::MaterialType cur_select_material_type) { return false; };
};

class PresetMaterialBaseView:public views::View,public PresetMaterialBridge
{
public:
	explicit PresetMaterialBaseView(PresetMaterialBridge* parentObserver);
	virtual ~PresetMaterialBaseView();
protected:
	obs_proxy::SceneItem* AddSource(bililive::SourceType source_type, PresetMaterial::MaterialType material_type);
	void DelectCurViewAndSource();
	virtual bool StartBroadcastingCheckValid(PresetMaterial::MaterialType cur_select_material_type);
protected:
	views::View* cur_select_view_ = nullptr;	
	PresetMaterialBridge* parentObserver_ = nullptr;
private:
	void* cur_select_source_params = nullptr;
};


//中间内容互斥显示控制类
class PresetSingleChildShowContainerView : public views::View
{
public:
	PresetSingleChildShowContainerView() = default;
	void ReLayout();
	void ShowChildView(views::View *child_view);
	void SetFixedSize(gfx::Size fixed_size) { fixed_size_ = fixed_size; }
	void SetCenterHorizontally(bool center_horizontally) { center_horizontally_ = center_horizontally; }
protected:
	// View
	gfx::Size GetPreferredSize() override;

private:
	bool center_horizontally_ = false;//是否水平居中
	gfx::Size pref_size_ = gfx::Size(0, 0);
	gfx::Size fixed_size_ = gfx::Size(0, 0);
};


#endif