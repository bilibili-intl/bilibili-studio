#include "bililive/bililive/ui/views/livehime/tips_dialog/fleet_achievement_dialog.h"

#include <regex>

#include "base/ext/callable_callback.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/display.h"
#include "ui/gfx/screen.h"

#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/bililive/utils/convert_util.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"


namespace {

    const int kCountdownSeconds = 5;

    class FleetImageButton : public BililiveImageButton {
    public:
        FleetImageButton(const string16& text, SkColor color, views::ButtonListener* listener)
            : BililiveImageButton(listener),
              text_color_(color),
              label_text_(text)
        {
            SetStretchPaint(true);
        }

        void SetText(const string16& text) { label_text_ = text; }

    private:
        void OnPaint(gfx::Canvas* canvas) override {
            __super::OnPaint(canvas);

            auto& font = ftTwelve;
            int cx = 0;
            int cy = 0;
            gfx::Canvas::SizeStringInt(label_text_, font, &cx, &cy, 0, gfx::Canvas::NO_ELLIPSIS);
            canvas->DrawStringInt(label_text_, font, text_color_,
                (width() - cx) / 2, (height() - cy) / 2, cx, cy);
        }

        SkColor text_color_;
        string16 label_text_;
    };

    class LivehimeNavyImageView : public LivehimeImageView {
    public:
        LivehimeNavyImageView() {}

    protected:
        gfx::ImageSkia OnImageChanging(
            const gfx::ImageSkia& img, int id, bool is_placeholder) override
        {
            if (id == -1) {
                return bililive::CreateCircleImageEx(img);
            }
            return img;
        }
    };

}


// static
void FleetAchievementDialog::ShowDialog(views::Widget* parent, const FleetAchievementInfo& info) {
    views::Widget* widget = new views::Widget();

    views::Widget::InitParams params;
    params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;
    params.native_widget = new BililiveNativeWidgetWin(widget);
    params.parent = parent->GetNativeView();

    auto view = new FleetAchievementDialog(info);
    ShowWidget(view, widget, params, false);

    if (parent->IsMinimized()) {
        AdjustWindowPosition(widget);
    }
}

FleetAchievementDialog::FleetAchievementDialog(const FleetAchievementInfo& info)
    : info_(info),
      countdown_(kCountdownSeconds),
      weak_ptr_factory_(this)
{
    InitViews();
}

FleetAchievementDialog::~FleetAchievementDialog() {
}

views::NonClientFrameView* FleetAchievementDialog::CreateNonClientFrameView(views::Widget *widget) {
    BililiveNonTitleBarFrameView* frame_view = new BililiveNonTitleBarFrameView(this);
    frame_view->SetEnableDragMove(true);
    return frame_view;
}

gfx::Size FleetAchievementDialog::GetPreferredSize() {
    int width_dip = 326;
    int height_dip = width_dip / 2.504f;
    static gfx::Size size(GetLengthByDPIScale(width_dip), GetLengthByDPIScale(height_dip));
    return size;
}

void FleetAchievementDialog::Layout() {
    __super::Layout();

    auto bounds = GetContentsBounds();
    bg_view_->SetBounds(bounds.x(), bounds.y(), bounds.width(), bounds.height());

    auto size = avatar_view_->GetPreferredSize();
    avatar_view_->SetBounds(
        bounds.x() + GetLengthByDPIScale(33),
        bounds.y() + (bounds.height() - size.height()) / 2,
        size.width(), size.height());

    size = button_view_->GetPreferredSize();
    button_view_->SetBounds(
        bounds.right() - size.width() - GetLengthByDPIScale(36),
        bounds.bottom() - size.height() - GetLengthByDPIScale(14),
        size.width(), size.height());

    {
        // 一行的左半部分
        size = left_label_->GetPreferredSize();
        int x = bounds.x() + GetLengthByDPIScale(114);
        // 限制宽度，防止溢出
        int width = std::min(bounds.right() - x - GetLengthByDPIScale(14), size.width());
        left_label_->SetBounds(
            x, bounds.y() + GetLengthByDPIScale(32),
            width, size.height());
    }

    {
        // 一行的右半部分
        size = right_label_->GetPreferredSize();
        int x = left_label_->bounds().right() + GetLengthByDPIScale(2);
        // 限制宽度，防止溢出
        int width = std::min(bounds.right() - x - GetLengthByDPIScale(14), size.width());
        width = std::max(0, width);
        right_label_->SetBounds(
            x, bounds.y() + GetLengthByDPIScale(32),
            width, size.height());
    }

    int tip_width = bounds.width() - GetLengthByDPIScale(110 + 34);
    int tip_height = GetLengthByDPIScale(32);
    tip_label_->SetBounds(
        bounds.x() + GetLengthByDPIScale(114),
        left_label_->bounds().bottom() + GetLengthByDPIScale(2),
        tip_width, tip_height);
}

int FleetAchievementDialog::NonClientHitTest(const gfx::Point &point) {
    gfx::Point tp = point;
    ConvertPointToTarget(this, button_view_, &tp);
    bool hit_iknow = button_view_->HitTestPoint(tp);
    if (hit_iknow) {
        return HTNOWHERE;
    }

    return HTCAPTION;
}

void FleetAchievementDialog::InitViews() {
    set_background(nullptr);

    auto& rb = ui::ResourceBundle::GetSharedInstance();

    SkColor text_color;
    SkColor button_text_color;
    if (info_.level < FLEET_ACH_THOUSAND) {
        text_color = SK_ColorWHITE;
        button_text_color = SK_ColorBLACK;
    } else if (info_.level < FLEET_ACH_TEN_THOUSAND) {
        text_color = SK_ColorWHITE;
        button_text_color = SK_ColorWHITE;
    } else {
        text_color = SK_ColorBLACK;
        button_text_color = SK_ColorBLACK;
    }

    bool left;
    string16 plain_text, highligh_text;
    ProcessFirstLineText(info_.first_line_text, &plain_text, &highligh_text, &left);

    SkColor highligh_color;
    if (!bililive::ParseColorString(info_.highligh_color, &highligh_color)) {
        highligh_color = text_color;
    }

    // 标题左半边
    left_label_ = new LivehimeTitleLabel();
    left_label_->SetFont(ftFourteenBold);
    left_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);

    // 标题右半边
    right_label_ = new LivehimeTitleLabel();
    right_label_->SetFont(ftFourteenBold);
    right_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);

    if (left) {
        // 高亮文字在左边
        left_label_->SetText(highligh_text);
        left_label_->SetTextColor(highligh_color);

        right_label_->SetText(plain_text);
        right_label_->SetTextColor(text_color);
    } else {
        // 高亮文字在右边
        left_label_->SetText(plain_text);
        left_label_->SetTextColor(text_color);

        right_label_->SetText(highligh_text);
        right_label_->SetTextColor(highligh_color);
    }

    // 说明
    tip_label_ = new LivehimeTipLabel(info_.second_line_text);
    tip_label_->SetTextColor(text_color);
    tip_label_->SetMultiLine(true);
    tip_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    tip_label_->SetVerticalAlignment(gfx::ALIGN_TOP);

    // 背景图 View
    bg_view_ = new LivehimeImageView();
    bg_view_->SetEnabled(false);

    int insets = GetLengthByDPIScale(4);

    // 头像/头像框 View
    avatar_view_ = new LivehimeNavyImageView();
    avatar_view_->SetEnabled(false);
    avatar_view_->SetPreferredSize(GetSizeByDPIScale(gfx::Size(64, 64)));
    avatar_view_->SetImagePlaceholder(*rb.GetImageSkiaNamed(IDR_LIVEMAIN_TITLEBAR_USER_FACE));
    avatar_view_->SetImageUrl(info_.avatar_url);
    avatar_view_->SetImagePadding(gfx::Insets(insets, insets, insets, insets));
    avatar_view_->SetImageUrl(info_.avatar_frame_url, 0);

    auto str = base::StringPrintf(
        rb.GetLocalizedString(IDS_TIP_DLG_NAVY_ACHIEVEMENT_IKNOW).c_str(),
        countdown_);

    // 按钮 View
    button_view_ = new FleetImageButton(str, button_text_color, this);
    button_view_->SetPreferredSize(GetSizeByDPIScale(gfx::Size(90, 24)));
    if (info_.level < FLEET_ACH_THOUSAND) {
        button_view_->SetAllStateImage(rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_TIPS_DIALOG_FLEET_HUND_BUTTON));
    } else if (info_.level < FLEET_ACH_TEN_THOUSAND) {
        button_view_->SetAllStateImage(rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_TIPS_DIALOG_FLEET_THOU_BUTTON));
    } else {
        button_view_->SetAllStateImage(rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_TIPS_DIALOG_FLEET_TTHO_BUTTON));
    }

    AddChildView(bg_view_);
    AddChildView(avatar_view_);
    AddChildView(button_view_);
    AddChildView(left_label_);
    AddChildView(right_label_);
    AddChildView(tip_label_);

    DownloadBgImage();
}

void FleetAchievementDialog::DownloadBgImage() {
    if (info_.bg_url.empty()) {
        GetWidget()->Close();
        return;
    }

    auto callback = base::MakeCallable(base::Bind(
        &FleetAchievementDialog::OnGetImageByUrl,
        weak_ptr_factory_.GetWeakPtr()));

    GetBililiveProcess()->secret_core()->danmaku_hime_service()->DownloadImage(
        info_.bg_url, callback).Call();
}

void FleetAchievementDialog::ButtonPressed(views::Button* sender, const ui::Event& event) {
    if (sender == button_view_) {
        GetWidget()->Close();
    }
}

void FleetAchievementDialog::OnCountdownTimer() {
    if (countdown_ == 0) {
        timer_.Stop();
        GetWidget()->Close();
        return;
    }
    --countdown_;

    auto& rb = ui::ResourceBundle::GetSharedInstance();
    auto str = base::StringPrintf(
        rb.GetLocalizedString(IDS_TIP_DLG_NAVY_ACHIEVEMENT_IKNOW).c_str(),
        countdown_);
    static_cast<FleetImageButton*>(button_view_)->SetText(str);
    button_view_->SchedulePaint();
}

void FleetAchievementDialog::OnGetImageByUrl(
    bool valid_response, const std::string& data)
{
    if (!valid_response || data.empty()) {
        GetWidget()->Close();
        return;
    }

    auto img = bililive::MakeSkiaImage(
        reinterpret_cast<const unsigned char*>(data.data()), data.length());
    if (img.isNull()) {
        GetWidget()->Close();
        return;
    }

    bg_view_->SetImage(img);

    GetWidget()->Show();

    timer_.Start(
        FROM_HERE, base::TimeDelta::FromSeconds(1),
        base::Bind(&FleetAchievementDialog::OnCountdownTimer, base::Unretained(this)));
}

void FleetAchievementDialog::ProcessFirstLineText(
    const string16& org_text, string16* plain_text, string16* highlight_text, bool* left)
{
    if (org_text.empty()) {
        plain_text->clear();
        highlight_text->clear();
        return;
    }

    auto TrimSym = [](const string16& t) -> string16 {
        string16 out(t);
        ReplaceSubstringsAfterOffset(&out, 0, L"<%", L"");
        ReplaceSubstringsAfterOffset(&out, 0, L"%>", L"");
        return out;
    };

    {
        // 先匹配开头的高亮字符串
        std::wregex reg(L"^<%((?:.|\n)*?)%>((?:.|\n)*)$");
        std::wsmatch mat;
        if (std::regex_match(org_text, mat, reg)) {
            *highlight_text = TrimSym(mat.str(1));
            *plain_text = TrimSym(mat.str(2));
            *left = true;
            return;
        }
    }

    {
        // 开头没有高亮字符串，再匹配末尾的高亮字符串
        std::wregex reg(L"^((?:.|\n)*)<%((?:.|\n)*?)%>$");
        std::wsmatch mat;
        if (std::regex_match(org_text, mat, reg)) {
            *plain_text = TrimSym(mat.str(1));
            *highlight_text = TrimSym(mat.str(2));
            *left = false;
            return;
        }
    }

    // 开头结尾都没有高亮字符串，就当成普通字符串处理
    *plain_text = TrimSym(org_text);
    highlight_text->clear();
    *left = false;
}

// static
void FleetAchievementDialog::AdjustWindowPosition(views::Widget* widget) {
    gfx::Display disp = gfx::Screen::GetNativeScreen()->GetPrimaryDisplay();
    gfx::Rect work_area = disp.work_area();
    gfx::Rect wnd_rect = widget->GetWindowBoundsInScreen();
    gfx::Point position;
    position.set_x(work_area.x() + (work_area.width() - wnd_rect.width()) / 2);
    position.set_y(work_area.y() + (work_area.height() - wnd_rect.height()) / 2);
    widget->SetBounds(gfx::Rect(position.x(), position.y(), wnd_rect.width(), wnd_rect.height()));
}