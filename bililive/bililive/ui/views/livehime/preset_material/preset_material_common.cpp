#include "preset_material_common.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "bilibase/basic_types.h"
#include "ui/views/widget/widget.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "ui/views/widget/widget_delegate.h"
#include "bililive/bililive/ui/bililive_command_handler_livehime.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_property_base_view.h"

std::string PresetMaterial::GetLiveMainTypeName(PresetMaterial::LiveMainType main_type)
{
	std::string name;
	switch (main_type)
	{
	case PresetMaterial::LiveMainType::LiveType_Game:
		name = "game";
		break;
	case PresetMaterial::LiveMainType::LiveType_Art:
		name = "art";
		break;
	case PresetMaterial::LiveMainType::LiveType_Voice:
		name = "voice";
		break;
	case PresetMaterial::LiveMainType::LiveType_Virtual:
		name = "virtual";
		break;
	case PresetMaterial::LiveMainType::LiveType_Learning:
		name = "learning";
		break;
	default:
		break;
	}
	return name;
}

std::string PresetMaterial::GetLiveSubTypeName(PresetMaterial::MaterialType material_type)
{
	std::string name;
	switch (material_type)
	{
	case PresetMaterial::MaterialType::Type_PcGame:
		name = "pc_game";
		break;
	case PresetMaterial::MaterialType::Type_MobileGame:
		name = "mobile_game";
		break;
	case PresetMaterial::MaterialType::Type_Monitor:
	case PresetMaterial::MaterialType::Type_Windows:
	case PresetMaterial::MaterialType::Type_Screenshot:
		name = "screen_capture";
		break;
	case PresetMaterial::MaterialType::Type_ThirdPartyStreaming:
		name = "third_party_streaming";
		break;
	case PresetMaterial::MaterialType::Type_Camera:
		name = "camera";
		break;
	case PresetMaterial::MaterialType::Type_Image:
		name = "image";
		break;
	case PresetMaterial::MaterialType::Type_Vtuber:
		name = "vtuber";
		break;
	default:
		break;
	}
	return name;
}

std::string PresetMaterial::GetMaterialTypeName(PresetMaterial::MaterialType material_type)
{
	std::string name;
	switch (material_type)
	{
	case PresetMaterial::MaterialType::Type_PcGame:
		name = "pc_game";
		break;
	case PresetMaterial::MaterialType::Type_MobileGame:
		name = "mobile_game";
		break;
	case PresetMaterial::MaterialType::Type_Monitor:
		name = "monitor";
		break;
	case PresetMaterial::MaterialType::Type_Windows:
		name = "windows";
		break;
	case PresetMaterial::MaterialType::Type_Screenshot:
		name = "screenshot";
		break;
	case PresetMaterial::MaterialType::Type_ThirdPartyStreaming:
		name = "third_party_streaming";
		break;
	case PresetMaterial::MaterialType::Type_Camera:
		name = "camera";
		break;
	case PresetMaterial::MaterialType::Type_Image:
		name = "image";
		break;
	case PresetMaterial::MaterialType::Type_Vtuber:
		name = "vtuber";
		break;
	default:
		break;
	}
	return name;
}

gfx::Size PresetSingleChildShowContainerView::GetPreferredSize()
{
	if (fixed_size_.width() > 0 && fixed_size_.height() > 0)
	{
		return fixed_size_;
	}
	if (pref_size_.width() == 0 || pref_size_.height() == 0)
	{
		return __super::GetPreferredSize();
	}
	return pref_size_;
}

void PresetSingleChildShowContainerView::ReLayout()
{
	for (int i = 0; i < this->child_count(); i++)
	{
		if (child_at(i)->visible())
		{
			pref_size_ = child_at(i)->GetPreferredSize();
			int x = 0;
			int y = 0;
			if (fixed_size_.width() > 0 && fixed_size_.height() > 0)
			{
				x = (fixed_size_.width() - pref_size_.width()) / 2;//中间
				y = (fixed_size_.height() - pref_size_.height()) / 2;
			}
			child_at(i)->SetBounds(x, y, pref_size_.width(), pref_size_.height());
		}
	}
}

void PresetSingleChildShowContainerView::ShowChildView(views::View* child_view)
{
	for (int i = 0; i < this->child_count(); i++)
	{
		if (child_at(i) == child_view)
		{
			child_at(i)->SetVisible(true);
			pref_size_ = child_at(i)->GetPreferredSize();
			bool fixed = fixed_size_.width() > 0 && fixed_size_.height() > 0;
			this->SetSize(fixed ? fixed_size_ : pref_size_);
			int x = 0;
			int y = 0;
			if (fixed)
			{
				if (center_horizontally_)//水平居中
				{
					x = (fixed_size_.width() - pref_size_.width()) / 2;//中间
					y = 0;
				}
				else
				{
					x = (fixed_size_.width() - pref_size_.width()) / 2;//中间
					y = (fixed_size_.height() - pref_size_.height()) / 2;
				}
			}
			child_at(i)->SetBounds(x, y, pref_size_.width(), pref_size_.height());
		}
		else
		{
			child_at(i)->SetVisible(false);
		}
	}
}


PresetMaterialBaseView::PresetMaterialBaseView(PresetMaterialBridge* parentObserver):parentObserver_(parentObserver)
{

}

PresetMaterialBaseView::~PresetMaterialBaseView()
{
	if (cur_select_source_params)
	{
		bililive::PresetMaterialAddSourceEnd(livehime::ResultCode::RC_CANCEL, cur_select_source_params);
		cur_select_source_params = nullptr;
	}
}

obs_proxy::SceneItem* PresetMaterialBaseView::AddSource(bililive::SourceType source_type, PresetMaterial::MaterialType material_type)
{
	void* source_params = nullptr;
	if (source_type == bililive::SourceType::Receiver){ 

		bililive::ProjectionData data(0);
		bililive::CreatingSourceParams params(source_type, &data);
		source_params = bililive::PresetMaterialAddSource(CommandParams<bililive::CreatingSourceParams>(&params));

	}else{
	
		bililive::CreatingSourceParams params(source_type);
		source_params = bililive::PresetMaterialAddSource(CommandParams<bililive::CreatingSourceParams>(&params));
	}

  
	bililive::SceneItemCmdHandlerParam* modal_param = static_cast<bililive::SceneItemCmdHandlerParam*>(source_params);
	if (modal_param && modal_param->scene_item)
	{
		cur_select_source_params = source_params;
		if (parentObserver_)
		{
			PresetMaterial::PresetMaterialMsgInfo msgInfo(PresetMaterial::MsgType_UpdateCurSelectSourceType);
			msgInfo.material_type_ = material_type;
			parentObserver_->ProcessingPresetMaterialMsg(msgInfo);
		}
		return modal_param->scene_item;
	}
	return nullptr;
}

void PresetMaterialBaseView::DelectCurViewAndSource()
{
	if (cur_select_view_)
	{
		delete cur_select_view_;
		cur_select_view_ = nullptr;
	}
	if (cur_select_source_params)
	{
		bililive::PresetMaterialAddSourceEnd(livehime::ResultCode::RC_CANCEL, cur_select_source_params);
		cur_select_source_params = nullptr;
	}
}

bool PresetMaterialBaseView::StartBroadcastingCheckValid(PresetMaterial::MaterialType cur_select_material_type)
{
	if (cur_select_view_)
	{
		if (cur_select_material_type == PresetMaterial::MaterialType::Type_PcGame ||
			cur_select_material_type == PresetMaterial::MaterialType::Type_MobileGame ||
			cur_select_material_type == PresetMaterial::MaterialType::Type_Monitor ||
			cur_select_material_type == PresetMaterial::MaterialType::Type_Windows)
		{
			livehime::BasePropertyView* base_pro = static_cast<livehime::BasePropertyView*>(cur_select_view_);
			if (!base_pro->CheckSetupValid())
			{
				return false;
			}
			if (!base_pro->SaveSetupChange())
			{
				return false;
			}
		}
	}

	if (cur_select_source_params)
	{
		bililive::PresetMaterialAddSourceEnd(livehime::ResultCode::RC_OK, cur_select_source_params);
		cur_select_source_params = nullptr;//这里直接置为null，上面函数PresetMaterialAddSourceEnd会删除
		return true;
	}
	return false;
}