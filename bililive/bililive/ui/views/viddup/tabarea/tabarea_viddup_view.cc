#include "bililive/bililive/ui/views/viddup/tabarea/tabarea_viddup_view.h"

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
//#include "bililive/bililive/ui/views/tabarea/tabarea_scenes_view.h"
#include "bililive/bililive/ui/views/tabarea/tabarea_danmaku_view.h"
#include "bililive/bililive/ui/views/viddup/tabarea/tabarea_scenes_viddup_view.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/public/event_tracking_service.h"

namespace
{
    // Portrait new feature introduction
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

TabAreaViddupView::TabAreaViddupView()
    : weakptr_factory_(this)
{
}

TabAreaViddupView::~TabAreaViddupView()
{
}

void TabAreaViddupView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            static bool s_view_added = false;
            if (s_view_added)
            {
                return;
            }

            s_view_added = true;

            InitViews();

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

void TabAreaViddupView::InitViews()
{
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

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

        layout->AddPaddingRow(0, GetLengthByDPIScale(4));
        layout->StartRow(0, 0);
        layout->AddView(model_label_);
        layout->AddPaddingRow(0, GetLengthByDPIScale(4));
    }

    layout->StartRow(0, 0);
    layout->AddView(mode_view_);
    layout->AddPaddingRow(0, GetLengthByDPIScale(1));
    scenes_view_ = new TabAreaScenesViddupView();

    layout->StartRow(1.0f, 1);
    layout->AddView(scenes_view_);
}

gfx::Size TabAreaViddupView::GetPreferredSize()
{
    gfx::Size size = views::View::GetPreferredSize();
    size.set_width(std::max(size.width(), kMainWndTabAreaWidth));
    return size;
}

void TabAreaViddupView::OpenChatRoomView()
{
    scenes_view_->OpenChatRoomView();
}

void TabAreaViddupView::CloseChatRoomView()
{
    scenes_view_->CloseChatRoomView();
}

void TabAreaViddupView::OnPaintBackground(gfx::Canvas* canvas)
{
    // canvas->FillRect(GetContentsBounds(), GetColor(WindowClient));
    int radus = GetLengthByDPIScale(8);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(GetColor(WindowClient));
    paint.setStyle(SkPaint::kFill_Style);
    
    auto bounds = mode_view_->bounds();
    SkRRect rrect;
    SkVector sk_vector[]{ SkVector::Make(radus,radus),SkVector::Make(radus,radus), SkVector::Make(0,0),SkVector::Make(0,0) };
    rrect.setRectRadii(SkRect::MakeXYWH(bounds.x(), bounds.y(), bounds.width(), bounds.height()), sk_vector);
    canvas->sk_canvas()->drawRRect(rrect, paint);
}

void TabAreaViddupView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    LiveModelController::GetInstance()->ChangeLayoutModel(
        LiveModelController::GetInstance()->IsLandscapeModel() ?
        LiveModelController::ModelType::Portrait : LiveModelController::ModelType::Landscape);

    if (model_button_guide_bubble_ && model_button_guide_bubble_->GetWidget())
    {
        model_button_guide_bubble_->GetWidget()->Close();
        model_button_guide_bubble_ = nullptr;
    }
}

void TabAreaViddupView::OnWidgetClosing(views::Widget* widget)
{
    model_button_guide_bubble_ = nullptr;
}

void TabAreaViddupView::Observe(int type, const base::NotificationSource& source, const base::NotificationDetails& details)
{
}

void TabAreaViddupView::ShowModelButtonGuideBubble()
{
}

void TabAreaViddupView::OnEnterIntoThirdPartyStreamingMode()
{
}

void TabAreaViddupView::OnLeaveThirdPartyStreamingMode()
{
}

void TabAreaViddupView::OnLiveLayoutModelChanged(bool user_invoke)
{
    bool landscape = LiveModelController::GetInstance()->IsLandscapeModel();
    model_label_->SetText(GetLocalizedString(
        landscape ? IDS_TABAREA_MODEL_TYPE_H : IDS_TABAREA_MODEL_TYPE_V));

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

gfx::Rect TabAreaViddupView::GetMiddleAreaBounds() const
{
    gfx::Rect rect = scenes_view_->GetMiddleAreaBounds();
    rect = scenes_view_->ConvertRectToParent(rect);
    return rect;
}

gfx::Rect TabAreaViddupView::GetCameraItemEditBtnBounds(const std::string& name) const
{
    if (!scenes_view_)
        return gfx::Rect();

    return scenes_view_->GetCameraItemEditBtnBounds(name);
}

void TabAreaViddupView::ShowOperateView(const std::string& name, bool show)
{
    if (!scenes_view_)
        return;

    scenes_view_->ShowOperateView(name, show);
}