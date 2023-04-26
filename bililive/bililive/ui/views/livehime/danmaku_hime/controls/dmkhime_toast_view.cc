#include "dmkhime_toast_view.h"

#include "base/bind.h"
#include "base/prefs/pref_service.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/widget/widget_observer.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "grit/theme_resources.h"
#include "grit/generated_resources.h"


namespace
{
    const int kPadding = GetLengthByDPIScale(16);
    const int kMarginHorizontal = GetLengthByDPIScale(24);
    const SkColor kClrDefault = SkColorSetA(SkColorSetRGB(0x47, 0xd2, 0x79), 255);
    const SkColor kClrWarning = SkColorSetA(SkColorSetRGB(0xff, 0x64, 0x64), 255);

    class DmkhimeToastView
        : public views::WidgetDelegateView
        , public views::WidgetObserver
    {
    public:
        explicit DmkhimeToastView(
            views::Widget* parent, const gfx::Rect& parent_bounds,
            dmkhime::DmkToastType type, const base::string16 &text)
            : alpha_(255)
            , parent_bounds_(parent_bounds)
            , parent_(parent)
            , weakptr_factory_(this)
        {
            ResourceBundle &rb = ResourceBundle::GetSharedInstance();

            label_ = new LivehimeContentLabel(text);
            label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
            label_->SetAllowCharacterBreak(true);
            label_->SetMultiLine(true);
            label_->SetTextColor(SK_ColorWHITE);
            label_->set_no_subpixel_rendering(true);
            AddChildView(label_);

            paint_.setAntiAlias(true);
            paint_.setColor(clrLivehime);
            paint_.setStyle(SkPaint::kFill_Style);
        }

        ~DmkhimeToastView() {
        }

        void AdjustPosition() {
            if (!parent_) {
                return;
            }

            gfx::Rect parent_rect = parent_bounds_;
            gfx::Rect rect = GetWidget()->GetWindowBoundsInScreen();
            gfx::Rect bounds(
                parent_rect.x() + (parent_rect.width() - rect.width()) / 2,
                parent_rect.bottom() - GetLengthByDPIScale(72) - rect.height(),
                rect.width(), rect.height());
            GetWidget()->SetBounds(bounds);
        }

    protected:
        // WidgetDelegate
        views::View* GetContentsView() override {
            return this;
        }

        void WindowClosing() override {
            if (g_toast_view_ == this) {
                g_toast_view_ = nullptr;
            }

            if (parent_) {
                parent_->RemoveObserver(this);
            }
        }

        // View
        void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override {
            if (details.child == this) {
                if (details.is_add) {
                    if (parent_) {
                        parent_->AddObserver(this);
                    }

                    base::MessageLoop::current()->PostDelayedTask(
                        FROM_HERE,
                        base::Bind(&DmkhimeToastView::FadeOut, weakptr_factory_.GetWeakPtr()),
                        base::TimeDelta::FromSeconds(2));
                }
            }
        }

        gfx::Size GetPreferredSize() override {
            int max_width = parent_bounds_.width() - kMarginHorizontal * 2;
            auto content_size = label_->GetPreferredSize();
            int width = std::min(content_size.width() + kPadding * 2, max_width);

            int height = label_->GetHeightForWidth(width - kPadding * 2);
            height += kPadding * 2;

            return gfx::Size(width, height);
        }

        void Layout() override {
            auto bounds = GetContentsBounds();

            int label_x = kPadding;
            int label_cx = bounds.width() - kPadding - label_x;
            int label_cy = label_->GetHeightForWidth(label_cx);
            label_->SetBounds(
                bounds.x() + label_x,
                bounds.y() + (bounds.height() - label_cy) / 2,
                label_cx, label_cy);
        }

        void OnPaint(gfx::Canvas* canvas) override {
            static int radius = GetLengthByDPIScale(8);

            gfx::Rect rect = GetContentsBounds();
            canvas->DrawRoundRect(rect, radius, paint_);
        }

        // WidgetObserver
        void OnWidgetClosing(views::Widget* widget) override {
            if (widget == parent_) {
                parent_->RemoveObserver(this);
                parent_ = nullptr;
            }
        }

        void OnWidgetBoundsChanged(views::Widget* widget, const gfx::Rect& new_bounds) override {
            if (widget == parent_) {
                AdjustPosition();
            }
        }

    private:
        void FadeOut() {
            alpha_ -= 10;
            if (alpha_ > 0) {
                GetWidget()->SetOpacity(static_cast<unsigned char>(alpha_));

                base::MessageLoop::current()->PostDelayedTask(
                    FROM_HERE,
                    base::Bind(&DmkhimeToastView::FadeOut, weakptr_factory_.GetWeakPtr()),
                    base::TimeDelta::FromMilliseconds(16));
            } else {
                GetWidget()->Close();
            }
        }

    public:
        static DmkhimeToastView *g_toast_view_;

    private:
        int alpha_;
        SkPaint paint_;
        gfx::Rect parent_bounds_;
        LivehimeContentLabel *label_;

        views::Widget* parent_;
        base::WeakPtrFactory<DmkhimeToastView> weakptr_factory_;
    };

    DmkhimeToastView *DmkhimeToastView::g_toast_view_ = nullptr;
}


namespace dmkhime {

    void ShowDmkhimeToast(
        views::Widget* parent,
        const gfx::Rect& parent_bounds,
        DmkToastType type, const base::string16 &text)
    {
        if (DmkhimeToastView::g_toast_view_) {
            DmkhimeToastView::g_toast_view_->GetWidget()->Hide();
            DmkhimeToastView::g_toast_view_->GetWidget()->Close();
        }

        DmkhimeToastView::g_toast_view_ = new DmkhimeToastView(parent, parent_bounds, type, text);
        views::Widget *widget_ = new views::Widget();
        views::Widget::InitParams params(views::Widget::InitParams::TYPE_POPUP);
        params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;
        params.parent = parent->GetNativeView();
        params.delegate = DmkhimeToastView::g_toast_view_;
        params.child = true;
        params.accept_events = false;
        params.can_activate = false;
        params.remove_standard_frame = true;
        widget_->set_frame_type(views::Widget::FRAME_TYPE_FORCE_CUSTOM);
        widget_->Init(params);
        widget_->Hide();

        DmkhimeToastView::g_toast_view_->AdjustPosition();

        widget_->ShowInactive();
    }

    void CloseDmkhimeToast() {
        if (DmkhimeToastView::g_toast_view_) {
            DmkhimeToastView::g_toast_view_->GetWidget()->Hide();
            DmkhimeToastView::g_toast_view_->GetWidget()->Close();
        }
    }

}