#include "live_handon_notify_window.h"

#include <vector>
#include "base/strings/utf_string_conversions.h"
#include "base/ext/callable_callback.h"

#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/view.h"
#include "ui/gfx/screen.h"
#include "ui/gfx/display.h"

#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/secret/core/bililive_secret_core_impl.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/secret/services/live_streaming_service_impl.h"


LiveHandonNotifyWindow* g_live_handon_window = nullptr;
std::vector<LiveHandonNotifyWindow*> all_window;
std::vector<gfx::Rect*> all_monitor_rect;
std::vector<gfx::Rect*> all_monitor_rects;
static bool is_show_window = false;

static BOOL CALLBACK EnumAllMonitor(HMONITOR monitor, HDC hdc, LPRECT rc, LPARAM data) {
    //保存显示器信息
    MONITORINFO monitorinfo;
    monitorinfo.cbSize = sizeof(MONITORINFO);
    ::GetMonitorInfo(monitor, &monitorinfo);
    RECT recte1;
    gfx::Rect* rect2 = new gfx::Rect();
    recte1 = monitorinfo.rcMonitor;
    rect2->set_x(recte1.left);
    rect2->set_y(recte1.top);
    rect2->set_width(recte1.right - recte1.left);
    rect2->set_height(recte1.bottom - recte1.top);
    all_monitor_rects.push_back(rect2);
    return true;
}

static gfx::Rect GetAllMonitorUnionRect() {
    gfx::Rect rect;
    bool success = !!::EnumDisplayMonitors(NULL, NULL, EnumAllMonitor, (LPARAM)&rect);
    DCHECK(success);
    return rect;
}

static void CloseAllWindow() {
    int window_count = all_window.size();
    if (window_count > 0) {
        for (int i = 0; i < window_count; i++) {
            all_window[i]->GetWidget()->Close();
        }
        all_window.clear();
    }
    is_show_window = false;
}

LiveHandonNotifyWindow::LiveHandonNotifyWindow()
    :title_label_(nullptr),
    tips_label_(nullptr),
    confirm_button_(nullptr),
    time_label_(nullptr),
    weak_ptr_factory_(this){

}

LiveHandonNotifyWindow::~LiveHandonNotifyWindow() {
    g_live_handon_window = nullptr;
}

void LiveHandonNotifyWindow::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) {
    if (details.child == this) {
        if (details.is_add) {
            InitViews();
        }
        else {
        }
    }
}

void LiveHandonNotifyWindow::OnPaintBackground(gfx::Canvas* canvas) {
    //背景透明处理
   canvas->FillRect(GetLocalBounds(), SkColorSetA(SK_ColorWHITE, 255));
}

void LiveHandonNotifyWindow::WindowClosing() {
    CloseAllWindow();
}

// ButtonListener
void LiveHandonNotifyWindow::ButtonPressed(views::Button* sender, const ui::Event& event) {
    if (sender == confirm_button_) {
        HttpUpdateNotifyResult(1);
        livehime::PolarisEventReport(
            secret::LivehimePolarisBehaviorEvent::HandonLiveVerificationClick,"");
        CloseAllWindow();
    }
}

void LiveHandonNotifyWindow::InitViews() {
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(16));
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, GetLengthByDPIScale(248), 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(16));

    title_label_ = new LivehimeContentLabel(L"");
    title_label_->SetTextColor(SkColorSetRGB(0x21, 0x21, 0x21));
    title_label_->SetFont(ftSixteen);
    title_label_->SetHorizontalAlignment(gfx::ALIGN_CENTER);

    tips_label_ = new LivehimeContentLabel(L" ");
    tips_label_->SetTextColor(SkColorSetRGB(0x50, 0x50, 0x50));
    tips_label_->SetFont(ftThirteen);
    tips_label_->SetHorizontalAlignment(gfx::ALIGN_CENTER);
    tips_label_->SetMultiLine(true);

    confirm_button_ = new LivehimeActionLabelButton(this, GetLocalizedString(IDS_LIVE_ROOM_INFO_EDIT_OK), true);
    confirm_button_->set_focusable(false);
    confirm_button_->SetIsDefault(true);
    confirm_button_->SetPreferredSize(gfx::Size(GetLengthByDPIScale(248), GetLengthByDPIScale(34)));

    time_label_ = new LivehimeContentLabel(L" ");
    time_label_->SetTextColor(SkColorSetRGB(0x99, 0x99, 0x99));
    time_label_->SetFont(ftFourteen);
    time_label_->SetHorizontalAlignment(gfx::ALIGN_CENTER);

    layout->AddPaddingRow(0, GetLengthByDPIScale(23));
    layout->StartRow(0, 0);
    layout->AddView(title_label_);
    layout->AddPaddingRow(0, kPaddingRowHeightForCtrlTips);
    layout->StartRow(0, 0);
    layout->AddView(tips_label_);
    layout->AddPaddingRow(0, GetLengthByDPIScale(14));
    layout->StartRow(0, 0);
    layout->AddView(confirm_button_);
    layout->AddPaddingRow(0, kPaddingRowHeightForCtrlTips);
    layout->StartRow(0, 0);
    layout->AddView(time_label_);
    layout->AddPaddingRow(0, GetLengthByDPIScale(12));
}

void LiveHandonNotifyWindow::UninitViews() {
    //GetBililiveProcess()->bililive_obs()->broadcast_service()->RemoveObserver(this);
}

void LiveHandonNotifyWindow::OnCountDownTimer() {
    if (countdown_ == 0) {
        timer_.Stop();
        HttpUpdateNotifyResult(2);
        GetWidget()->Close();

        //auto main_view = GetBililiveProcess()->bililive_obs()->obs_view();
        //main_view->StopLiveStream();
        is_show_window = false;
        return;
    }
    --countdown_;
    SetTimeLabStr(countdown_);
}

void LiveHandonNotifyWindow::SetTimeLabStr(int count) {
    int time_count = count;
    if (time_count > 0) {
        int minute = time_count / 60;
        int sec = time_count % 60;

        char time_str[64] = { 0 };
        sprintf_s(time_str, "%02d:%02d", minute, sec);
        time_label_->SetText(base::UTF8ToWide(time_str));
    }
}

void LiveHandonNotifyWindow::HttpUpdateNotifyResult(int flag) {

}

void LiveHandonNotifyWindow::OnUpdateNotifyResult(bool succ, int code) {
    if (succ) {
        bool succc = succ;
    }
}
