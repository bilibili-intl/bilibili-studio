#include "tabarea_view.h"
#include "tabarea_danmaku_view.h"

#include "SkRRect.h"

#include "base/notification/notification_service.h"
#include "base/prefs/pref_service.h"
#include "base/strings/stringprintf.h"

#include "ui/base/win/dpi.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/fill_layout.h"

#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/bililive/livehime/notify_ui_control/notify_ui_controller.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_bubble.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/livehime/new_live_room/new_live_room_guide_view.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/public/event_tracking_service.h"

namespace
{
    // 竖屏新功能引导
    class PortraitModelGuideView :
        public views::View,
        views::ButtonListener
    {
    public:
        PortraitModelGuideView()
        {
            views::GridLayout* layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            static const int kLabelCX = GetLengthByDPIScale(170);

            auto col = layout->AddColumnSet(0);
            col->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
            col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED,
                kLabelCX, kLabelCX);
            col->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);

            col = layout->AddColumnSet(1);
            col->AddPaddingColumn(1.0f, kPaddingColWidthForGroupCtrls);
            col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            col->AddPaddingColumn(1.0f, kPaddingColWidthForGroupCtrls);

            BililiveLabel* label = new LivehimeContentLabel(GetLocalizedString(IDS_TABAREA_MODEL_BUTTON_GUIDE));
            label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
            label->SetMultiLine(true);
            label->SetAllowCharacterBreak(true);
            label->SetTextColor(SK_ColorWHITE);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
            layout->AddView(label);

            LivehimeTitlebarButton* close_btn = new LivehimeTitlebarButton(this, GetLocalizedString(IDS_IKNOW));

            layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
            layout->AddView(close_btn);

            layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
        }

        ~PortraitModelGuideView() = default;

    protected:
        // ButtonListener
        void ButtonPressed(views::Button* sender, const ui::Event& event) override
        {
            GetWidget()->Close();
        }
    };
}


TabAreaView::TabAreaView()
    : weakptr_factory_(this)
{
}

TabAreaView::~TabAreaView()
{
}

void TabAreaView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            static bool s_view_added = false;
            if (s_view_added)
            {
                return;////为了实现左侧工具栏弹出，会多次触发AddChildView()，所以只能添加一次
            }
            s_view_added = true;

            InitViews();

            notifation_register_.Add(this,
                bililive::NOTIFICATION_BILILIVE_DOGUIDE_CFG_CHECKED,
                base::NotificationService::AllSources());
            notifation_register_.Add(this,
                bililive::NOTIFICATION_BILILIVE_DOGUIDE,
                base::NotificationService::AllSources());

            LivehimeLiveRoomController::GetInstance()->AddObserver(this);
            LiveModelController::GetInstance()->AddObserver(this);
        }
        else
        {
            notifation_register_.RemoveAll();
            if (model_button_guide_bubble_ && model_button_guide_bubble_->GetWidget())
            {
                model_button_guide_bubble_->GetWidget()->RemoveObserver(this);
            }
            LivehimeLiveRoomController::GetInstance()->RemoveObserver(this);
            if (LiveModelController::GetInstance())
            {
                LiveModelController::GetInstance()->RemoveObserver(this);
            }
        }
    }
}

void TabAreaView::InitViews()
{
    /*set_background(views::Background::CreateSolidBackground(GetColor(WindowClient)));*/
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    /*auto col = layout->AddColumnSet(0);
    col->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    col->AddPaddingColumn(1.0f, 0);
    col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    col->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    col->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);*/

    auto col = layout->AddColumnSet(0);
    col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);

    col = layout->AddColumnSet(1);
    col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

    mode_view_ = new views::View();
    {
        views::GridLayout* layout = new views::GridLayout(mode_view_);
        mode_view_->SetLayoutManager(layout);

        mode_view_->SetSize(gfx::Size(GetLengthByDPIScale(255), GetLengthByDPIScale(40)));
        auto col = layout->AddColumnSet(0);
        col->AddPaddingColumn(0, GetLengthByDPIScale(10));
        col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
        col->AddPaddingColumn(1.0f, 0);
        col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
        //col->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        //col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
        col->AddPaddingColumn(0, GetLengthByDPIScale(10));

        model_label_ = new LivehimeTitleLabel(GetLocalizedString(IDS_TABAREA_MODEL_TYPE_H));
        model_label_->SetTextColor(clrTextALL);
        model_label_->SetFont(ftPrimary);
        model_label_->SetPreferredSize(GetSizeByDPIScale({ 57, 18}));
        /*model_button_ = new BililiveImageButton(this);
        model_button_->SetAllStateImage(GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_MODEL_H));
        model_button_->SetTooltipText(GetLocalizedString(IDS_TABAREA_MODEL_TYPE_V));
        direction_label_ = new LivehimeTitleLabel(GetLocalizedString(IDS_TABAREA_MODEL_TYPE_V_GUIDE));*/

        switch_button_ = new LivehimeSwitchModeButton(this, GetLocalizedString(IDS_TABAREA_MODEL_TYPE_SWITCH_GUIDE));
        switch_button_->SetImage(views::Button::STATE_NORMAL, *GetImageSkiaNamed(IDR_LIVEHIME_V4_H_NORMAL));
        switch_button_->SetImage(views::Button::STATE_HOVERED, *GetImageSkiaNamed(IDR_LIVEHIME_V4_H_HOV));
        switch_button_->set_round_corner(false);

        layout->AddPaddingRow(0, GetLengthByDPIScale(4));
        layout->StartRow(0, 0);
        layout->AddView(model_label_);
        //layout->AddView(model_button_);
        layout->AddView(switch_button_);
        layout->AddPaddingRow(0, GetLengthByDPIScale(4));
    }
    /*model_label_ = new LivehimeTitleLabel(GetLocalizedString(IDS_TABAREA_MODEL_TYPE_H));
    model_button_ = new BililiveImageButton(this);
    model_button_->SetAllStateImage(GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_MODEL_H));
    model_button_->SetTooltipText(GetLocalizedString(IDS_TABAREA_MODEL_TYPE_V));
    direction_label_ = new LivehimeTitleLabel(GetLocalizedString(IDS_TABAREA_MODEL_TYPE_V_GUIDE));

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(model_label_);
    layout->AddView(direction_label_);
    layout->AddView(model_button_);
    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);*/

    layout->StartRow(0, 0);
    layout->AddView(mode_view_);
    layout->AddPaddingRow(0, GetLengthByDPIScale(1));
}

gfx::Size TabAreaView::GetPreferredSize()
{
    gfx::Size size = views::View::GetPreferredSize();
    size.set_width(std::max(size.width(), kMainWndTabAreaWidth));

    return size;
}

void TabAreaView::OpenChatRoomView()
{
    
}

void TabAreaView::CloseChatRoomView()
{
    
}

void TabAreaView::OnPaintBackground(gfx::Canvas* canvas)
{
   // canvas->FillRect(GetContentsBounds(), GetColor(WindowClient));
	int radus = GetLengthByDPIScale(0);
	SkPaint paint;
	paint.setAntiAlias(true);
    paint.setColor(SkColorSetRGB(0xf9, 0xf9, 0xf9));
	paint.setStyle(SkPaint::kFill_Style);

	auto bounds = mode_view_->bounds();
	SkRRect rrect;
	SkVector sk_vector[]{ SkVector::Make(radus,radus),SkVector::Make(radus,radus), SkVector::Make(0,0),SkVector::Make(0,0) };
	rrect.setRectRadii(SkRect::MakeXYWH(bounds.x(), bounds.y(), bounds.width(), bounds.height()), sk_vector);
	canvas->sk_canvas()->drawRRect(rrect, paint);
}

void TabAreaView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    // 只是尝试去变更，具体UI状态变更等切换完成通知再做
    LiveModelController::GetInstance()->ChangeLayoutModel(
        LiveModelController::GetInstance()->IsLandscapeModel() ?
        LiveModelController::ModelType::Portrait : LiveModelController::ModelType::Landscape);

    if (model_button_guide_bubble_ && model_button_guide_bubble_->GetWidget())
    {
        model_button_guide_bubble_->GetWidget()->Close();
        model_button_guide_bubble_ = nullptr;
    }
}

void TabAreaView::OnWidgetClosing(views::Widget* widget)
{
    model_button_guide_bubble_ = nullptr;
}

void TabAreaView::Observe(int type, const base::NotificationSource& source, const base::NotificationDetails& details)
{
    switch (type)
    {
    case bililive::NOTIFICATION_BILILIVE_DOGUIDE_CFG_CHECKED:
    case bililive::NOTIFICATION_BILILIVE_DOGUIDE:
        {
            base::MessageLoop::current()->PostTask(FROM_HERE,
                base::Bind(&TabAreaView::ShowModelButtonGuideBubble, weakptr_factory_.GetWeakPtr()));
        }
        break;
    default:
        break;
    }
}

void TabAreaView::ShowModelButtonGuideBubble()
{

}

void TabAreaView::OnEnterIntoThirdPartyStreamingMode()
{
    switch_button_->SetEnabled(false);
}

void TabAreaView::OnLeaveThirdPartyStreamingMode()
{
    switch_button_->SetEnabled(true);
}

void TabAreaView::OnLiveLayoutModelChanged(bool user_invoke)
{
    bool landscape = LiveModelController::GetInstance()->IsLandscapeModel();
    model_label_->SetText(GetLocalizedString(
        landscape ? IDS_TABAREA_MODEL_TYPE_H : IDS_TABAREA_MODEL_TYPE_V));
    //model_button_->SetAllStateImage(GetImageSkiaNamed(
    //    landscape ? IDR_LIVEHIME_V3_TABAREA_MODEL_H : IDR_LIVEHIME_V3_TABAREA_MODEL_V));
    //model_button_->SetTooltipText(GetLocalizedString(
    //    landscape ? IDS_TABAREA_MODEL_TYPE_V : IDS_TABAREA_MODEL_TYPE_H));
    if (landscape) {
        switch_button_->SetImage(views::Button::STATE_NORMAL, *GetImageSkiaNamed(IDR_LIVEHIME_V4_H_NORMAL));
        switch_button_->SetImage(views::Button::STATE_HOVERED, *GetImageSkiaNamed(IDR_LIVEHIME_V4_H_HOV));
    }
    else {
        switch_button_->SetImage(views::Button::STATE_NORMAL, *GetImageSkiaNamed(IDR_LIVEHIME_V4_V_NORMAL));
        switch_button_->SetImage(views::Button::STATE_HOVERED, *GetImageSkiaNamed(IDR_LIVEHIME_V4_V_HOV));
    }


   /* direction_label_->SetText(GetLocalizedString(
        landscape ? IDS_TABAREA_MODEL_TYPE_H_GUIDE : IDS_TABAREA_MODEL_TYPE_V_GUIDE));*/

    if (user_invoke)
    {
        livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::LiveModelButtonClick,
            base::StringPrintf("screen_status:%d", landscape ? 1 : 2));
    }

    if (landscape && user_invoke) {
        if (AppFunctionController::GetInstance()->app_function_settings().new_live_room_switch) {
            if (GetBililiveProcess()->secret_core()->anchor_info().get_join_slide() == 0) {
                livehime::NotifyUIController::GetInstance()->AddNotifyTask(
                    livehime::NotifyUILayer::NewLiveRoomStyle,
                    base::Bind(NewLiveRoomGuideWindow::ShowLayerWindow));
            }
        }
    }
}

void TabAreaView::ShowOperateView(const std::string& name, bool show)
{

}