#include "bililive/bililive/ui/views/livehime/sources_properties/source_property_base_view.h"

#include <shellapi.h>

#include "bilibase/scope_guard.h"
#include "bililive/bililive/livehime/help_center/help_center.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_camera_property_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_image_property_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_media_property_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_color_property_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_text_property_view.h"
#include "bililive/bililive/ui/views/tabarea/tabarea_materials_widget.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/bililive/livehime/event_report/event_report_center.h"

#include "ui/gfx/screen.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/views/controls/link.h"
#include "ui/views/layout/grid_layout.h"

#include "grit/generated_resources.h"


namespace{

enum SrcPropColumnSet
{
    DETAILCOLUMNSET = 0,
    BUTTONCOLUMNSET
};

}   // namespace

namespace livehime
{
    // BasePropertyData
    BasePropertyData::BasePropertyData()
    {
        max_label_width_ = LivehimePaddingCharWidth(LivehimeTitleLabel::GetFont()) * 4;
    }

    void BasePropertyData::PostSaveSetupChange(obs_proxy::SceneItem* scene_item)
    {
        secret::BehaviorEventMaterialType event_msg_id = secret::BehaviorEventMaterialType::Unknown;//埋点对应id
        switch (scene_item->type())
        {
        case obs_proxy::SceneItemType::DisplayCapture:
            event_msg_id = secret::BehaviorEventMaterialType::Screen;
            break;
        case obs_proxy::SceneItemType::WindowCapture:
            event_msg_id = secret::BehaviorEventMaterialType::Window;
            break;
        case obs_proxy::SceneItemType::GameCapture:
            event_msg_id = secret::BehaviorEventMaterialType::Game;
            break;
        case obs_proxy::SceneItemType::Text:
            event_msg_id = secret::BehaviorEventMaterialType::Text;
            break;
        case obs_proxy::SceneItemType::Slider:
        case obs_proxy::SceneItemType::Image:
        case obs_proxy::SceneItemType::ColorSource:
            event_msg_id = secret::BehaviorEventMaterialType::Image;
            break;
        case obs_proxy::SceneItemType::VideoCaptureDevice:
            event_msg_id = secret::BehaviorEventMaterialType::Camera;
            break;
        case obs_proxy::SceneItemType::MediaSource:
            event_msg_id = secret::BehaviorEventMaterialType::Media;
            break;
        case obs_proxy::SceneItemType::ReceiverSource:
            event_msg_id = secret::BehaviorEventMaterialType::Projection;
            break;
        case obs_proxy::SceneItemType::BrowserSource:
            event_msg_id = secret::BehaviorEventMaterialType::Browser;
            break;
        case obs_proxy::SceneItemType::AudioInputCapture:
            event_msg_id = secret::BehaviorEventMaterialType::Microphone;
            break;
        case obs_proxy::SceneItemType::AudioOutputCapture:
            event_msg_id = secret::BehaviorEventMaterialType::Loudspeaker;
            break;
        case obs_proxy::SceneItemType::DmkhimeSource:
            event_msg_id = secret::BehaviorEventMaterialType::DanmakuHime;
            break;
        default:
            break;
        }

        if (secret::BehaviorEventMaterialType::Unknown != event_msg_id)
        {
            livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::SourceSettingSubmitClick,
                "material_type:" + std::to_string((size_t)event_msg_id));
        }
    }

    // BasePropertyView
    BasePropertyView::BasePropertyView()
    {
        max_control_width_ = LivehimePaddingCharWidth(ftPrimary) * 20;
        non_header_column_padding_ = kPaddingColWidthEndsSmall + kPaddingColWidthForGroupCtrls + MaxLabelWidth();

        // 海外统一的背景图修改 rgb(34, 35, 49)
        set_background(views::Background::CreateSolidBackground(SkColorSetRGB(34, 35, 49)));
    }

    BaseScrollbarView::BaseScrollbarView()
    {
        max_control_width_ = LivehimePaddingCharWidth(ftPrimary) * 20;
        non_header_column_padding_ = kPaddingColWidthEndsSmall + kPaddingColWidthForGroupCtrls + MaxLabelWidth();
    }

    // LivehimeSrcPropView
    LivehimeSrcPropView::LivehimeSrcPropView(BasePropertyView* content_view, obs_proxy::SceneItem* scene_item)
        : BililiveWidgetDelegate(*content_view->GetSkiaIcon(),
            content_view->GetCaption(),
            TBB_CLOSE),

          scene_item_(scene_item),
          content_view_(content_view),
          ok_button_(nullptr),
          cancel_button_(nullptr)
    {

    }

    LivehimeSrcPropView::~LivehimeSrcPropView()
    {
    }

    void LivehimeSrcPropView::WindowClosing()
    {
        LOG(INFO) << "PropView close. " << this;

        if (!GetResultCode())
        {
            content_view_->Cancel();
        }
    }

    void LivehimeSrcPropView::CheckSourceTextSensitive(bool is_sensitive)
    {
        if (!is_sensitive)
        {
            auto old_name = scene_item_->name();

            if (!content_view_->SaveSetupChange())
            {
                LOG(INFO) << "PropView SaveSetupChange. " << this;
                return;
            }
            content_view_->PostSaveSetupChange(scene_item_);//文字源做敏感词判断，所以没有在ButtonPressed触发埋点，这里处理
            auto new_name = scene_item_->name();
            if (old_name != new_name) 
            {
				if (modal_end_dialog_handler_.data && !modal_end_dialog_handler_.closure.is_null() &&
					modal_end_dialog_handler_.data_type == EndDialogSignalHandler::DataType::DataType_SceneItemCmdHandlerParam)
				{
					bililive::SceneItemCmdHandlerParam* param = static_cast<bililive::SceneItemCmdHandlerParam*>(modal_end_dialog_handler_.data);
					param->item_name = new_name;
				}
                auto secret_core = GetBililiveProcess()->secret_core();
                auto msg = std::string("type:").append(scene_item_->type_name());
                secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
                    secret::LivehimeBehaviorEvent::LivehimeSceneRename1,
                    secret_core->account_info().mid(), msg).Call();
            }
            SetResultCode(IDOK);
            GetWidget()->Close();
        }
    }

    void LivehimeSrcPropView::OnCreateNonClientFrameView(views::NonClientFrameView *non_client_frame_view)
    {
        DCHECK(strcmp(non_client_frame_view->GetClassName(), BililiveFrameView::kDevViewClassName) == 0);
        ((BililiveFrameView*)non_client_frame_view)->SetTitlebarDelegate(this);
    }

    void LivehimeSrcPropView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
    {
        if (details.child == this)
        {
            if (details.is_add)
            {
                InitView();
                InitData();
            }
        }
    }

    bool LivehimeSrcPropView::OnTitleBarButtonPressed(TitleBarButton button)
    {
        if (button == TBB_HELP)
        {
            switch (scene_item_->type())
            {
            case obs_proxy::SceneItemType::MediaSource:
            case obs_proxy::SceneItemType::AudioInputCapture:
            case obs_proxy::SceneItemType::AudioOutputCapture:
                ShowHelp(HelpType::SourceMedia);
                break;
            case obs_proxy::SceneItemType::DisplayCapture:
                ShowHelp(HelpType::SourceMonitor);
                break;
            case obs_proxy::SceneItemType::GameCapture:
                ShowHelp(HelpType::SourceGame);
                break;
            case obs_proxy::SceneItemType::WindowCapture:
                ShowHelp(HelpType::SourceWindow);
                break;
            case obs_proxy::SceneItemType::VideoCaptureDevice:
                ShowHelp(HelpType::SourceCamera);
                break;
            case obs_proxy::SceneItemType::ReceiverSource:
                ShowHelp(HelpType::SourceProjection);
                break;
                break;
            default:
                ShowHelp(HelpType::Home);
                break;
            }
            return false;
        }
        return true;
    }

    void LivehimeSrcPropView::InitView()
    {
        ResourceBundle &rb = ResourceBundle::GetSharedInstance();

        // 海外统一的背景图修改 rgb(34, 35, 49)
        set_background(views::Background::CreateSolidBackground(SkColorSetRGB(34, 35, 49)));

        views::GridLayout* layout = new views::GridLayout(this);
        SetLayoutManager(layout);

        views::ColumnSet* column_set = layout->AddColumnSet(SrcPropColumnSet::DETAILCOLUMNSET);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

        layout->StartRow(1.0f, SrcPropColumnSet::DETAILCOLUMNSET);
        layout->AddView(content_view_);

        ok_button_ = new LivehimeActionLabelButton(this, rb.GetLocalizedString(IDS_SRCPROP_COMMON_OK), true);
        ok_button_->SetIsDefault(true);
        cancel_button_ = new LivehimeActionLabelButton(this, rb.GetLocalizedString(IDS_SRCPROP_COMMON_CANCEL), false);

        column_set = layout->AddColumnSet(SrcPropColumnSet::BUTTONCOLUMNSET);
        column_set->AddPaddingColumn(1.0f, 0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForActionButton);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(1.0f, 0);

        layout->AddPaddingRow(0, kPaddingRowHeightForDiffGroups);
        layout->StartRow(0, SrcPropColumnSet::BUTTONCOLUMNSET);
        layout->AddView(ok_button_);
        layout->AddView(cancel_button_);

        layout->AddPaddingRow(0, kPaddingRowHeightEnds);

        LOG(INFO) << scene_item_->type_name() << " PropView Show. " << this;
    }

    void LivehimeSrcPropView::InitData() {
        //switch (scene_item_->type()) {
        //case obs_proxy::SceneItemType::WindowCapture:
        //case obs_proxy::SceneItemType::BrowserSource:
        //    SetCacheMaxminSize(false);
        //    break;
        //}
    }

    void LivehimeSrcPropView::EnableButtons(bool enable)
    {
        ok_button_->SetEnabled(enable);
        cancel_button_->SetEnabled(enable);
    }

    void LivehimeSrcPropView::ButtonPressed(views::Button* sender, const ui::Event& event)
    {
        LOG_IF(ERROR, !GetWidget()) << "PropView widget closed. " << this;
        LOG(INFO) << "PropView ButtonPressed. " << this;

        EnableButtons(false);
        auto enable_buttons = MAKE_SCOPE_GUARD{
            EnableButtons(true);
        };

        if (ok_button_ == sender)
        {
            auto old_name = scene_item_->name();
            if (!content_view_->CheckSetupValid())
            {
                LOG(INFO) << "PropView CheckSetupValid. " << this;
                return;
            }

            if (scene_item_->type() == obs_proxy::SceneItemType::Text)
            {
                using namespace std::placeholders;
                auto handle = std::bind(&LivehimeSrcPropView::CheckSourceTextSensitive, this, _1);

                TextPropDetailView* text_view = (TextPropDetailView*)content_view_;
                text_view->CheckSourceText(handle);
                return;
            }

            if (!content_view_->SaveSetupChange())
            {
                LOG(INFO) << "PropView SaveSetupChange. " << this;
                return;
            }
            content_view_->PostSaveSetupChange(scene_item_);
            auto new_name = scene_item_->name();
            if (old_name != new_name) 
            {
				if (modal_end_dialog_handler_.data && !modal_end_dialog_handler_.closure.is_null() &&
                    modal_end_dialog_handler_.data_type == EndDialogSignalHandler::DataType::DataType_SceneItemCmdHandlerParam)
				{
                    bililive::SceneItemCmdHandlerParam* param = static_cast<bililive::SceneItemCmdHandlerParam*>(modal_end_dialog_handler_.data);
                    param->item_name = new_name;
				}
                auto secret_core = GetBililiveProcess()->secret_core();
                auto msg = std::string("type:").append(scene_item_->type_name());
                secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
                    secret::LivehimeBehaviorEvent::LivehimeSceneRename1,
                    secret_core->account_info().mid(), msg).Call();
            }

            SetResultCode(IDOK);
            GetWidget()->Close();
        }
        else
        {
            if (!content_view_->Cancel())
            {
                LOG(INFO)<< "PropView Revert Change. " << this;
                return;
            }

            SetResultCode(IDCANCEL);
            GetWidget()->Close();
        }
        enable_buttons.Dismiss();
        LOG(INFO) << "Post PropView ButtonPressed. " << this;
    }


    // global function
    void OpenSourcePropertyWidget(obs_proxy::SceneItemType type, views::Widget* parent,
                                  obs_proxy::SceneItem* scene_item, ViewType view_type/* = VT_CREATESOURCE*/,
                                  const EndDialogSignalHandler *handler/* = nullptr*/, int projection_type,bool from_preset_matrial, const std::string& tab_type )
    {
        using namespace obs_proxy;

        bool adjust_position = false;
        bool pos_right_corner = false;
        BasePropertyView* base_view = nullptr;
        secret::BehaviorEventMaterialType event_msg_id = secret::BehaviorEventMaterialType::Unknown;//埋点对应id
        switch (type)
        {
        case SceneItemType::VideoCaptureDevice:
            event_msg_id = secret::BehaviorEventMaterialType::Camera;
            base_view = CreateCameraSourcePropertyDetailView(scene_item, tab_type);
            adjust_position = true;
            pos_right_corner = from_preset_matrial;
            break;
        case SceneItemType::GameCapture:

            break;
        case SceneItemType::DisplayCapture:
            event_msg_id = secret::BehaviorEventMaterialType::Screen;
            base_view = CreateMonitorSourcePropertyDetailView(scene_item);
            break;
        case SceneItemType::WindowCapture:
            event_msg_id = secret::BehaviorEventMaterialType::Window;
            base_view = CreateWindowSourcePropertyDetailView(scene_item);
            break;
        case SceneItemType::MediaSource:
            event_msg_id = secret::BehaviorEventMaterialType::Media;
            base_view = CreateMediaSourcePropertyDetailView(scene_item);
            break;
        case SceneItemType::Text:
            event_msg_id = secret::BehaviorEventMaterialType::Text;
            base_view = CreateTextSourcePropertyDetailView(scene_item);
            break;
        case SceneItemType::Slider:
            event_msg_id = secret::BehaviorEventMaterialType::Image;
            base_view = CreateAlbumSourcePropertyDetailView(scene_item);
            break;
        case SceneItemType::Image:
            event_msg_id = secret::BehaviorEventMaterialType::Image;
            base_view = CreateImageSourcePropertyDetailView(scene_item);
            break;
        case SceneItemType::ColorSource:
            event_msg_id = secret::BehaviorEventMaterialType::Image;
            base_view = CreateColorSourcePropertyDetailView(scene_item);
            break;
        case SceneItemType::ReceiverSource:
            adjust_position = true;
            break;
        case SceneItemType::BrowserSource:
            event_msg_id = secret::BehaviorEventMaterialType::Browser;
            base_view = CreateBrowserSourcePropertyDetailView(scene_item);
            break;
        default:
            LOG(WARNING) << "unexpected source property type = " << (int)type << ", name = " << scene_item->name().c_str();
            DCHECK(false);
        }

        if (base_view)
        {
            views::Widget* widget_ = new views::Widget();
            BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(widget_);
            native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_CLOSE);
            views::Widget::InitParams params;
            params.native_widget = native_widget;
            params.parent = parent->GetNativeView();
            params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
            LivehimeSrcPropView* srcpropview = new LivehimeSrcPropView(base_view, scene_item);
            CHECK(srcpropview != 0);

            BililiveWidgetDelegate::DoModalWidget(srcpropview, widget_, params, handler);

            //统一放到主界面左下角
			gfx::Display disp = gfx::Screen::GetNativeScreen()->GetPrimaryDisplay();
			gfx::Rect work_area = disp.work_area();
            gfx::Rect wnd_rect = widget_->GetWindowBoundsInScreen();
            int space = GetLengthByDPIScale(10);
            views::Widget* main_view_widget = GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget();
			if (main_view_widget)
			{
				gfx::Rect main_rect = main_view_widget->GetWindowBoundsInScreen();
                int x = pos_right_corner ? main_rect.x() + main_rect.width() - wnd_rect.width() - space : main_rect.x() + space;
                if (x < 0)
                {
                    x = space;
                }
                else if( x + wnd_rect.width()  > work_area.width())
                {
                    x = work_area.width() - wnd_rect.width() - space;
                }
				int y = main_rect.bottom() - wnd_rect.height() - space;
                if (y < 0)
                {
                    y = space;
				}
				else if (y + wnd_rect.height() > work_area.height())
				{
					y = work_area.height() - wnd_rect.height() - space;
				}
                widget_->SetBounds(gfx::Rect(x, y, wnd_rect.width(), wnd_rect.height()));
                adjust_position = false;
			}

            if (adjust_position)
            {
                gfx::Point position;
                position.set_x(work_area.x() + GetLengthByDPIScale(10));
                position.set_y(work_area.bottom() - wnd_rect.height() - GetLengthByDPIScale(10));
                widget_->SetBounds(gfx::Rect(position.x(), position.y(), wnd_rect.width(), wnd_rect.height()));
            }

            //埋点:
            if (event_msg_id != secret::BehaviorEventMaterialType::Unknown)
            {
                livehime::PolarisEventReport(
                          secret::LivehimePolarisBehaviorEvent::SourceSettingShow,
                          "material_type:" + std::to_string((size_t)event_msg_id));
            }
        }
    }

}   // namespace livehime
