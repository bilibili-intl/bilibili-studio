#include "tabarea_toast_view.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_observer.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/fill_layout.h"

#include "bililive/public/bililive/bililive_process.h"

#include "base/prefs/pref_service.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/stringprintf.h"

#include "ui/base/resource/resource_bundle.h"
#include "grit/theme_resources.h"
#include "grit/generated_resources.h"


namespace
{
    const SkColor kClrDefault = SkColorSetA(SkColorSetRGB(0x47, 0xd2, 0x79), 255);
    const SkColor kClrWarning = SkColorSetA(SkColorSetRGB(0xff, 0x64, 0x64), 255);

    class TabAreaToastView
        : public views::WidgetDelegateView
        , public views::WidgetObserver
    {
    public:
        explicit TabAreaToastView(TabAreaToastType type, const base::string16 &text)
            : image_(nullptr)
            , alpha_(255)
            , color_bk_(kClrDefault)
            , weakptr_factory_(this)
        {
            ResourceBundle &rb = ResourceBundle::GetSharedInstance();
            switch (type)
            {
            case TabAreaToastType_Default:
                image_ = rb.GetImageSkiaNamed(IDR_LIVEHIME_TABAREA_TOAST_OK);
                color_bk_ = kClrDefault;
                break;
            case TabAreaToastType_Warning:
                image_ = rb.GetImageSkiaNamed(IDR_LIVEHIME_TABAREA_TOAST_WARN);
                color_bk_ = kClrWarning;
                break;
            default:
                break;
            }
            label_ = new LivehimeContentLabel(text);
            label_->SetAllowCharacterBreak(true);
            label_->SetMultiLine(true);
            label_->SetTextColor(SK_ColorWHITE);
            label_->set_no_subpixel_rendering(true);
            label_->SetFont(ftThirteen);
            AddChildView(label_);
        }

        virtual ~TabAreaToastView()
        {
        };

        void AdjustPosition()
        {
            //gfx::Rect tab_rect = GetBililiveProcess()->bililive_obs()->obs_view()->tabarea_view()->GetMiddleAreaBounds();
            //gfx::Point ori = tab_rect.origin();
            //views::View::ConvertPointToScreen(GetBililiveProcess()->bililive_obs()->obs_view()->tabarea_view(), &ori);
            //gfx::Rect rect = GetWidget()->GetWindowBoundsInScreen();
            //gfx::Rect bounds(ori.x(), ori.y() + (tab_rect.height() - rect.height()) / 2, rect.width(), rect.height());
            //GetWidget()->SetBounds(bounds);
        };

    protected:
        // WidgetDelegate
        virtual views::View* GetContentsView() override
        {
            return this;
        }

        virtual void WindowClosing() override
        {
            if (TabAreaToastView::g_toast_view_ == this)
            {
                g_toast_view_ = nullptr;
            }
            GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->RemoveObserver(this);
        }

        // View
        virtual void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override
        {
            if (details.child == this)
            {
                if (details.is_add)
                {
                    GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->AddObserver(this);

                    base::MessageLoop::current()->PostDelayedTask(FROM_HERE, 
                        base::Bind(&TabAreaToastView::FadeOut, weakptr_factory_.GetWeakPtr()), base::TimeDelta::FromSeconds(3));
                }
            }
        };

        //virtual gfx::Size GetPreferredSize() override
        //{
        //    gfx::Rect mid_bounds = GetBililiveProcess()->bililive_obs()->obs_view()->tabarea_view()->GetMiddleAreaBounds();
        //    mid_bounds = GetBililiveProcess()->bililive_obs()->obs_view()->tabarea_view()->ConvertRectToWidget(mid_bounds);
        //    int label_x = 0;
        //    int cy = 0;
        //    if (image_)
        //    {
        //        label_x = image_->width();
        //        cy = image_->height();
        //    }
        //    int label_cy = label_->GetHeightForWidth(mid_bounds.width() - kPaddingColWidthForGroupCtrls * 2 - label_x);
        //    label_cy += ftPrimary.GetHeight();
        //    cy = std::max(cy, label_cy);
        //    return gfx::Size(mid_bounds.width(), cy);
        //};

        virtual void Layout() override
        {
            int label_x = kPaddingColWidthForGroupCtrls;
            if (image_)
            {
                label_x += image_->width();
            }
            int label_cx = width() - kPaddingColWidthForGroupCtrls - label_x;
            int label_cy = label_->GetHeightForWidth(label_cx);
            label_cy += ftPrimary.GetHeight();
            label_->SetBounds(label_x, height() - label_cy, label_cx, label_cy);
        };

        virtual void OnPaint(gfx::Canvas* canvas) override
        {
            gfx::Rect rect = label_->bounds();
            gfx::Rect bk_rect(0, rect.y(), width(), rect.height());
            canvas->FillRect(bk_rect, color_bk_);
            if (image_)
            {
                canvas->DrawImageInt(*image_, 0, (height() - image_->height()) / 2);
            }
        };

        // WidgetObserver
        virtual void OnWidgetBoundsChanged(views::Widget* widget, const gfx::Rect& new_bounds) override
        {
            if (widget == GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget())
            {
                AdjustPosition();
            }
        }

    private:
        void FadeOut()
        {
            alpha_ -= 25;
            if (alpha_ > 0)
            {
                GetWidget()->SetOpacity((unsigned char)alpha_);

                base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
                    base::Bind(&TabAreaToastView::FadeOut, weakptr_factory_.GetWeakPtr()), base::TimeDelta::FromMilliseconds(100));
            }
            else
            {
                GetWidget()->Close();
            }
        }

    public:
        static TabAreaToastView *g_toast_view_;

    private:
        gfx::ImageSkia *image_;
        SkColor color_bk_;
        LivehimeContentLabel *label_;
        int alpha_;

        base::WeakPtrFactory<TabAreaToastView> weakptr_factory_;
    };

    TabAreaToastView *TabAreaToastView::g_toast_view_ = nullptr;
}


void ShowTabAreaToast(TabAreaToastType type, const base::string16 &text)
{
    if (TabAreaToastView::g_toast_view_)
    {
        TabAreaToastView::g_toast_view_->GetWidget()->Hide();
        TabAreaToastView::g_toast_view_->GetWidget()->Close();
    }

    TabAreaToastView::g_toast_view_ = new TabAreaToastView(type, text);
    views::Widget *widget_ = new views::Widget();
    views::Widget::InitParams params(views::Widget::InitParams::TYPE_POPUP);
    params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;
    params.parent = GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeView();
    params.delegate = TabAreaToastView::g_toast_view_;
    params.child = true;
    params.accept_events = false;
    params.can_activate = false;
    params.remove_standard_frame = true;
    widget_->set_frame_type(views::Widget::FRAME_TYPE_FORCE_CUSTOM);
    widget_->Init(params);
    widget_->Hide();

    TabAreaToastView::g_toast_view_->AdjustPosition();

    widget_->ShowInactive();
}
