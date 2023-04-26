#include "bililive_user_info_viddup_frame.h"

#include <shellapi.h>

#include "base/prefs/pref_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/ext/callable_callback.h"

#include "ui/base/win/dpi.h"
#include "ui/gfx/screen.h"
#include "ui/views/controls/link.h"


#include "ui/base/resource/resource_bundle.h"

#include "grit/generated_resources.h"

#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/user_info/user_info_service.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/views/controls/bililive_bubble.h"
#include "bililive/bililive/ui/views/controls/bililive_labelbutton.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/bililive_progressbar.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_user_related_ctrls.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/utils/net_util.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/bililive/utils/fast_forward_url_convert.h"

#include "bililive/bililive/utils/fast_forward_url_convert.h"

namespace
{
    const wchar_t kLiveRoomLink[] = L"https://www.bilibili.tv/live";
    const wchar_t kLiveData[] = L"https://studio.bilibili.tv/live/live-data";

    const int kHeadshotSize = 60;

    const SkColor clrValueNormalColor = SkColorSetRGB(0x2c, 0x44, 0x57);

    enum TitleBarUserInfoViddupViewCtrlID
    {
        Button_Avatar,
        Button_NickName,
        Button_Medal,
        Button_Room,
        Button_San,
        Button_Fans,
        Button_Sailors,
        Button_MedalFans,
        Button_LiveData,
        Button_Logout,
    };

    TitleBarUserInfoViddupView* g_single_instance_ = nullptr;

    string16 GetFormatString(int64_t value)
    {
        base::string16 text;
        if (value >= 10000)
        {
            text = base::UTF8ToWide(base::StringPrintf("%0.1lf", double(value) / 10000));
            text.append(L"Íò");
        }
        else
        {
            text = std::to_wstring(value);
        }
        return text;
    }

    string16 GetFormatStringNotRound(int64_t value)
    {
        base::string16 text;
        if (value >= 10000)
        {
            text.append(std::to_wstring(value / 10000));
            int digit = (std::abs(value) % 10000) / 1000;
            if (digit != 0) {
                text.push_back(L'.');
                text.append(std::to_wstring(digit));
            }
            text.append(L"Íò");
        }
        else
        {
            text = std::to_wstring(value);
        }
        return text;
    }

    const std::map<TitleBarUserInfoViddupViewCtrlID, secret::LivehimeBehaviorEvent> kEventTable
    {
        { Button_Avatar, secret::LivehimeBehaviorEvent::LivehimeHoverMyRoom },
        { Button_Room, secret::LivehimeBehaviorEvent::LivehimeHoverMyRoom1 },
        { Button_LiveData, secret::LivehimeBehaviorEvent::LivehimeHoverLiveData },
    };

    void EventTracking(secret::LivehimeBehaviorEvent event_id) {
        auto secret_core = GetBililiveProcess()->secret_core();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
            event_id, secret_core->account_info().mid(), std::string()).Call();
    }

    void OnCleanCleanRedAlarm(bool valid_response, int code, const std::string& err_msg,
        int ret)
    {
        if (!valid_response)
        {
            return;
        }

        if (code == secret::LiveStreamingService::OK)
        {
            if (ret == 1)
            {
                if (GetBililiveProcess() && GetBililiveProcess()->secret_core())
                {
                    GetBililiveProcess()->secret_core()->anchor_info().set_captain_warn_status(0);
                }
            }
        }
    }

    void CleanCleanRedAlarm()
    {
        if (GetBililiveProcess() && GetBililiveProcess()->secret_core() &&
            GetBililiveProcess()->secret_core()->live_streaming_service())
        {
           
        }
    }



    class MedalView :
        public views::CustomButton,
        public livehime::SupportsEventReportV2
    {
    public:
        MedalView(views::ButtonListener* listener, const string16& text)
            : views::CustomButton(listener)
            , text_(text)
        {
        }

        void Update(int64_t medal_status, const std::string& medal_name)
        {
            if (medal_status == 2)
            {
                image_->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_TITLEBAR_MEDAL));
            }
            else
            {
                image_->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_TITLEBAR_DEFAULT_MEDAL));
            }

            if (medal_name.empty())
            {
                label_->SetText(L"medal");
                label_->SetTextColor(SkColorSetRGB(0x99, 0x99, 0x99));
            }
            else
            {
                label_->SetText(base::UTF8ToUTF16(medal_name));
                label_->SetTextColor(SkColorSetRGB(0xF5, 0x3C, 0x80));
            }

            medal_status_ = medal_status;

            InvalidateLayout();
            Layout();
        }

    protected:
        // button
        void NotifyClick(const ui::Event& event) override
        {
            ReportBehaviorEvent();
            __super::NotifyClick(event);
        }

    private:
        void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override
        {
            if (details.child == this)
            {
                if (details.is_add)
                {
                    InitViews();
                }
            }
        }

        void OnPaint(gfx::Canvas* canvas) override
        {
            __super::OnPaint(canvas);

            gfx::Rect rect = GetContentsBounds();
            auto img_size = GetImageSkiaNamed(IDR_LIVEHIME_V3_TITLEBAR_MEDAL)->size();

            rect.Inset(img_size.width() / 2, 0, 0, 0);

            SkColor clr = (medal_status_ == 2 ? SkColorSetRGB(0xFF, 0xDA, 0xDA) : SkColorSetRGB(0xE7, 0xE7, 0xE7));
            scoped_ptr<views::Painter> painter(views::Painter::CreateVerticalGradient(clr, clr));

            static SkScalar radius = GetLengthByDPIScale(4);
            SkPath path;
            SkRect sk_rect = SkRect::MakeXYWH(rect.x(), rect.y(), rect.width(), rect.height());
            SkScalar sk_radius[] = { 0, 0, radius, radius, radius, radius, radius, radius };

            path.addRoundRect(sk_rect, sk_radius);
            canvas->sk_canvas()->clipPath(path, SkRegion::kIntersect_Op, true);

            views::Painter::PaintPainterAt(canvas, painter.get(), rect);
        }

        gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override
        {
            static HCURSOR cursor = ::LoadCursor(NULL, IDC_HAND);
            return cursor;
        }

        void InitViews()
        {
            views::GridLayout* layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            views::ColumnSet* column_set = layout->AddColumnSet(0);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);

            image_ = new LivehimeImageView(listener_);
            image_->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_TITLEBAR_DEFAULT_MEDAL));
            image_->SetEnabled(false);

            label_ = new LivehimeContentLabel();
            label_->SetLimitText(4);
            label_->SetFont(ftEleven);
            label_->SetText(text_);

            layout->StartRow(0, 0);
            layout->AddView(image_);
            layout->AddView(label_);
        }

    private:
        LivehimeImageView* image_ = nullptr;
        BililiveLabel* label_ = nullptr;
        int64_t medal_status_ = 0;
        string16 text_;
    };

    class BubbleContentView : public views::View
        , public views::ButtonListener {
    public:
        BubbleContentView(const string16& tips_text, const string16& link_text, const string16& link_url)
            : tips_text_(tips_text)
            , link_text_(link_text)
            , link_url_(link_url) {
        }

    private:
        void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override {
            if (details.child == this) {
                if (details.is_add) {
                    InitViews();
                }
            }
        }

        void ButtonPressed(views::Button* sender, const ui::Event& event) override {
            ShellExecuteW(nullptr, L"open", bililive::FastForwardChangeEnv(link_url_).c_str(), nullptr, nullptr, SW_SHOW);
        }

        void InitViews() {
            views::GridLayout* layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            views::ColumnSet* column_set = layout->AddColumnSet(0);
            column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);

            tips_label_ = new LivehimeContentLabel(tips_text_);
            link_button_ = new LivehimeLinkButton(this, link_text_);
            link_button_->SetTextColor(views::Button::STATE_NORMAL, clrLinkButtonPressed);

            layout->StartRow(0, 0);
            layout->AddView(tips_label_);
            layout->AddView(link_button_);
        }

    private:
        string16 tips_text_;
        string16 link_text_;
        string16 link_url_;
        BililiveLabel* tips_label_ = nullptr;
        BililiveLabelButton* link_button_ = nullptr;
    };

    // Great Nautical loss warning bubble
    class CaptainBubbleView : public views::View
    {
    public:
        CaptainBubbleView(int expired, int will_expired)
            : expired_(expired)
            , will_expired_(will_expired) {
        }

        void UpdateValues(int expired, int will_expired)
        {
            expired_ = expired;
            will_expired_ = will_expired;
            if (expired_tips_label_)
            {
                expired_tips_label_->SetText(base::IntToString16(expired));
            }
            if (willexpired_tips_label_)
            {
                willexpired_tips_label_->SetText(base::IntToString16(will_expired_));
            }
        }

    private:
        void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override {
            if (details.child == this) {
                if (details.is_add) {
                    InitViews();
                }
            }
        }

        bool OnMousePressed(const ui::MouseEvent& event) override
        {
            return true;
        }

        void OnMouseReleased(const ui::MouseEvent& event) override
        {
            CleanCleanRedAlarm();
            livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::GuardWarningClick, "");
        }

        gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override
        {
            static HCURSOR cursor = ::LoadCursor(NULL, IDC_HAND);
            return cursor;
        }

        void InitViews() {
            views::GridLayout* layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            views::ColumnSet* column_set = layout->AddColumnSet(0);
            column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);

            layout->StartRow(0, 0);
            BililiveLabel* tips_label = new LivehimeContentLabel(L"Crew within seven days of expiration");
            tips_label->SetFont(gfx::Font(ftNormal2));
            layout->AddView(tips_label);
            expired_tips_label_ = new LivehimeContentLabel(base::IntToString16(expired_));
            expired_tips_label_->SetFont(gfx::Font(ftNormal2));
            expired_tips_label_->SetTextColor(SkColorSetRGB(0x0e, 0xbe, 0xff));
            layout->AddView(expired_tips_label_);
            tips_label = new LivehimeContentLabel(L"People, about to expire crew");
            tips_label->SetFont(gfx::Font(ftNormal2));
            layout->AddView(tips_label);
            willexpired_tips_label_ = new LivehimeContentLabel(base::IntToString16(will_expired_));
            willexpired_tips_label_->SetFont(gfx::Font(ftNormal2));
            willexpired_tips_label_->SetTextColor(SkColorSetRGB(0x0e, 0xbe, 0xff));
            layout->AddView(willexpired_tips_label_);
            tips_label = new LivehimeContentLabel(L"People");
            layout->AddView(tips_label);
        }

    private:
        int expired_ = 0;
        int will_expired_ = 0;
        BililiveLabel* expired_tips_label_ = nullptr;
        BililiveLabel* willexpired_tips_label_ = nullptr;
    };

    // Big Navigation details page jump button
    class GuardHoverTipButton : public views::ImageButton,
        public views::WidgetObserver
    {
    public:
        GuardHoverTipButton(const string16& tips_text, const string16& link_text, const string16& link_url)
            : ImageButton(nullptr)
            , tips_text_(tips_text)
            , link_text_(link_text)
            , link_url_(link_url)
        {
            gfx::ImageSkia* skia = GetImageSkiaNamed(IDR_LIVEHIME_V3_TITLEBAR_GUARD_TIPS_NORMAL);
            gfx::ImageSkia* skia_hv = GetImageSkiaNamed(IDR_LIVEHIME_V3_TITLEBAR_GUARD_TIPS_HOVER);
            SetImage(views::Button::STATE_NORMAL, skia);
            SetImage(views::Button::STATE_HOVERED, skia_hv);
            SetImage(views::Button::STATE_PRESSED, skia_hv);
            SetImage(views::Button::STATE_DISABLED, skia);
            SetImageAlignment(views::ImageButton::ALIGN_CENTER, views::ImageButton::ALIGN_MIDDLE);
        }
        ~GuardHoverTipButton() {}

    private:
        void StateChanged() override
        {
            if (state() == Button::STATE_HOVERED || state() == Button::STATE_PRESSED)
            {
                if (!bubble_view_)
                {
                    bubble_view_ = new BubbleContentView(tips_text_, link_text_, link_url_);
                    BililiveBubbleView* bubble = livehime::ShowBubble(this, views::BubbleBorder::Arrow::BOTTOM_RIGHT,
                        bubble_view_);
                    bubble->GetWidget()->AddObserver(this);
                    bubble->GetWidget()->Activate();
                    bubble->set_close_on_deactivate(true);
                }
            }
        }

        void OnWidgetDestroying(views::Widget* widget) override
        {
            bubble_view_ = nullptr;
            widget->RemoveObserver(this);
        }

    private:
        string16 tips_text_;
        string16 link_text_;
        string16 link_url_;
        BubbleContentView* bubble_view_ = nullptr;
    };

    // Great Sea loss warning? button
    class CaptainWarnTipButton : public views::ImageButton,
        public views::ButtonListener
    {
    public:
        CaptainWarnTipButton(int expired, int will_expired)
            :ImageButton(this)
            , expired_(expired)
            , will_expired_(will_expired)
        {
            gfx::ImageSkia* skia = GetImageSkiaNamed(IDR_LIVEHIME_V3_TITLEBAR_CAPTAIN_WARNING_TIPS);
            SetImage(views::Button::STATE_NORMAL, skia);
            SetImage(views::Button::STATE_HOVERED, skia);
            SetImage(views::Button::STATE_PRESSED, skia);
            SetImage(views::Button::STATE_DISABLED, skia);
            SetImageAlignment(views::ImageButton::ALIGN_CENTER, views::ImageButton::ALIGN_MIDDLE);
        }
        ~CaptainWarnTipButton() {};

        void ViewHierarchyChanged(const View::ViewHierarchyChangedDetails& details) override
        {
            if (details.child == this && details.is_add)
            {
                BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                    base::Bind(&CaptainWarnTipButton::ShowBubble,
                        base::Unretained(this)));
            }
        }

        void ButtonPressed(views::Button* sender, const ui::Event& event) override
        {
            CleanCleanRedAlarm();
        }

        gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override
        {
            static HCURSOR cursor = ::LoadCursor(NULL, IDC_HAND);
            return cursor;
        }

        void ShowBubble()
        {
            bubble_view_ = new CaptainBubbleView(expired_, will_expired_);

            bubble_ = livehime::ShowBubble(this, views::BubbleBorder::Arrow::BOTTOM_RIGHT,
                bubble_view_);

            bubble_->GetWidget()->Hide();

            GetWidget()->Activate();
        }

        void UpdateValues(int expired, int will_expired)
        {
            expired_ = expired;
            will_expired_ = will_expired;
            if (bubble_view_)
            {
                bubble_view_->UpdateValues(expired, will_expired);
            }

            if (bubble_)
            {
                if (expired_ > 0 || will_expired_ > 0)
                {
                    bubble_->GetWidget()->Show();

                    livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::GuardWarningShow, "");
                }
                else
                {
                    bubble_->GetWidget()->Hide();
                }
                GetWidget()->Activate();
            }
        }

    private:
        int expired_ = 0;
        int will_expired_ = 0;
        CaptainBubbleView* bubble_view_ = nullptr;
        BililiveBubbleView* bubble_ = nullptr;
    };

}

// TitleBarUserInfoViddupView
void TitleBarUserInfoViddupView::ShowForm(views::View* anchor_view, views::View* detect_view)
{
    if (!g_single_instance_)
    {
        BililiveBubbleView* bubble = new BililiveBubbleView(anchor_view,
            views::BubbleBorder::TOP_CENTER,
            new TitleBarUserInfoViddupView(detect_view));
        bubble->set_margins(gfx::Insets());
        bubble->set_background_color(GetColor(WindowTitle));
        bubble->set_close_on_esc(false);
        bubble->set_close_on_deactivate(false);
        bubble->set_move_with_anchor(false);
        bubble->set_adjust_if_offscreen(true);
        bubble->SetRadius(8);
        bubble->SetShadowBorderThickness(0);
        views::Widget* widget = views::BubbleDelegateView::CreateBubble(bubble);
        widget->ShowInactive();
        livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::UserInfoWndShow, "");
    }
}

TitleBarUserInfoViddupView::TitleBarUserInfoViddupView(views::View* relation_view)
    : relation_view_(relation_view)
    , weakptr_factory_(this)
{
    g_single_instance_ = this;
    medal_img_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_TITLEBAR_MEDAL);
    default_medal_img_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_TITLEBAR_DEFAULT_MEDAL);
}

TitleBarUserInfoViddupView::~TitleBarUserInfoViddupView()
{
    StopDoCheckMouse();
}

void TitleBarUserInfoViddupView::ViewHierarchyChanged(const View::ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();
            StartDoCheckMouse();

            base::NotificationService::current()->Notify(
                bililive::NOTIFICATION_LIVEHIME_USERINFO_VIEW_SHOW,
                base::NotificationService::AllSources(),
                base::NotificationService::NoDetails());

            notifation_register_.Add(this,
                bililive::NOTIFICATION_LIVEHIME_UPDATE_AVATAR,
                base::NotificationService::AllSources());

            notifation_register_.Add(this,
                bililive::NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_SUCCESS,
                base::NotificationService::AllSources());

            notifation_register_.Add(this,
                bililive::NOTIFICATION_LIVEHIME_UPDATE_SAN,
                base::NotificationService::AllSources());

            notifation_register_.Add(this,
                bililive::NOTIFICATION_LIVEHIME_UPDATE_CAPTAIN,
                base::NotificationService::AllSources());
        }
        else
        {
            g_single_instance_ = nullptr;

            base::NotificationService::current()->Notify(
                bililive::NOTIFICATION_LIVEHIME_USERINFO_VIEW_CLOSE,
                base::NotificationService::AllSources(),
                base::NotificationService::NoDetails());

            notifation_register_.RemoveAll();
        }
    }
}

void TitleBarUserInfoViddupView::InitViews()
{
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForActionButton);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForActionButton);

    static int label_cx = LivehimePaddingCharWidth(ftPrimary) * 5;

    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED, label_cx, label_cx);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED, label_cx, label_cx);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED, label_cx, label_cx);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED, label_cx, label_cx);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);

    column_set = layout->AddColumnSet(2);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED, label_cx, label_cx);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED, label_cx, label_cx);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED, label_cx, label_cx);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED, label_cx, label_cx);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);

    column_set = layout->AddColumnSet(3);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(40));
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(40));

    column_set = layout->AddColumnSet(4);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);

    avatar_ = new LivehimeUserPhotoExView(this);
    avatar_->SetEnabled(true);
    avatar_->SetPreferredSize(GetSizeByDPIScale(gfx::Size(kHeadshotSize, kHeadshotSize)));
    avatar_->SetIconSize(GetSizeByDPIScale({ 16, 16 }));
    avatar_->set_id(Button_Avatar);
    auto& avatar_data = GetBililiveProcess()->secret_core()->user_info().avatar();
    avatar_->SetAvatar(avatar_data.data(), avatar_data.size());
    auto is_nft = GetBililiveProcess()->secret_core()->user_info().nft();
    avatar_->SetNFT(is_nft);

    // Click the avatar button to bury the point
    //avatar_->SetReportEventDetails(secret::LivehimeBehaviorEventV2::UserInfoItemClick, "button_type:1");

    views::View* right_view = new views::View();
    {
        views::GridLayout* grid_layout = new views::GridLayout(right_view);
        right_view->SetLayoutManager(grid_layout);

        views::ColumnSet* r_col = grid_layout->AddColumnSet(0);
        r_col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
        r_col->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
        r_col->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        /*r_col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);*/
        r_col->AddPaddingColumn(1.0f, 0);

        r_col = grid_layout->AddColumnSet(1);
        r_col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);

        r_col = grid_layout->AddColumnSet(2);
        r_col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
        r_col->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
        r_col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED,
            GetLengthByDPIScale(60), GetLengthByDPIScale(60));
        r_col->AddPaddingColumn(1.0f, 0);

        r_col = grid_layout->AddColumnSet(3);
        r_col->AddColumn(views::GridLayout::LEADING, views::GridLayout::LEADING, 0, views::GridLayout::USE_PREF, 0, 0);
        r_col->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
        r_col->AddColumn(views::GridLayout::FILL, views::GridLayout::LEADING, 1.0f, views::GridLayout::USE_PREF, 0, 0);

        auto nickname = GetBililiveProcess()->secret_core()->user_info().nickname();
        user_name_button_ = new LivehimeLinkButton(this, base::UTF8ToUTF16(nickname));

        user_name_button_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
        user_name_button_->SetFont(ftPrimaryBold);
        user_name_button_->SetTextColor(views::Button::STATE_NORMAL, GetColor(LivehimeColorType::TextTitle));
        user_name_button_->set_id(Button_NickName);
        user_name_button_->SetReportEventDetails(secret::LivehimeBehaviorEventV2::UserInfoItemClick, "button_type:2");

        auto room_id = GetBililiveProcess()->secret_core()->user_info().room_id();
        room_id_label_ = new LivehimeContentLabel(std::to_wstring(room_id));
        room_id_label_->SetTextColor(GetColor(LivehimeColorType::TextContent));
        room_id_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);

        auto uid = GetBililiveProcess()->secret_core()->account_info().mid();
        uid_label_ = new LivehimeContentLabel(std::to_wstring(uid));
        uid_label_->SetTextColor(GetColor(LivehimeColorType::TextContent));
        uid_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);

        grid_layout->AddPaddingRow(1.0f, 0);
        grid_layout->StartRow(0, 0);
        grid_layout->AddView(user_name_button_);
        //grid_layout->AddView(medal_view_);

        auto label = new LivehimeContentLabel(GetLocalizedString(IDS_LIVEHIME_MY_LIVE_ROOM_ID_LABEL));
        label->SetTextColor(GetColor(LivehimeColorType::TextTitle));
        grid_layout->AddPaddingRow(0, kPaddingRowHeightForCtrlTips);
        grid_layout->StartRow(0, 3);
        grid_layout->AddView(label);
        grid_layout->AddView(room_id_label_);

        label = new LivehimeContentLabel(GetLocalizedString(IDS_LIVEHIME_MY_UID_LABEL));
        label->SetTextColor(GetColor(LivehimeColorType::TextTitle));
        grid_layout->AddPaddingRow(0, GetLengthByDPIScale(4));
        grid_layout->StartRow(0, 3);
        grid_layout->AddView(label);
        grid_layout->AddView(uid_label_);
    }

    layout->AddPaddingRow(0, kPaddingRowHeightForDiffGroups);
    layout->StartRow(0, 0);
    layout->AddView(avatar_);
    layout->AddView(right_view);

    live_room_button_ = new LivehimeLinkButton(this, GetLocalizedString(IDS_LIVEHIME_MY_LIVE_ROOM));
    live_room_button_->SetTextColor(views::Button::STATE_NORMAL, GetColor(TextTitle));
    live_room_button_->SetImage(views::Button::STATE_NORMAL, *GetImageSkiaNamed(IDR_LIVEHIME_V3_HOSTCENTER_LIVEROOM));
    live_room_button_->SetImage(views::Button::STATE_HOVERED, *GetImageSkiaNamed(IDR_LIVEHIME_V3_HOSTCENTER_LIVEROOM_HV));
    live_room_button_->SetImage(views::Button::STATE_PRESSED, *GetImageSkiaNamed(IDR_LIVEHIME_V3_HOSTCENTER_LIVEROOM_HV));
    live_room_button_->set_id(Button_Room);
    live_room_button_->SetReportEventDetails(secret::LivehimeBehaviorEventV2::UserInfoItemClick, "button_type:7");

    live_data_button_ = new LivehimeLinkButton(this, GetLocalizedString(IDS_LIVEHIME_LIVE_DATA));
    live_data_button_->SetTextColor(views::Button::STATE_NORMAL, GetColor(TextTitle));
    live_data_button_->SetImage(views::Button::STATE_NORMAL, *GetImageSkiaNamed(IDR_LIVEHIME_V3_HOSTCENTER_LIVEDATA));
    live_data_button_->SetImage(views::Button::STATE_HOVERED, *GetImageSkiaNamed(IDR_LIVEHIME_V3_HOSTCENTER_LIVEDATA_HV));
    live_data_button_->SetImage(views::Button::STATE_PRESSED, *GetImageSkiaNamed(IDR_LIVEHIME_V3_HOSTCENTER_LIVEDATA_HV));
    live_data_button_->set_id(Button_LiveData);
    live_data_button_->SetReportEventDetails(secret::LivehimeBehaviorEventV2::UserInfoItemClick, "button_type:8");

    layout->AddPaddingRow(0, kPaddingRowHeightForDiffGroups + kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 3);
    layout->AddView(live_room_button_);
    layout->AddView(live_data_button_);

    logout_button_ = new LivehimeLinkButton(this, GetLocalizedString(IDS_TITLEBAR_MORE_LOGOUT));
    logout_button_->SetTextColor(views::Button::STATE_NORMAL, GetColor(TextTitle));
    logout_button_->set_id(Button_Logout);
    logout_button_->SetReportEventDetails(secret::LivehimeBehaviorEventV2::UserInfoItemClick, "button_type:9");

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls * 2);
    layout->StartRow(0, 4);
    layout->AddView(logout_button_);
    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);

    GetBililiveProcess()->bililive_obs()->user_info_service()->GetUserInfo();
    GetBililiveProcess()->bililive_obs()->user_info_service()->GetRoomInfo();
}

gfx::Size TitleBarUserInfoViddupView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.SetToMax(GetSizeByDPIScale(gfx::Size(330, 0)));
    return size;
}

void TitleBarUserInfoViddupView::OnPaintBackground(gfx::Canvas* canvas)
{
    //__super::OnPaintBackground(canvas);
    gfx::Point xp_bl = uid_label_->GetBoundsInScreen().bottom_left();
    views::View::ConvertPointFromScreen(this, &xp_bl);

    static SkColor kGradientBegin = SkColorSetRGB(0x31, 0x32, 0x46);
    static SkColor kGradientEnd = SkColorSetRGB(0x31, 0x32, 0x46);

    scoped_ptr<views::Painter> painter(views::Painter::CreateVerticalGradient(kGradientBegin, kGradientEnd));
    views::Painter::PaintPainterAt(canvas, painter.get(),
        gfx::Rect(0, 0, width(), xp_bl.y() + kPaddingRowHeightForDiffGroups));

    SkPaint paint;
    paint.setColor(SkColorSetA(GetColor(TextTitle), kAlphaOpaqueOneTenths));
    paint.setStrokeWidth(2);

    gfx::Rect rect = live_room_button_->bounds();
    /*canvas->DrawLine(gfx::Point(0, rect.y() - kPaddingRowHeightForGroupCtrls),
        gfx::Point(width(), rect.y() - kPaddingRowHeightForGroupCtrls),
        paint);*/

    gfx::Point pt_bl = live_room_button_->bounds().bottom_left();
    gfx::Point pt_br = live_data_button_->bounds().bottom_right();
    canvas->DrawLine(gfx::Point(0, rect.bottom() + kPaddingRowHeightForGroupCtrls),
        gfx::Point(width(), rect.bottom() + kPaddingRowHeightForGroupCtrls),
        paint);
}

void TitleBarUserInfoViddupView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    int id = sender->id();
    switch (id)
    {
    case Button_Avatar:
    case Button_NickName:
    case Button_Room:
        ShellExecuteW(nullptr, L"open", bililive::FastForwardChangeEnv(GetLiveRoomUrl()).c_str(), nullptr, nullptr, SW_SHOW);
        if (id != Button_NickName) {
            EventTracking(kEventTable.at(TitleBarUserInfoViddupViewCtrlID(id)));
        }
        break;
    case Button_MedalFans:
        ShellExecuteW(nullptr, L"open", bililive::FastForwardChangeEnv(GetLiveRoomUrl()).c_str(), nullptr, nullptr, SW_SHOW);
        break;
    case Button_Logout:
        bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_LOGOUT);
        GetWidget()->Close();
        break;
    case Button_LiveData:
        ShellExecuteW(nullptr, L"open", bililive::FastForwardChangeEnv(kLiveData).c_str(), nullptr, nullptr, SW_SHOW);
        EventTracking(kEventTable.at(TitleBarUserInfoViddupViewCtrlID(id)));
        break;
    default:
        break;
    }
}

void TitleBarUserInfoViddupView::StartDoCheckMouse()
{
    timer_.Start(FROM_HERE, base::TimeDelta::FromMilliseconds(100),
        this, &TitleBarUserInfoViddupView::OnCheckMouse);
}

void TitleBarUserInfoViddupView::StopDoCheckMouse()
{
    if (timer_.IsRunning())
        timer_.Stop();
}

void TitleBarUserInfoViddupView::OnCheckMouse()
{
    if (GetWidget()->IsVisible())
    {
        gfx::Rect rcWidget = GetWidget()->GetWindowBoundsInScreen();
        gfx::Rect rcRelate = relation_view_->GetBoundsInScreen();
        if (rcRelate.bottom() <= rcWidget.y())
        {
            rcRelate.Inset(0, 0, 0, rcRelate.bottom() - rcWidget.y());
        }
        else
        {
            rcRelate.Inset(0, rcWidget.bottom() - rcRelate.y(), 0, 0);
        }
        static const int kExtendDetectThickness = GetLengthByDPIScale(20);
        rcWidget.Inset(-kExtendDetectThickness, -kExtendDetectThickness);
        gfx::Point cursor_pos = gfx::Screen::GetNativeScreen()->GetCursorScreenPoint();
        if (!rcWidget.Contains(cursor_pos) && !rcRelate.Contains(cursor_pos))
        {
            StopDoCheckMouse();
            static_cast<BililiveLabelButton*>(relation_view_)->SetState(
                views::Button::ButtonState::STATE_NORMAL);
            GetWidget()->Close();
        };
    }
}

std::wstring TitleBarUserInfoViddupView::GetLiveRoomUrl() {
    auto room_id = GetBililiveProcess()->secret_core()->user_info().room_id();
    std::wstring url = kLiveRoomLink;
    url.append(L"/");
    url.append(std::to_wstring(room_id));
    return url;
}

void TitleBarUserInfoViddupView::Observe(
    int type, const base::NotificationSource& source, const base::NotificationDetails& details)
{
    switch (type)
    {
    case bililive::NOTIFICATION_LIVEHIME_UPDATE_AVATAR:
    {
        UpdateAvatar();
    }
    break;
    case bililive::NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_SUCCESS:
    {
        UpdateRoomInfo();
    }
    break;
    case bililive::NOTIFICATION_LIVEHIME_UPDATE_SAN:
    {
        UpdateSan();
    }
    break;
    case bililive::NOTIFICATION_LIVEHIME_UPDATE_CAPTAIN:
    {
        UpdateCaptainLoss();
    }
    break;
    default:
        break;
    }
}

void TitleBarUserInfoViddupView::UpdateAvatar() {

    auto nickname = GetBililiveProcess()->secret_core()->user_info().nickname();
    user_name_button_->SetText(base::UTF8ToUTF16(nickname));

    auto& avatar_data = GetBililiveProcess()->secret_core()->user_info().avatar();
    avatar_->SetAvatar(avatar_data.data(), avatar_data.size());
}

void TitleBarUserInfoViddupView::UpdateRoomInfo() {
}

void TitleBarUserInfoViddupView::UpdateSan() {
    auto san = GetBililiveProcess()->secret_core()->user_info().san();
    if (san_button_) {
        san_button_->SetText(std::to_wstring(san));
    }
}

void TitleBarUserInfoViddupView::UpdateMedalView() {
    auto medal_status = GetBililiveProcess()->secret_core()->anchor_info().medal_status();
    auto medal_name = GetBililiveProcess()->secret_core()->anchor_info().medal_name();

    medal_view_->Update(medal_status, medal_name);
}

void TitleBarUserInfoViddupView::UpdateCaptainLoss()
{
    if (!warn_button_ || !sailors_icon_)
        return;

    int expired = GetBililiveProcess()->secret_core()->anchor_info().get_captain_expired();
    int will_expire = GetBililiveProcess()->secret_core()->anchor_info().get_captain_will_expire();

    if (expired > 0 || will_expire > 0)
    {
        warn_button_->UpdateValues(expired, will_expire);
        warn_button_->SetVisible(true);
        sailors_icon_->SetVisible(false);
    }
    else
    {
        warn_button_->UpdateValues(expired, will_expire);
        warn_button_->SetVisible(false);
        sailors_icon_->SetVisible(true);
    }

    container_view_->InvalidateLayout();
}