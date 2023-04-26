#include "bililive/bililive/ui/views/viddup/main_view/livehime_main_viddup_view.h"

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
#include "bililive/bililive/ui/views/viddup/tabarea/tabarea_viddup_view.h"
#include "bililive/bililive/ui/views/viddup/titlebar/titlebar_viddup_view.h"
#include "bililive/bililive/ui/views/viddup/toolbar/livehime_top_toolbar_viddup_view.h"
#include "bililive/bililive/ui/views/viddup/toolbar/toolbar_viddup_view.h"

#include "bililive/bililive/utils/bililive_filetype_exts.h"
#include "bililive/bililive/utils/convert_util.h"
#include "bililive/bililive/utils/time_span.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/pref_names.h"
#include "bililive/public/secret/bililive_secret.h"
#include <bililive/public/bililive/bililive_thread.h>
#include "bililive/bililive/ui/views/livehime/notify_window/live_handon_notify_window.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_hotkey_notify_view.h"

#include "bililive/bililive/utils/fast_forward_url_convert.h"
#include "bililive/bililive/bililive_process_impl.h"

namespace
{
    const char *const kBililiveViewKey = "__BILILIVE_VIEW__";
    //const SkColor clrMainWindowsBg = SkColorSetRGB(0x00, 0x00, 0x00);
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

class LiveHimeMainViewClientLayout : public views::LayoutManager
{
public:
    LiveHimeMainViewClientLayout()
    {
    }

    virtual ~LiveHimeMainViewClientLayout()
    {
    }

    virtual void Layout(views::View* host) override
    {
        if (!host->has_children())
            return;

        if (host->child_count() < 2)
        {
            return;
        }

        views::View* tabarea_view = host->child_at(0);
        views::View* main_content_view = host->child_at(1);
        if (!main_content_view->visible())
        {
            return;
        }

        auto content_bounds = host->GetContentsBounds();
        auto tabarea_size = tabarea_view->GetPreferredSize();
        tabarea_view->SetBoundsRect(gfx::Rect(0,0, tabarea_size.width(), content_bounds.height()));

		    int x = 0;
        if (tabarea_view->visible())
        {
            x = tabarea_size.width();
        }

        if (host->child_count() >= 3 && host->child_at(2)->visible())
        {
            views::View* danmaku_view = host->child_at(2);
            if (content_bounds.width() >= dmkhime::kDanmakuMainViewWidth + kTabareaSeparatorLineThickness)
            {
                gfx::Rect main_content_rect(x, 0, content_bounds.width() - x - dmkhime::kDanmakuMainViewWidth - kTabareaSeparatorLineThickness, content_bounds.height());
                main_content_view->SetBoundsRect(main_content_rect);

                gfx::Rect danmaku_rect(x + main_content_view->bounds().width() + kTabareaSeparatorLineThickness, 0, dmkhime::kDanmakuMainViewWidth, content_bounds.height());
                danmaku_view->SetBoundsRect(danmaku_rect);
            }
            else
            {
                //DCHECK(0);
                main_content_view->SetBoundsRect(gfx::Rect(x,0,content_bounds.width() - x, content_bounds.height()));
            }
        }
        else
        {
            main_content_view->SetBoundsRect(gfx::Rect(x, 0, content_bounds.width() - x, content_bounds.height()));
        }
    }

    virtual gfx::Size GetPreferredSize(views::View* host) override
    {
        if (host->child_count() == 2)
        {
            auto tabarea_size = host->child_at(0)->GetPreferredSize();
            auto content_size = host->child_at(1)->GetPreferredSize();
            gfx::Rect rect(tabarea_size.width() + content_size.width(), content_size.height());
            rect.Inset(-host->GetInsets());
            return rect.size();
        }
        else if(host->child_count() > 2)
        {
            auto tabarea_size = host->child_at(0)->GetPreferredSize();
            auto content_size = host->child_at(1)->GetPreferredSize();
            auto danmaku_size = host->child_at(2)->GetPreferredSize();

            gfx::Size preferred_size(tabarea_size.width() + content_size.width() + danmaku_size.width(), content_size.height());
            gfx::Rect rect(preferred_size);
            rect.Inset(-host->GetInsets());
            return rect.size();
        }

        return gfx::Size();
    }

private:
    DISALLOW_COPY_AND_ASSIGN(LiveHimeMainViewClientLayout);
};

const char BililiveOBSViddupView::kViewClassName[] = "BililiveViewViddup";

BililiveOBSView *BililiveOBSViddupView::CreateBililiveObsView()
{
    views::Widget *widget_ = new views::Widget();
    BilibiliLivehimeNativeWidgetWin *native_widget = new BilibiliLivehimeNativeWidgetWin(widget_);
    views::Widget::InitParams params;
    params.native_widget = native_widget;
    params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
    params.double_buffer = true;
    BililiveOBSViddupView*view = new BililiveOBSViddupView();
    BililiveWidgetDelegate::ShowWidget(view, widget_, params, false);

    if (bilibase::win::IsWindows10V2004OrGreater()) {
        PrefService* global_prefs = GetBililiveProcess()->global_profile()->GetPrefs();
        if (global_prefs->GetBoolean(prefs::kExcludeMainWindowFromCapture)) {
            BOOL ret = ::SetWindowDisplayAffinity(
                widget_->GetNativeView(), WDA_EXCLUDEFROMCAPTURE);
        } else {
            BOOL ret = ::SetWindowDisplayAffinity(
                widget_->GetNativeView(), WDA_NONE);
        }
    }

    bool bounds_with_pref = false;
    PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();
    if (pref)
    {
        int x = pref->GetInteger(prefs::kLastMainWinX);
        int y = pref->GetInteger(prefs::kLastMainWinY);
        int cx = pref->GetInteger(prefs::kLastMainWinWidth);
        int cy = pref->GetInteger(prefs::kLastMainWinHeight);
        if (cx > 0 && cy > 0)
        {
            gfx::Size min_size = widget_->GetMinimumSize();
            gfx::Size max_size = widget_->GetMaximumSize();

            cx = std::max(min_size.width(), cx);
            cx = std::min(max_size.width(), cx);
            cy = std::max(min_size.height(), cy);
            cy = std::min(max_size.height(), cy);

            // 枚举当前的全部显示器，看上次记录的窗口区域在哪个显示器
            // 窗口的最大最小尺寸是以主显示器的工作区来定的，如果上次是在副显示器关闭的且窗口尺寸大于主显示器，
            // 那么本次启动时计算出允许的最大最小尺寸还是以主显示器来定，也就是说在此种情况下的窗口区域还原不能完全和
            // 上次记录的一致，不过这都是小问题了，重要的是能保持在上次记录的显示器显示就好
            // 期望的窗口区域在全部屏幕的显示区内就按期望的来，不然就放到主显示器显示
            gfx::Rect window_rect = GetAllMonitorUnionRect();
            if (!window_rect.Contains(gfx::Rect(x, y, cx, cy)))
            {
                x = std::min(x, window_rect.width() - cx);
                x = std::max(x, 0);
                y = std::min(y, window_rect.height() - cy);
                y = std::max(y, 0);
            }

            // 如果上次是最大化，则也调用 SetWindowPos 来设置上次最大化之前的窗口位置
            bool is_maximized = pref->GetBoolean(prefs::kLastMainWinMaximized);

            // 配置文件中的记录是按设备尺寸记录的，所以在此直接用win api，
            ::SetWindowPos(widget_->GetNativeView(), nullptr, x, y, cx, cy,
                SWP_NOZORDER | SWP_HIDEWINDOW);

            // 最大限度确保窗口不被任务栏遮挡
            views::InsureWidgetVisible(widget_, views::InsureShowCorner::ISC_NEARBY);

            if (is_maximized) {
                widget_->Maximize();
            }

            bounds_with_pref = true;
        }
    }

    // 读记录完毕，可以再次记录尺寸变化了
    view->start_record_bounds_ = true;
    if (!bounds_with_pref)
    {
        // 初次开启程序，不管当前dpi是多少，一律根据显示器的尺寸优先保证预览区尺寸为1280*720像素
        gfx::Size min_size = view->GetMinimumSize();
        min_size.Enlarge(2, 2); // 边框一像素
        int scx = GetSystemMetrics(SM_CXSCREEN);
        gfx::Size prev_size = view->preview_view_->size();
        if (prev_size.width() < 1280)
        {
            if (scx - min_size.width() >= 1280 - prev_size.width())
            {
                gfx::Size enlarge_size(1280 - prev_size.width(), std::max(0, 720 - prev_size.height()));
                gfx::Size siCur = widget_->GetWindowBoundsInScreen().size();
                siCur.Enlarge(enlarge_size.width(), enlarge_size.height());
                widget_->CenterWindow(siCur);
            }
        }
        widget_->Show();
    }

    // 主界面启动曝光埋点
    livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::MainWndShow, "source_event:1");
    //EventShowPreviewViewDuration();

    return view;
}

BililiveOBSViddupView::BililiveOBSViddupView()
    : bililive_obs_(GetBililiveProcess()->bililive_obs())
    , run_source_("source_event:1")
    , activity_id_("resource_id:0")
    , weakptr_factory_(this)
{
}

BililiveOBSViddupView::~BililiveOBSViddupView()
{
    //bililive_obs_->Shutdown();
}

string16 BililiveOBSViddupView::GetWindowTitle() const
{
    base::string16 wstr = GetLocalizedString(IDS_BILILIVE_TITLE);
    return wstr;
}

bool BililiveOBSViddupView::ExecuteWindowsCommand(int command_id)
{
    return false;
}

void BililiveOBSViddupView::OnWidgetMove()
{
    RecordLastBounds();
}

void BililiveOBSViddupView::RecordLastBounds()
{
    if (start_record_bounds_ && !GetWidget()->IsMinimized())
    {
        PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();
        if (pref)
        {
            bool is_maximized = GetWidget()->IsMaximized();
            // 最大化时不记录当前窗口位置，以保留最大化之前的窗口位置
            if (!is_maximized) {
                gfx::Rect rcWin = GetWidget()->GetWindowBoundsInScreen();

                if (danmaku_dummy_view_->visible())
                {
                    //保存去除弹幕姬宽度的大小
                    int new_width = std::max(rcWin.width() - dmkhime::kDanmakuMainViewWidth - kTabareaSeparatorLineThickness, 0);
                    rcWin.set_width(new_width);
                }

                pref->SetInteger(prefs::kLastMainWinX, rcWin.x());
                pref->SetInteger(prefs::kLastMainWinY, rcWin.y());
                pref->SetInteger(prefs::kLastMainWinWidth, rcWin.width());
                pref->SetInteger(prefs::kLastMainWinHeight, rcWin.height());
            }

            pref->SetBoolean(prefs::kLastMainWinMaximized, is_maximized);
        }
    }
}

void BililiveOBSViddupView::PostUIInitialized()
{
    NotifyRequiredComponentsMissing();
    SchedulePluggable();
    //OnMeleeStatusChanged(livehime::MeleeStatus::StartMelee); //debug
}

void BililiveOBSViddupView::NotifyRequiredComponentsMissing()
{
    auto missing_files = livehime::FilesIntegrityChecking();
    if (!missing_files.empty())
    {
        std::string files = JoinString(missing_files, "\n");

        DisplaySysBalloon(GetLocalizedString(IDS_MAIN_VIEW_FILE_LOSS_LABEL),
            base::StringPrintf(GetLocalizedString(IDS_MAIN_VIEW_FILE_LOSS_LIST_LABEL).c_str(), missing_files.size())
             + base::ASCIIToUTF16(files));
    }
}

void BililiveOBSViddupView::SchedulePluggable()
{
    
}

void BililiveOBSViddupView::ViewHierarchyChanged(const ViewHierarchyChangedDetails &details)
{
    if (details.is_add && details.child == this)
    {
        InitViews();
        OnThemeChanged();

        notifation_register_.Add(this,
            bililive::NOTIFICATION_BILILIVE_ROOM_LOCK,
            base::NotificationService::AllSources());
        notifation_register_.Add(this,
            bililive::NOTIFICATION_BILILIVE_ROOM_END,
            base::NotificationService::AllSources());
        notifation_register_.Add(this,
            bililive::NOTIFICATION_BILILIVE_STUDIO_END,
            base::NotificationService::AllSources());
        notifation_register_.Add(this,
            bililive::NOTIFICATION_BILILIVE_END_WIN_TASK_VIEW_CLOSE,
            base::NotificationService::AllSources());
        notifation_register_.Add(this,
            bililive::NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_SUCCESS,
            base::NotificationService::AllSources());

        HandlePresetMaterial();
    }
}

void BililiveOBSViddupView::InitViews()
{
    GetWidget()->SetNativeWindowProperty(kBililiveViewKey, this);
    GetWidget()->SetNativeWindowProperty(Profile::kProfileKey, GetBililiveProcess()->profile());

    auto root_layout = new views::GridLayout(this);//包含标题栏和其下面整个界面
    SetLayoutManager(root_layout);

    auto root_col = root_layout->AddColumnSet(0);
    root_col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

    titlebar_view_ = new TitleBarViddupView();

    root_layout->StartRow(0, 0);
    root_layout->AddView(titlebar_view_);

    // client_area
    {
        main_content_view_ = new BililiveRecursivePreferredSizeChangedContainerView();

        tabarea_dummy_view_ = new views::View();
        tabarea_dummy_view_->SetLayoutManager(new views::FillLayout());

        tabarea_view_ = new TabAreaViddupView();
        tabarea_dummy_view_->AddChildView(tabarea_view_);

        danmaku_dummy_view_ = new views::View();
        danmaku_dummy_view_->SetLayoutManager(new views::FillLayout());
        danmaku_dummy_view_->SetVisible(false);

        // Under the title bar the whole interface, the kTabareaSeparatorLineThickness interval does not include the up and down or so
        client_view_ = new views::View();
        client_view_->SetLayoutManager(new LiveHimeMainViewClientLayout());
        client_view_->AddChildView(tabarea_dummy_view_);
        client_view_->AddChildView(main_content_view_);
        client_view_->AddChildView(danmaku_dummy_view_);

        // There's a real name authentication page and a client page for Live Hime
        acttually_client_view_ = new BililiveSingleChildShowContainerView;

        acttually_client_view_->AddChildView(client_view_);

		    root_col = root_layout->AddColumnSet(1);
        root_col->AddPaddingColumn(0, kTabareaSeparatorLineThickness);
		    root_col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
        root_col->AddPaddingColumn(0, kTabareaSeparatorLineThickness);
        root_layout->AddPaddingRow(0, kTabareaSeparatorLineThickness);
        root_layout->StartRow(1.0f, 1);
        root_layout->AddView(acttually_client_view_);
        root_layout->AddPaddingRow(0, kTabareaSeparatorLineThickness);
        // main_content_view_
        {
            // Left sidebar and middle preview area, and lower status sidebar
            auto main_layout = new views::GridLayout(main_content_view_);
            main_content_view_->SetLayoutManager(main_layout);

            auto main_col = main_layout->AddColumnSet(0);
            main_col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
			      main_col->AddPaddingColumn(0, kTabareaSeparatorLineThickness);
			      main_col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

            tabarea_stretch_view_ = new StretchControlViddupView(this, tabarea_view_, tabarea_dummy_view_);
            auto right_view = new BililiveRecursivePreferredSizeChangedContainerView();

            main_layout->StartRow(1.0f, 0);
            main_layout->AddView(tabarea_stretch_view_);
            main_layout->AddView(right_view);

            right_view->set_background(views::Background::CreateSolidBackground(GetColor(Theme)));
            // right_view
            {
                auto right_layout = new views::GridLayout(right_view);
                right_view->SetLayoutManager(right_layout);

                auto right_col = right_layout->AddColumnSet(0);
                right_col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

                right_col = right_layout->AddColumnSet(1);
                right_col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

                auto preview_area_view = new BililiveSingleChildShowContainerView();
                top_toolbar_view_ = new LivehimeTopToolbarViddupView();
  
                right_layout->StartRow(0, 0);
				        right_layout->AddView(top_toolbar_view_);
				        right_layout->AddPaddingRow(0, kTabareaSeparatorLineThickness);
                right_layout->StartRow(1.0f, 1);
                right_layout->AddView(preview_area_view);

                preview_view_ = new LivehimePreviewView();
                preview_area_view->AddChildView(preview_view_);
            }
        }
    }

    toolbar_view_ = new ToolbarViddupView();

    root_layout->AddPaddingRow(0, GetLengthByDPIScale(2));
    root_layout->StartRow(0, 1);
    root_layout->AddView(toolbar_view_);
    //root_layout->AddPaddingRow(0, kTabareaSeparatorLineThickness);

    ShowTray();
}

void BililiveOBSViddupView::UninitViews()
{
    tabarea_stretch_view_->Quit();
    notifation_register_.RemoveAll();

    if (LiveModelController::GetInstance())
    {
        //LiveModelController::GetInstance()->RemoveObserver(this);
    }
}

gfx::Rect BililiveOBSViddupView::get_preview_rect()
{
    return preview_view_->GetBoundsInScreen();
}

int BililiveOBSViddupView::NonClientHitTest(const gfx::Point &point)
{
    gfx::Point point_bililive_view = point;
    gfx::Rect child_rect = titlebar_view_->ConvertRectToWidget(titlebar_view_->GetLocalBounds());
    if (child_rect.Contains(point))
    {
        views::View::ConvertPointToTarget(this, titlebar_view_, &point_bililive_view);
        return titlebar_view_->GetHTComponentForFrame(point_bililive_view);
    }

    return HTNOWHERE;
}

void BililiveOBSViddupView::DisplaySysBalloon(const base::string16& title, const base::string16& msg)
{
    scoped_ptr<SkBitmap> bitmap = IconUtil::CreateSkBitmapFromIconResource(
        GetModuleHandle(bililive::kBililiveResourcesDll), IDR_MAINFRAME, IconUtil::kMediumIconSize);
    status_icon_->DisplayBalloon(
        gfx::ImageSkia(gfx::ImageSkiaRep(*bitmap, ui::GetScaleFactorFromScale(ui::win::GetDeviceScaleFactor()))),
        title, msg);
}

void BililiveOBSViddupView::AttachMergedDanmakuMainView(views::View* danmaku_merged_view)
{
    danmaku_dummy_view_->AddChildView(danmaku_merged_view);
    danmaku_dummy_view_->Layout();
    danmaku_dummy_view_->SchedulePaint();
}

void BililiveOBSViddupView::ShowDanmakuMergedView(bool show)
{
    if (danmaku_dummy_view_->visible() == show)
    {
        return;
    }

    danmaku_dummy_view_->SetVisible(show);

    bool is_maximized = GetWidget()->IsMaximized();
    auto bounds = GetWidget()->GetWindowBoundsInScreen();
    auto pref_size = GetPreferredSize();

    int new_width{ 0 };
    if (show && !is_maximized)
    {
        new_width = std::max(bounds.width() + dmkhime::kDanmakuMainViewWidth + kTabareaSeparatorLineThickness, pref_size.width());
        //GetWidget()->SetSize(gfx::Size(new_width, bounds.height()));
        //Solve the problem that the display is not in the middle of the screen when opening for the first time
        GetWidget()->CenterWindow(gfx::Size(new_width, bounds.height()));
    }
    else if(!show && !is_maximized)
    {
        new_width = std::max(bounds.width() - dmkhime::kDanmakuMainViewWidth - kTabareaSeparatorLineThickness, pref_size.width());
        //GetWidget()->SetSize(gfx::Size(new_width, bounds.height()));
        GetWidget()->CenterWindow(gfx::Size(new_width, bounds.height()));
    }

    Layout();

    // Fixed no hidden bug in the merged grey window of the occasional bullet screen when maximizing all pop-ups
    main_content_view_->InvalidateLayout();
}

void BililiveOBSViddupView::RequestCoverImage()
{
}

void BililiveOBSViddupView::AddPluggableInfo(const PluggableInfo& info)
{
    LOG(INFO) <<__FUNCTION__<< "thread_id = " << std::this_thread::get_id();
    base::MessageLoop::current()->PostTask(FROM_HERE,
        base::Bind(&BililiveOBSViddupView::PostUiAddPluggable, weakptr_factory_.GetWeakPtr(), info));
}

void BililiveOBSViddupView::SetRoomTitleEditFocus()
{
}

void BililiveOBSViddupView::OpenChatRoomView()
{
    tabarea_view_->OpenChatRoomView();
}

void BililiveOBSViddupView::CloseChatRoomView()
{
    tabarea_view_->CloseChatRoomView();
}

void BililiveOBSViddupView::Observe(int type,
    const base::NotificationSource& source,
    const base::NotificationDetails& details)
{
    switch (type)
    {
    case bililive::NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_SUCCESS:
        {
            if (pluggable_info_queue_.size() == 0) {
                PvEnventReport();
            }
        }
        break;
    case bililive::NOTIFICATION_BILILIVE_ROOM_LOCK:
        bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_LIVE_ROOM_LOCKED);
        break;
    case bililive::NOTIFICATION_BILILIVE_ROOM_END:
        if (!LivehimeLiveRoomController::GetInstance()->IsLiveReady() &&
            bililive::OutputController::GetInstance()->IsRestartLiveStreamingSafePeriodExpires())
        {
            LOG(INFO) << "stop live streaming by cmd=PREPARING";
            bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_STOP_LIVE_STREAMING);
        }
        else
        {
            LOG(INFO) << "Not safe to stop live streaming!";
        }
        break;
    case bililive::NOTIFICATION_BILILIVE_STUDIO_END:
        if (!LivehimeLiveRoomController::GetInstance()->IsLiveReady())
        {
            bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_STOP_STUDIO_LIVE_STREAMING);
        }
        break;
    case bililive::NOTIFICATION_BILILIVE_END_WIN_TASK_VIEW_CLOSE:
        if (melee_end_win_task_id_ != -1)
        {
            livehime::HidePreviewArea(livehime::PFLFWH_Second, melee_end_win_task_id_, true);
            melee_final_task_view_ = nullptr;
            melee_end_win_task_id_ = -1;
        }
        break;
    default:
        break;
    }
}

void BililiveOBSViddupView::ShowTray()
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

void BililiveOBSViddupView::OnStatusIconClicked()
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

void BililiveOBSViddupView::ExecuteCommand(int command_id, int event_flags)
{
    switch (command_id)
    {
    case TrayContextMenuId_MainWnd:
    {
        bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_ACTIVE_MAIN_WINDOW);
    }
    break;
    case TrayContextMenuId_Danmaku:
    {

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

void BililiveOBSViddupView::UpdateMenu()
{
    LivehimeMenuItemView *menu = new LivehimeMenuItemView(this);
    menu->AppendMenuItem(TrayContextMenuId_MainWnd,
        GetLocalizedString(IDS_LIVEHIME_TRAY_MODEL_MAIN_SHOW));
    //menu->AppendMenuItem(TrayContextMenuId_Danmaku,
    //    GetLocalizedString(IDS_LIVEHIME_TRAY_MODEL_DANMAKU_SHOW));
    menu->AppendMenuItem(-1, L"", views::MenuItemView::SEPARATOR);
    menu->AppendMenuItem(TrayContextMenuId_Exit, GetLocalizedString(IDS_LIVEHIME_TRAY_MODEL_EXIT));
    status_icon_->SetContextMenu(menu);
}

void BililiveOBSViddupView::OnBoundsChanged(const gfx::Rect& previous_bounds)
{
    RecordLastBounds();
}

gfx::Size BililiveOBSViddupView::GetPreferredSize()
{
    static float preview_scale = 16 * 1.0f / 9;
    gfx::Size title_size = ((views::View*)titlebar_view_)->GetPreferredSize();
    gfx::Size top_toolbar_size = ((views::View*)top_toolbar_view_)->GetPreferredSize();
    gfx::Size tool_size = ((views::View*)toolbar_view_)->GetPreferredSize();
    gfx::Size tab_size = ((views::View*)tabarea_dummy_view_)->GetPreferredSize();
    //gfx::Size status_size = ((views::View*)status_bar_view_)->GetPreferredSize();
    int preview_cx = tool_size.width();
    preview_cx = std::max(preview_cx, title_size.width() - tab_size.width());

    // In order to minimize the size of the preview area does not appear grey invalid render area
    int mod = preview_cx % 16;
    preview_cx += mod == 0 ? 0 : 16 - mod;
    int preview_cy = preview_cx / preview_scale;
    int cx = preview_cx + tab_size.width();

    if (danmaku_dummy_view_->visible())
    {
        cx += dmkhime::kDanmakuMainViewWidth;
    }

    int w = cx + kTabareaSeparatorLineThickness * 4;
    int h = title_size.height() + top_toolbar_size.height() + preview_cy + tool_size.height() /*+ status_size.height()*/ + kTabareaSeparatorLineThickness * 4;
	  int minH = GetLengthByDPIScale(700);
    if (h < minH) // This is the minimum height
    {
        h = minH;
    }

    return gfx::Size(w,h);
}

views::View* BililiveOBSViddupView::GetEventHandlerForPoint(const gfx::Point& point)
{
    if (GetWidget() && GetWidget()->IsMaximized()) {
        return __super::GetEventHandlerForPoint(point);
    }

    // If the current side panel is retracted, the upper right hand operation returns the oblique stretch
    gfx::Rect rtLocal = GetLocalBounds();
    gfx::Rect rtClt = rtLocal;
    rtClt.Inset(kHitTestThickness, kHitTestThickness, kHitTestThickness, kHitTestThickness);
    SkPath path;
    path.addRect(SkRect::MakeLTRB(rtLocal.x(), rtLocal.y(), rtLocal.right(), rtLocal.bottom()));
    SkPath pathClt;
    pathClt.addRect(SkRect::MakeLTRB(rtClt.x(), rtClt.y(), rtClt.right(), rtClt.bottom()));
    path.reverseAddPath(pathClt);
    if (path.contains(point.x(), point.y()))
    {
        return this;
    }

    return __super::GetEventHandlerForPoint(point);
}

void BililiveOBSViddupView::OnThemeChanged()
{
    //__super::OnThemeChanged();
    set_background(views::Background::CreateSolidBackground(GetColor(Theme)));
}

void BililiveOBSViddupView::OnPaintBackground(gfx::Canvas* canvas)
{
    //__super::OnPaintBackground(canvas);

    //if(main_content_view_->visible())
    {
        gfx::Size title_size = ((views::View*)titlebar_view_)->GetPreferredSize();
        gfx::Rect top_rect(0, 0, width(), title_size.height());

		    SkPaint paint;
		    paint.setAntiAlias(true);
		    paint.setColor(GetColor(Theme));
		    paint.setStyle(SkPaint::kFill_Style);
		    canvas->DrawRoundRect(GetContentsBounds(), 0, paint);
    }
}

void BililiveOBSViddupView::ChildPreferredSizeChanged(View* child)
{

}

bool BililiveOBSViddupView::GetDropFormats(
    int *formats,
    std::set<ui::OSExchangeData::CustomFormat> *custom_formats)
{
    return false;
}

bool BililiveOBSViddupView::AreDropTypesRequired()
{
    return true;
}

bool BililiveOBSViddupView::CanDrop(const ui::OSExchangeData &data)
{
    if (!IsWindowEnabled(GetWidget()->GetNativeView()))
        return false;

    base::ThreadRestrictions::ScopedAllowIO io_allow;
    std::vector<ui::OSExchangeData::FileInfo> dragged_files;

    if (data.GetFilenames(&dragged_files))
    {
        if (dragged_files.size() != 1)
        {
            return false;
        }

        if (base::PathExists(dragged_files[0].path) &&
            !base::DirectoryExists(dragged_files[0].path) &&
            IsFileTypeDroppable(dragged_files[0].path))
        {
            return true;
        }

        return false;
    }

    base::string16 dragged_text;
    return data.HasString() && data.GetString(&dragged_text) && dragged_text.length() > 0;
}

void BililiveOBSViddupView::OnDragEntered(const ui::DropTargetEvent &event)
{
}

int BililiveOBSViddupView::OnDragUpdated(const ui::DropTargetEvent &event)
{
    if (preview_view()->bounds().Contains(event.location()))
    {
        return ui::DragDropTypes::DRAG_COPY;
    }

    return ui::DragDropTypes::DRAG_NONE;
}

void BililiveOBSViddupView::OnDragExited()
{
}

int BililiveOBSViddupView::OnPerformDrop(const ui::DropTargetEvent &event)
{
    base::ThreadRestrictions::ScopedAllowIO io_allow;

    gfx::Point pt = event.location();
    gfx::Size preview_size = gfx::Size();
    if (preview_view())
    {
        ConvertPointFromWidget(preview_view(), &pt);
        preview_size = preview_view()->size();
    }
    else
    {
        pt = gfx::Point();
    }

    base::FilePath dropped_file;
    base::string16 dragged_text;

    if (event.data().GetFilename(&dropped_file))
    {
        bililive::SourceType target_source_type;
        if (base::PathExists(dropped_file) &&
            !base::DirectoryExists(dropped_file) &&
            bililive::GuessSourceType(dropped_file, target_source_type))
        {
            bililive::DropFileData data(dropped_file);
            data.silence_mode = true;
            bililive::CreatingSourceParams params(target_source_type, &data);
            bililive::ExecuteCommandWithParams(
                GetBililiveProcess()->bililive_obs(),
                IDC_LIVEHIME_ADD_SOURCE,
                CommandParams<bililive::CreatingSourceParams>(&params));
        }
    }
    else if (event.data().HasString()
        && event.data().GetString(&dragged_text)
        && dragged_text.length() > 0)
    {
        bililive::DropTextData data(dragged_text);
        data.silence_mode = true;
        bililive::CreatingSourceParams params(bililive::SourceType::Text, &data);
        bililive::ExecuteCommandWithParams(
            GetBililiveProcess()->bililive_obs(),
            IDC_LIVEHIME_ADD_SOURCE,
            CommandParams<bililive::CreatingSourceParams>(&params));
    }

    return ui::DragDropTypes::DRAG_COPY;
}

void BililiveOBSViddupView::OnWidgetVisibilityChanged(views::Widget* widget, bool visible)
{
    UpdateMenu();
}

void BililiveOBSViddupView::OnWidgetDestroying(views::Widget* widget)
{
    UninitViews();
}

void BililiveOBSViddupView::NewDanmaku(const DanmakuInfo& danmaku)
{
    switch (danmaku.dt)
    {
    case DT_CUT_OFF:
    case DT_WARNING:
    {
        if (danmaku.category == 1)
        {
            OnLiveVerifyNotice(danmaku);
        }
        else
        {
            if (danmaku.room_id == GetBililiveProcess()->secret_core()->user_info().room_id())
            {
                base::string16 msg = base::UTF8ToUTF16(danmaku.msg);
                if (msg.empty() && (DT_CUT_OFF == danmaku.dt))
                {
                    NOTREACHED() << "why server's reason empty";
                    msg = GetLocalizedString(IDS_LIVEROOM_CUT_MESSAGE);
                }

                base::MessageLoop::current()->
                    PostTask(FROM_HERE, base::Bind(ShowWarningMessage, msg));

                if (DT_CUT_OFF == danmaku.dt)
                {
                    LOG(INFO) << "live room been cut off.";
                    if (LivehimeLiveRoomController::IsInTheThirdPartyStreamingMode())
                    {
                        bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(),
                            IDC_LIVEHIME_LEAVE_THIRD_PARTY_STREAM_MODE);
                    }
                }

                // Audit side closure planting site
                base::StringPairs event_ext;
                event_ext.push_back(std::pair<std::string, std::string>("close_reason", "3"));
                livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::CloseLiveTracker, event_ext);
            }
        }
        break;
    }
    default:
        break;
    }
}

void BililiveOBSViddupView::OnLiveLayoutModelChanged(bool user_invoke)
{
    if (faux_view_)
    {
        faux_view_->SetOrientation(LiveModelController::GetInstance()->IsLandscapeModel());
    }
}

void BililiveOBSViddupView::OnLiveRoomPreviewStatusChanged(bool show, bool landscape)
{
    if (faux_view_)
    {
        faux_view_->SetVisible(show);
        preview_view_->SetVisible(!show);
        faux_view_->SetOrientation(landscape);
    }
}

void BililiveOBSViddupView::EnterMainNoviceGuideView()
{
	  PrefService* pref = GetBililiveProcess()->global_profile()->GetPrefs();
	  bool novice_guide_show = false;
	  if (pref) {
		novice_guide_show = pref->GetBoolean(prefs::kMainViewNoviceGuide);
		if (novice_guide_show) {
			pref->SetBoolean(prefs::kMainViewNoviceGuide, false);

			std::string extend_str = run_source_ + ";";
			extend_str += activity_id_;
			extend_str += "is_certify:1";
			LOG(INFO) << "extend_str:, \n\t" << extend_str;
			livehime::PolarisEventReport(
				secret::LivehimePolarisBehaviorEvent::MainNoviceGuide, extend_str);
		}
		else {
			
		}
	}
}

void BililiveOBSViddupView::PreEnterClientView()
{
    if (request_get_live_record_total_time_finished_){
        EnterClientView();
    }
}

void BililiveOBSViddupView::ProcPluggableCmd()
{
    LOG(INFO) << "novice_guide_finish_ :" << novice_guide_finish_ << " module_entrance_finish_ = " << module_entrance_finish_;
    //LOG(INFO) << __FUNCTION__ << "thread_id = " << std::this_thread::get_id();
    int activity_id = 0;
    int64_t virtual_id = 0;
    int64_t game_id = 0;
    std::string resource_id;
    std::string source_event;

    while (!pluggable_info_queue_.empty()) {
        auto itr = pluggable_info_queue_.front();
        pluggable_info_queue_.pop();
        if (first_start_) {
            first_start_ = false;
            PluggableController::GetInstance()->ScheduleCurrentProcessCmdline();
            std::wstring parameter = PluggableController::GetInstance()->GetParameterCmd();
            if (!parameter.empty()) {
                if (toolbar_view_) {
                    toolbar_view_->SetSourceEvent(2);
                    base::StringPairs string_params;
                    base::SplitStringIntoKeyValuePairs(itr.params, '=', '&', &string_params);
                    for (size_t i = 0; i < string_params.size(); ++i) {
                        const std::pair<std::string, std::string>& pair = string_params[i];
                        if (pair.first == "resource_id") {
                            int64_t resource_id;
                            if (base::StringToInt64(pair.second.c_str(), &resource_id)) {
                                toolbar_view_->SetResourceId(resource_id);
                                activity_id = resource_id;
                            }
                        }
                        else if (pair.first == "game_id") {
                            if (base::StringToInt64(pair.second.c_str(), &game_id)) {
                                toolbar_view_->SetGameId(game_id);
                            }
                        }
                        else if (pair.first == "virtual_id") {
                            if (base::StringToInt64(pair.second.c_str(), &virtual_id)) {
                                toolbar_view_->SetVirtualId(virtual_id);
                            }
                        }
                    }
                }
            }
        }

        source_event = "source_event:2";
        run_source_ = source_event;
        resource_id = "resource_id:" + std::to_string(activity_id);
        activity_id_ = resource_id;

        switch (itr.type)
        {
        case PluggableType::OpenPreviewDebug: {
            std::wstring  text = L"Debug preview";
            std::string report_str = "function_type:" + UTF16ToUTF8(text);
            web_link_open_report_ = report_str;
        }
        break;
        default:
            break;
        }
        PvEnventReport();
    }

}

void BililiveOBSViddupView::PostUiAddPluggable(const PluggableInfo& info)
{
    //LOG(INFO) << __FUNCTION__ << "thread_id = " << std::this_thread::get_id();
    pluggable_info_queue_.push(info);
    if (novice_guide_finish_&& module_entrance_finish_) {
        ProcPluggableCmd();
    }
}

void BililiveOBSViddupView::PvEnventReport()
{
    static bool inited = true;
    if (inited)
    {
        inited = false;
        std::string extend_str = run_source_ + ";";
        extend_str += activity_id_;
        if (!web_link_open_report_.empty()) {
            extend_str += ";" + web_link_open_report_;
            web_link_open_report_ = "";
        }
#if defined(_WIN64)
                extend_str += ";processor:64;";
#else
                extend_str += ";processor:32;";
#endif // defined(_WIN64)
        LOG(INFO) << "extend_str:, \n\t" << extend_str;
        livehime::PolarisEventReport(
            secret::LivehimePolarisBehaviorEvent::MainWndPV, extend_str);
    }
}

void BililiveOBSViddupView::EnterClientView()
{    
    client_view_->SetVisible(true);
    titlebar_view_->SetBusinessFunctionButtonVisible(true);
    set_background(views::Background::CreateSolidBackground(GetColor(Theme)));
}

void BililiveOBSViddupView::PostUiNoviceGuideFinish()
{
	  base::NotificationService::current()->Notify(
        bililive::NOTIFICATION_BILILIVE_DOGUIDE_CFG_CHECKED,
        base::NotificationService::AllSources(),
        base::NotificationService::NoDetails());
    
    novice_guide_finish_ = true;
    if (module_entrance_finish_) {
        ProcPluggableCmd();
    }
}

void BililiveOBSViddupView::OnLiveVerifyNotice(const DanmakuInfo& danmaku) {

    if (danmaku.pop_button_url.empty()) {
        livehime::ShowMessageBox(GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeView(),
            base::UTF8ToUTF16(danmaku.pop_title),
            base::UTF8ToUTF16(danmaku.pop_msg),
            GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_IKNOW));
    } else {
        livehime::MessageBoxEndDialogSignalHandler handler;
        handler.closure = base::Bind(&OnVerifyFailDialog);
        scoped_ptr<std::wstring> button_url(new std::wstring(base::UTF8ToUTF16(danmaku.pop_button_url)));
        handler.data = button_url.release();
        livehime::ShowMessageBox(
            GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeView(),
            base::UTF8ToUTF16(danmaku.pop_title),
            base::UTF8ToUTF16(danmaku.pop_msg),
            GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_IKNOW) + L"," + base::UTF8ToUTF16(danmaku.pop_button_text),
            &handler, livehime::MessageBoxType_NONEICON, base::UTF8ToUTF16(danmaku.pop_button_text));
    }
}

void BililiveOBSViddupView::ShowVideoPkByThirdPartyTips(const std::string& msg) {
    if (msg.empty()) {
        return;
    }

    livehime::ShowMessageBox(
        GetWidget()->GetNativeView(),
        GetLocalizedString(IDS_MELEE_MSGBOX_NOTIFICATION),
        base::UTF8ToUTF16(msg),
        GetLocalizedString(IDS_SURE));
}

void BililiveOBSViddupView::ShowGoldPkMatchingView() {
}

void BililiveOBSViddupView::HandlePresetMaterial()
{
    PrefService* pref = GetBililiveProcess()->global_profile()->GetPrefs();
	  if (pref)
    {
        bool already_show = pref->GetBoolean(prefs::kPresetMaterialGuide);
        if (already_show)
        {
            request_get_live_record_total_time_finished_ = true;
            BililiveThread::PostDelayedTask(BililiveThread::UI,
                FROM_HERE,
                base::Bind(&BililiveOBSViddupView::PreEnterClientView,
                    weakptr_factory_.GetWeakPtr()),
                base::TimeDelta::FromMilliseconds(1500));
            return;
        }
    }
    
}

void BililiveOBSViddupView::OnRequestGetLiveRecordTotalTimeRes(bool success, int64_t total_time)
{
    request_get_live_record_total_time_finished_ = true;
    bool is_new_anchor = false;
    // 180 days of less than 30 minutes for new anchors
    if (success && total_time < 30 * 60)
    {
        is_new_anchor = true;
    }

    PreEnterClientView();
}

void BililiveOBSViddupView::ProcessingPresetMaterialMsg(PresetMaterial::PresetMaterialMsgInfo msgInfo)
{
}