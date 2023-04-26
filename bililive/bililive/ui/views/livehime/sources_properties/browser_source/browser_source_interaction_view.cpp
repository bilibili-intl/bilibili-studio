#include "browser_source_interaction_view.h"

#include "base/notification/notification_service.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/preview/livehime_obs_preview_control.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/secret/bililive_secret.h"

#include "obs/obs-studio/libobs/obs.hpp"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_sources_wrapper_impl.h"

#include "ui/views/layout/fill_layout.h"


namespace
{
    LivehimeBrowserSourceInteractionView* single_instance = nullptr;

    class BrowserSourceOBSPreviewControl
        : public LivehimeOBSPreviewControl
    {
    public:
        explicit BrowserSourceOBSPreviewControl(obs_source_t* source)
            : LivehimeOBSPreviewControl(obs_proxy_ui::PreviewType::PREVIEW_SOURCE)
            , source_(source)
        {
        }

        virtual ~BrowserSourceOBSPreviewControl() = default;

        // obs_proxy_ui::OBSPreviewController
        obs_source_t* GetRenderSource() override
        {
            return source_;
        }
        
    private:
        OBSSource source_;
    };
}

void LivehimeBrowserSourceInteractionView::ShowForm(gfx::NativeView par_hwnd, obs_proxy::SceneItem* scene_item)
{
    if (!single_instance)
    {
        views::Widget *widget = new views::Widget();
        views::Widget::InitParams params;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.parent = par_hwnd;
        params.native_widget = new BililiveNativeWidgetWin(widget);

        single_instance = new LivehimeBrowserSourceInteractionView(scene_item);
        DoModalWidget(single_instance, widget, params);
    }
    else
    {
        if (single_instance->GetWidget())
        {
            single_instance->GetWidget()->Activate();
        }
    }
}

LivehimeBrowserSourceInteractionView::LivehimeBrowserSourceInteractionView(obs_proxy::SceneItem* scene_item)
    : BililiveWidgetDelegate(*GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_PROPERTY_BROWSER_TITLE),
                             // base::StringPrintf(L"Óë\'%ls\'½»»¥", base::UTF8ToWide(scene_item->name()).c_str()),
                             base::StringPrintf(GetLocalizedString(IDS_CONTENTMENU_INTERACTION_WITH).c_str(), base::UTF8ToWide(scene_item->name()).c_str()),
                             TBB_CLOSE | TBB_MAX)
{
    obs_proxy::SceneItemImpl* impl = static_cast<obs_proxy::SceneItemImpl*>(scene_item);
    preview_ctrl_ = new BrowserSourceOBSPreviewControl(impl->AsSource());
}

LivehimeBrowserSourceInteractionView::~LivehimeBrowserSourceInteractionView()
{
    single_instance = nullptr;
}

void LivehimeBrowserSourceInteractionView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details)
{
    if (details.is_add && details.child == this)
    {
        InitViews();
    }
}

gfx::Size LivehimeBrowserSourceInteractionView::GetPreferredSize()
{
    static gfx::Size size(GetLengthByDPIScale(660), GetLengthByDPIScale(560));
    return size;
}

void LivehimeBrowserSourceInteractionView::Layout()
{
    gfx::Rect rect = GetLocalBounds();
    rect.Inset(kPaddingColWidthForGroupCtrls, kPaddingColWidthForGroupCtrls);
    preview_ctrl_->SetBoundsRect(rect);
}

void LivehimeBrowserSourceInteractionView::WindowClosing()
{
    BililiveWidgetDelegate::WindowClosing();
}

void LivehimeBrowserSourceInteractionView::OnCreateNonClientFrameView(views::NonClientFrameView *non_client_frame_view)
{
    BililiveFrameView* frame_view = static_cast<BililiveFrameView*>(non_client_frame_view);
    frame_view->SetEnableDragSize(true);
}

void LivehimeBrowserSourceInteractionView::InitViews()
{
    AddChildView(preview_ctrl_);
}
