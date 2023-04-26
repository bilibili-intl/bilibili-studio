#include "bililive/bililive/ui/views/livehime/colive/colive_crop_view.h"

#include "base/bind.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/screen.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"

#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/controls/bililive_floating_view.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/livehime/colive/colive_preview_control.h"
#include "bililive/bililive/ui/views/livehime/colive/colive_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"

#include "grit/generated_resources.h"

#include "obs/obs_proxy/public/proxy/obs_proxy.h"
#include "obs/obs_proxy/ui_proxy/obs_preview/obs_preview.h"


namespace
{
    class PreviewCropContentView
        : public views::View
    {
        class DragView : public views::View
        {
        public:
            explicit DragView(PreviewCropContentView *capture_view)
                : capture_view_(capture_view)
            {
                set_background(views::Background::CreateSolidBackground(SkColorSetA(SK_ColorWHITE, 1)));

                set_border(views::Border::CreateSolidBorder(1, SK_ColorRED));
            }

            ~DragView() = default;

            void Relocate(const gfx::RectF& region)
            {
                static float kScale = 8 * 1.0f / 9;
                gfx::Rect limit_rect = capture_view_->valid_preview_rect_;

                int cy = limit_rect.height();
                int cx = std::round(cy * kScale);
                if (!LiveModelController::GetInstance()->IsLandscapeModel())
                {
                    cx = limit_rect.width();
                    cy = std::round(cx / kScale);
                }

                int real_x = limit_rect.x() + limit_rect.width() * region.x();
                real_x = std::max(real_x, limit_rect.x());
                real_x = std::min(real_x, limit_rect.right() - cx);
                int real_y = limit_rect.y() + limit_rect.height() * region.y();
                real_y = std::max(real_y, limit_rect.y());
                real_y = std::min(real_y, limit_rect.bottom() - cy);
                SetBounds(real_x, real_y, cx, cy);
            }

        protected:
            bool OnMousePressed(const ui::MouseEvent& event) override
            {
                POINT point = { 0 };
                ::GetCursorPos(&point);
                down_point_ = point;
                down_origin_ = bounds().origin();
                return true;
            }

            bool OnMouseDragged(const ui::MouseEvent& event) override
            {
                POINT point = { 0 };
                ::GetCursorPos(&point);
                gfx::Point curpt(point);
                int xoffset = curpt.x() - down_point_.x();
                int yoffset = curpt.y() - down_point_.y();
                int new_x = down_origin_.x() + xoffset;
                int new_y = down_origin_.y() + yoffset;

                new_x = std::max(new_x, capture_view_->valid_preview_rect_.x());
                new_x = std::min(new_x, capture_view_->valid_preview_rect_.right() - width());
                new_y = std::max(new_y, capture_view_->valid_preview_rect_.y());
                new_y = std::min(new_y, capture_view_->valid_preview_rect_.bottom() - height());

                SetPosition(gfx::Point(new_x, new_y));
                capture_view_->Update();

                return true;
            }

        private:
            gfx::Point down_point_;
            gfx::Point down_origin_;
            PreviewCropContentView *capture_view_;
        };

    public:
        explicit PreviewCropContentView(PreviewCropDelegate* d)
            : is_init_(true),
              delegate_(d)
        {
            drag_view_ = new DragView(this);
            AddChildView(drag_view_);
        }

        virtual ~PreviewCropContentView()
        {
        }

    protected:
        // View
        void OnPaintBackground(gfx::Canvas* canvas) override
        {
            SkPath path;
            path.addRect(SkRect::MakeXYWH(0, 0, width(), height()));
            gfx::Rect drag_rect = drag_view_->bounds();
            SkPath drag_path;
            drag_path.addRect(SkRect::MakeXYWH(drag_rect.x(), drag_rect.y(), drag_rect.width(), drag_rect.height()));
            path.reverseAddPath(drag_path);
            SkPaint paint;
            paint.setColor(clrColiveCropShade);
            canvas->DrawPath(path, paint);
        }

        void OnBoundsChanged(const gfx::Rect& previous_bounds) override
        {
            // 确定预览区区域，即裁减视口所能滑动的整个区域
            CalculateValidPreviewRect();

            // 重新定位裁剪区
            gfx::RectF region(0.5f, 0, 0.5f, 1.0f);
            if (delegate_) {
                region = delegate_->GetCropRegion();
            }
            drag_view_->Relocate(region);
        }

    private:
        void CalculateValidPreviewRect()
        {
            // 由于该View的大小已由父View（PreviewCropView）按照16:9或者9:16比例给确定了，
            // 而我们的预览分辨率（推流分辨率）当前是固定几个（都是16:9）的，
            // 那么在这里就不需要再额外的去根据已给好的View尺寸去二度计算预览尺寸了，
            // 直接采用当前View的尺寸即可
            valid_preview_rect_ = GetLocalBounds();
        }

        gfx::RectF current_capture_region()
        {
            int xoffset = drag_view_->bounds().x() - valid_preview_rect_.x();
            int yoffset = drag_view_->bounds().y() - valid_preview_rect_.y();
            float xs = xoffset * 1.0f / valid_preview_rect_.width();
            float ys = yoffset * 1.0f / valid_preview_rect_.height();

            float ws, hs;
            if (LiveModelController::GetInstance()->IsLandscapeModel()) {
                hs = 1.f;
                ws = hs * 9 / 9 * 8 / 16;
            } else {
                ws = 1.f;
                hs = ws * 9 / 8 * 9 / 16;
            }

            return gfx::RectF(xs, ys, ws, hs);
        }

        void Update()
        {
            SchedulePaint();

            if (delegate_) {
                gfx::RectF region = current_capture_region();
                delegate_->SetCropRegion(region);
            }
        }

    private:
        bool is_init_;
        DragView *drag_view_;
        gfx::Rect valid_preview_rect_; // 有效的预览渲染区
        PreviewCropDelegate* delegate_;
    };
}


PreviewCropView* PreviewCropView::instance_ = nullptr;

void PreviewCropView::ShowForm(View *related_view, PreviewCropDelegate* d)
{
    if (!instance_)
    {
        views::Widget *widget = new views::Widget();
        BililiveNativeWidgetWin *native_widget = new BililiveNativeWidgetWin(widget);
        native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_CLOSE);
        views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.native_widget = native_widget;
        params.parent = related_view->GetWidget()->GetNativeView();
        params.can_activate = true;

        instance_ = new PreviewCropView(d);
        params.delegate = instance_;
        params.remove_standard_frame = true;

        widget->set_frame_type(views::Widget::FRAME_TYPE_FORCE_CUSTOM);
        widget->Init(params);
        widget->Hide();

        gfx::Rect relate_rect = related_view->GetBoundsInScreen();
        gfx::Rect rect = widget->GetWindowBoundsInScreen();
        widget->SetBounds(gfx::Rect(
            relate_rect.x(), relate_rect.y() - GetLengthByDPIScale(20) - rect.height(),
            rect.width(), rect.height()));
        widget->Show();
    }
    else
    {
        if (instance_->GetWidget())
        {
            instance_->GetWidget()->Activate();
        }
    }
}

PreviewCropView::PreviewCropView(PreviewCropDelegate* d)
    : obs_preview_control_(nullptr)
    , capture_view_(nullptr)
    , tip_label_(nullptr)
    , start_check_active_(false)
    , delegate_(d)
    , weakptr_factory_(this) {
}

PreviewCropView::~PreviewCropView()
{
    instance_ = nullptr;
}

void PreviewCropView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this && details.is_add)
    {
        ResourceBundle& rb = ResourceBundle::GetSharedInstance();
        set_background(views::Background::CreateSolidBackground(clrColiveToolBar));

        auto layout = new views::GridLayout(this);
        SetLayoutManager(layout);

        auto column_set = layout->AddColumnSet(0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column_set->AddColumn(
            views::GridLayout::FILL, views::GridLayout::FILL,
            1.0f, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);

        //obs_preview_control_ = new ColivePreviewControl(obs_proxy_ui::PreviewType::PREVIEW_COLIVE_CROP);
        //capture_view_ = new BililiveFloatingView(
        //    obs_preview_control_, new PreviewCropContentView(delegate_));
        //capture_view_->SetFloatingViewDelegate(this);

        tip_label_ = new LivehimeSmallContentLabel(rb.GetLocalizedString(IDS_COLIVE_CROP_SCENE_TIP));
        tip_label_->SetTextColor(clrColiveTextTip);

        layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
        layout->StartRow(1.0f, 0);
        layout->AddView(capture_view_);
        layout->AddPaddingRow(0, kPaddingRowHeightForCtrlTips);
        layout->StartRow(0, 0);
        layout->AddView(tip_label_);
        layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);

        base::MessageLoop::current()->PostTask(FROM_HERE,
            base::Bind(&PreviewCropView::StartCheckActive, weakptr_factory_.GetWeakPtr()));
    }
}

gfx::Size PreviewCropView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    gfx::Size label_size = tip_label_->GetPreferredSize();

    // 给连麦服务器的目前只是8:9的画面，那么采取固定宽度，动态调整高度的方式来展示区域截取窗
    int cx = std::max(label_size.width(), GetLengthByDPIScale(160));

    int cy = size.height() + std::ceil(cx / 1.78f);
    if (!LiveModelController::GetInstance()->IsLandscapeModel())
    {
        cy = size.height() + std::ceil(cx / 0.5625f);
    }

    return gfx::Size(cx + kPaddingColWidthForGroupCtrls * 2, cy);
}

void PreviewCropView::OnFloatingViewKeyEvent(BililiveFloatingView *floating_view, ui::KeyEvent* event)
{
    if (event->type() == ui::EventType::ET_KEY_RELEASED)
    {
        if (event->key_code() == VK_ESCAPE)
        {
            GetWidget()->Close();
        }
    }
}

void PreviewCropView::OnFloatingViewFocus(BililiveFloatingView *floating_view)
{
}

void PreviewCropView::OnFloatingViewBlur(BililiveFloatingView *floating_view)
{
    if (start_check_active_)
    {
        GetWidget()->Close();
    }
}

void PreviewCropView::StartCheckActive()
{
    capture_view_->GetFloatingWidget()->Activate();
    start_check_active_ = true;
}