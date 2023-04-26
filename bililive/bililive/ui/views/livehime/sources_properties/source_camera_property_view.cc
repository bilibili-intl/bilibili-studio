#include "source_camera_property_view.h"

#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/sources_properties/source_camera_property_presenter_impl.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_tabbed.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/beauty_base_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/beauty_camera_control_preview_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/beauty_tab_item_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_camera_base_settings_view.h"

#include "bililive/public/log_ext/log_constants.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"


class BeautyLayoutView :
    public views::View
{
public:
    BeautyLayoutView(views::View* preview_view, views::View* content_view) :
        preview_view_(preview_view),
        content_view_(content_view)
    {
    }

    ~BeautyLayoutView()
    {

    }

    gfx::Size GetPreferredSize() override
    {
        return preferred_size_;
    }

    void Layout() override
    {
        auto rt = GetContentsBounds();
        int view_height = (rt.height() - GetLengthByDPIScale(16)) / 2.0;

        preview_view_->SetBounds(0, 0, rt.width(), view_height);
        content_view_->SetBounds(0, view_height + GetLengthByDPIScale(16), rt.width(), view_height);
    }

    void SetPreferredSize(const gfx::Size& size)
    {
        preferred_size_ = size;
    }


protected:
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override
    {
        if (details.child == this &&
            details.is_add)
        {
            InitView();
        }
    }

private:
    views::View* preview_view_ = nullptr;
    views::View* content_view_ = nullptr;
    gfx::Size preferred_size_ = {};

    void InitView()
    {
        AddChildView(preview_view_);
        AddChildView(content_view_);
    }
};


static bool show_old_beauty_ = false;
enum { kOldBeauty = 10 };

namespace livehime
{
    BasePropertyView* CreateCameraSourcePropertyDetailView(obs_proxy::SceneItem* scene_item, const std::string& tab_type)
    {
        return new LivehimeCameraSrcPropView(scene_item, tab_type);
    }   // namespace livehime
};

// BililiveCameraSrcPropView
LivehimeCameraSrcPropView::LivehimeCameraSrcPropView(obs_proxy::SceneItem* scene_item, const std::string& tab_type) :
    item_(scene_item),
    tab_type_(tab_type)
{

}

LivehimeCameraSrcPropView::~LivehimeCameraSrcPropView()
{
    
}

void LivehimeCameraSrcPropView::InitView()
{
    auto& rb = ResourceBundle::GetSharedInstance();

    views::GridLayout *layout = new views::GridLayout(this);
    tab_view_ = new TabBarView(false, this);
    tab_view_->SetInset(gfx::Insets(GetLengthByDPIScale(17), 0, 0, 0));

    preview_control_view_ = new  BeautyCameraControlPreviewView();
    preview_view_ = new PreStartLiveSourcePreviewView(item_);
    float_view_ = new BililiveFloatingView(preview_view_, preview_control_view_);

    if (!preview_control_view_)
    {
        LOG(INFO) << "BeautyCameraController:: Preview Control View Not Created";
    }

    //¾µÍ·
    auto item = new BeautyTabItemView(tab_view_, rb.GetLocalizedString(IDS_CAMERA_BEAUTY_CAMERA));
    item->SetPreferredSize(GetSizeByDPIScale({ 112, 56 }));
    tab_view_->AddTab(item);

    content_view_ = new TabSelectView();

    common_settings_view_ = new SourceCameraBaseSettingsView(item_);
    common_settings_view_->Container()->set_id(0);
    content_view_->AddChildView(common_settings_view_->Container());

    BeautyLayoutView* beauty_layout_view = new BeautyLayoutView(float_view_, content_view_);
    beauty_layout_view->SetPreferredSize(GetSizeByDPIScale({533, 800}));

    //¾µÍ·
    SetLayoutManager(layout);
    layout->SetInsets(GetLengthByDPIScale(12), GetLengthByDPIScale(7), 0, GetLengthByDPIScale(33));

    auto col_set = layout->AddColumnSet(0);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0.16867f, views::GridLayout::USE_PREF, 0, 0);
    col_set->AddPaddingColumn(0, GetLengthByDPIScale(32));
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0.83132f, views::GridLayout::USE_PREF, 0, 0);

    layout->StartRow(1.0f, 0);
    layout->AddView(tab_view_);
    layout->AddView(beauty_layout_view);

    tab_view_->Select(0);

	if (tab_type_ == "beauty"){
		def_tabindex_ = 1;
	}
	else if (tab_type_ == "filter"){
		def_tabindex_ = 4;
	}
	else if (tab_type_ == "sticker"){
		def_tabindex_ = 5;
	}
	else {
		def_tabindex_ = 0;
	}

    SelectDefTab(def_tabindex_);

}

void LivehimeCameraSrcPropView::UninitView()
{

}

gfx::Size LivehimeCameraSrcPropView::GetPreferredSize()
{
    return gfx::Size(GetLengthByDPIScale(718), GetLengthByDPIScale(652));
}

bool LivehimeCameraSrcPropView::TabItemSelect(int index, TabBarView* tab_view)
{
    if (content_view_)
    {
        if (index != -1 && index >= 0 && index < content_view_->child_count())
        {
            //BeautyMaterialType tab{};
            auto id = content_view_->child_at(index)->id();

            if (id != kOldBeauty)
            {
                content_view_->SelectView(index);
                content_view_->Layout();
                GetWidget()->GetContentsView()->Layout();
            }
        }
    }

    return true;
}

void LivehimeCameraSrcPropView::InitData()
{

}

bool LivehimeCameraSrcPropView::CheckSetupValid()
{
    return true;
}

bool LivehimeCameraSrcPropView::SaveSetupChange()
{
    common_settings_view_->SaveSetupChange();

    return true;
}

void LivehimeCameraSrcPropView::PostSaveSetupChange(obs_proxy::SceneItem* scene_item)
{
    common_settings_view_->PostSaveSetupChange(scene_item);
}

bool LivehimeCameraSrcPropView::Cancel()
{
    return true;
}

gfx::ImageSkia* LivehimeCameraSrcPropView::GetSkiaIcon()
{
    return ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_PROPERTY_CAMERA_TITLE);
}

std::wstring LivehimeCameraSrcPropView::GetCaption()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();
    return res.GetLocalizedString(IDS_SRCPROP_CAMERA_CAPTION);
}

void LivehimeCameraSrcPropView::SelectDefTab(int index)
{
    tab_view_->Select(index);
}

