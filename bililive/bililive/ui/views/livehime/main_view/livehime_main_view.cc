#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"

#include <shellapi.h>
#include <VersionHelpers.h>

#include "base/ext/callable_callback.h"
#include "base/notification/notification_service.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/stringprintf.h"

#include "ui/base/win/dpi.h"
#include "ui/gfx/icon_util.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/painter.h"

#include "bilibase/win/win10_ver.h"

#include "bililive/app/bililive_dll_resource.h"
#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/danmaku_hime/danmakuhime_data_handler.h"
#include "bililive/bililive/livehime/danmaku_hime/danmaku_hime_pref_service.h"
#include "bililive/bililive/livehime/obs/output_controller.h"
#include "bililive/bililive/livehime/obs/source_creator.h"
#include "bililive/bililive/log_report/app_integrity_checking.h"

#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_menu.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/faux_audient_effect_view.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_native_widget_win.h"
#include "bililive/bililive/ui/views/livehime/status_bar/livehime_status_bar_view.h"
#include "bililive/bililive/ui/views/livehime/tips_dialog/fleet_achievement_dialog.h"
#include "bililive/bililive/ui/views/preview/livehime_preview_view.h"
#include "bililive/bililive/ui/views/tabarea/tabarea_materials_view.h"
#include "bililive/bililive/ui/views/tabarea/tabarea_view.h"

#include "bililive/bililive/utils/bililive_filetype_exts.h"
#include "bililive/bililive/utils/convert_util.h"
#include "bililive/bililive/utils/time_span.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/pref_names.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/bililive/ui/views/livehime/notify_window/live_handon_notify_window.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_hotkey_notify_view.h"
#include "bililive/bililive/utils/fast_forward_url_convert.h"
#include "bililive/bililive/bililive_process_impl.h"
#include <bililive/public/bililive/bililive_thread.h>

namespace
{
    const char *const kBililiveViewKey = "__BILILIVE_VIEW__";
    const SkColor clrMainWindowsBg = SkColorSetRGB(221, 225, 228);
    const int kTabareaSeparatorLineThickness = GetLengthByDPIScale(4);

    enum TrayContextMenuId
    {
        TrayContextMenuId_MainWnd,
        TrayContextMenuId_Danmaku,
        TrayContextMenuId_Exit,
    };

    bool IsFileTypeDroppable(const base::FilePath& filepath)
    {
        bililive::SourceType dummy;
        return bililive::GuessSourceType(filepath, dummy);
    }

    void ShowWarningMessage(const base::string16& msg)
    {
        livehime::ShowMessageBox(GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeView(),
            GetLocalizedString(IDS_TIP_DLG_TIP),
            msg,
            GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_IKNOW),
            nullptr, nullptr, gfx::ALIGN_LEFT, L"", true);
        ::MessageBeep(MB_ICONWARNING);
    }

    void OnVerifyFailDialog(const string16& btn, void* data) {
        DCHECK(data);
        scoped_ptr<std::wstring> button_url(static_cast<std::wstring*>(data));

        if (btn != GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_IKNOW)) {
            ShellExecuteW(nullptr, L"open", bililive::FastForwardChangeEnv((*button_url)).c_str(), nullptr, nullptr, SW_SHOW);
        }
    }

    BOOL CALLBACK EnumAllMonitor(HMONITOR monitor, HDC hdc, LPRECT rc, LPARAM data)
    {
        gfx::Rect* rect = (gfx::Rect*)data;
        *rect = gfx::UnionRects(*rect, gfx::Rect(*rc));
        return true;
    }

    gfx::Rect GetAllMonitorUnionRect()
    {
        static gfx::Rect rect;
        if (rect.IsEmpty())
        {
            bool success = !!::EnumDisplayMonitors(NULL, NULL, EnumAllMonitor, (LPARAM)&rect);
            DCHECK(success);
        }
        return rect;
    }

}

const char BililiveOBSView::kViewClassName[] = "BililiveView";

BililiveOBSView *BililiveOBSView::CreateBililiveObsView()
{
    return nullptr;
}

BililiveOBSView::BililiveOBSView()
    : bililive_obs_(GetBililiveProcess()->bililive_obs())
    , weakptr_factory_(this)
{
}

BililiveOBSView::~BililiveOBSView()
{
    bililive_obs_->Shutdown();
}

string16 BililiveOBSView::GetWindowTitle() const
{
    base::string16 wstr = GetLocalizedString(IDS_BILILIVE_TITLE);
    return wstr;
}

bool BililiveOBSView::ExecuteWindowsCommand(int command_id)
{
    return false;
}

void BililiveOBSView::OnWidgetMove()
{
}

void BililiveOBSView::ViewHierarchyChanged(const ViewHierarchyChangedDetails &details)
{
    if (details.is_add && details.child == this)
    {
        InitViews();
    }
}

gfx::Rect BililiveOBSView::get_preview_rect()
{
    return gfx::Rect();
}

int BililiveOBSView::NonClientHitTest(const gfx::Point &point)
{
    gfx::Point point_bililive_view = point;
    return HTNOWHERE;
}

void BililiveOBSView::DisplaySysBalloon(const base::string16& title, const base::string16& msg)
{
    scoped_ptr<SkBitmap> bitmap = IconUtil::CreateSkBitmapFromIconResource(
        GetModuleHandle(bililive::kBililiveResourcesDll), IDR_MAINFRAME, IconUtil::kMediumIconSize);
    status_icon_->DisplayBalloon(
        gfx::ImageSkia(gfx::ImageSkiaRep(*bitmap, ui::GetScaleFactorFromScale(ui::win::GetDeviceScaleFactor()))),
        title, msg);
}

void BililiveOBSView::AttachMergedDanmakuMainView(views::View* danmaku_merged_view)
{
}

void BililiveOBSView::ShowDanmakuMergedView(bool show)
{
}

void BililiveOBSView::RequestCoverImage()
{
}

void BililiveOBSView::AddPluggableInfo(const PluggableInfo& info)
{
}

void BililiveOBSView::SetRoomTitleEditFocus()
{
}

void BililiveOBSView::OpenChatRoomView()
{
}

void BililiveOBSView::CloseChatRoomView()
{
}

void BililiveOBSView::Observe(int type,
    const base::NotificationSource& source,
    const base::NotificationDetails& details)
{
}

void BililiveOBSView::InitViews()
{
}

void BililiveOBSView::UninitViews()
{
}

void BililiveOBSView::ShowTray()
{
    if (!status_tray_)
    {
        status_tray_.reset(StatusTray::Create());
    }
    if (status_tray_ && !status_icon_)
    {

        gfx::ImageSkia image_skia(*GetImageSkiaNamed(IDR_LIVEHIME_STATUS));

        status_icon_ = status_tray_->CreateStatusIcon(
            StatusTray::NOTIFICATION_TRAY_ICON, image_skia,
            GetLocalizedString(IDS_BILILIVE_TITLE));
        if (status_icon_)
        {
            status_icon_->AddObserver(this);

            UpdateMenu();
        }
    }
}

void BililiveOBSView::OnStatusIconClicked()
{
    if (!GetWidget()->IsVisible())
    {
        GetWidget()->Show();
    }

    if (GetWidget()->IsMinimized())
    {
        GetWidget()->Restore();
    }

    GetWidget()->Activate();
}

void BililiveOBSView::ExecuteCommand(int command_id, int event_flags)
{
    switch (command_id)
    {
    case TrayContextMenuId_MainWnd:
    {
        bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_ACTIVE_MAIN_WINDOW);
    }
    break;
    case TrayContextMenuId_Exit:
    {
        bool exit_immediately = true;
        bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_CLOSE_APP, CommandParams<bool>(&exit_immediately));
    }
    break;
    default:
        break;
    }
}

void BililiveOBSView::UpdateMenu()
{
    LivehimeMenuItemView *menu = new LivehimeMenuItemView(this);
    menu->AppendMenuItem(TrayContextMenuId_MainWnd,
        GetLocalizedString(IDS_LIVEHIME_TRAY_MODEL_MAIN_SHOW));
    menu->AppendMenuItem(TrayContextMenuId_Danmaku,
        GetLocalizedString(IDS_LIVEHIME_TRAY_MODEL_DANMAKU_SHOW));
    menu->AppendMenuItem(-1, L"", views::MenuItemView::SEPARATOR);
    menu->AppendMenuItem(TrayContextMenuId_Exit, GetLocalizedString(IDS_LIVEHIME_TRAY_MODEL_EXIT));
    status_icon_->SetContextMenu(menu);
}

views::NonClientFrameView* BililiveOBSView::CreateNonClientFrameView(views::Widget *widget)
{
    BililiveNonTitleBarFrameView* frame_view = new BililiveNonTitleBarFrameView(this);
    frame_view->SetEnableDragMove(true);
    frame_view->SetEnableDragSize(true);
    frame_view->SetCacheMaxminSize(false);
    return frame_view;
}

void BililiveOBSView::OnBoundsChanged(const gfx::Rect& previous_bounds)
{
}

views::View* BililiveOBSView::GetEventHandlerForPoint(const gfx::Point& point)
{
    return __super::GetEventHandlerForPoint(point);
}

void BililiveOBSView::OnThemeChanged()
{
    __super::OnThemeChanged();
    set_background(views::Background::CreateSolidBackground(clrMainWindowsBg));
}

void BililiveOBSView::OnPaintBackground(gfx::Canvas* canvas)
{
    __super::OnPaintBackground(canvas);
}

void BililiveOBSView::ChildPreferredSizeChanged(View* child)
{
}

bool BililiveOBSView::GetDropFormats(
    int *formats,
    std::set<ui::OSExchangeData::CustomFormat> *custom_formats)
{
    return false;
}

bool BililiveOBSView::AreDropTypesRequired()
{
    return true;
}

bool BililiveOBSView::CanDrop(const ui::OSExchangeData &data)
{
    return true;
}

void BililiveOBSView::OnDragEntered(const ui::DropTargetEvent &event)
{
}

int BililiveOBSView::OnDragUpdated(const ui::DropTargetEvent &event)
{
    if (preview_view()->bounds().Contains(event.location()))
    {
        return ui::DragDropTypes::DRAG_COPY;
    }

    return ui::DragDropTypes::DRAG_NONE;
}

void BililiveOBSView::OnDragExited()
{
}

int BililiveOBSView::OnPerformDrop(const ui::DropTargetEvent &event)
{
    return ui::DragDropTypes::DRAG_COPY;
}

void BililiveOBSView::OnWidgetVisibilityChanged(views::Widget* widget, bool visible)
{
    UpdateMenu();
}

void BililiveOBSView::OnWidgetDestroying(views::Widget* widget)
{
}

void BililiveOBSView::NewDanmaku(const DanmakuInfo& danmaku)
{
}

void BililiveOBSView::OnLiveLayoutModelChanged(bool user_invoke)
{
}

void BililiveOBSView::OnLiveRoomPreviewStatusChanged(bool show, bool landscape)
{
}

void BililiveOBSView::ProcessingPresetMaterialMsg(PresetMaterial::PresetMaterialMsgInfo msgInfo)
{
}