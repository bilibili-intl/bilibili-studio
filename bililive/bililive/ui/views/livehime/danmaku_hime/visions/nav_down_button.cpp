#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/nav_down_button.h"

#include "bililive/bililive/ui/views/livehime/danmaku_hime/d2d_manager.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/paint.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/sculptor.h"


namespace {

    const int kLeftPadding = 10;   // 文字左侧空间
    const int kRightPadding = 10;  // 箭头右侧空间
    const int kMiddlePadding = 6;  // 文字和箭头中间的空间
    const int kVertPadding = 6;    // 上部和下部空间
    const int kBottomMargin = 8;   // 按钮底部距离 View 底部的距离
    const int kArrowWidth = 14;    // 箭头宽度

    const D2D1_COLOR_F clrBgColor = D2D1::ColorF(0x0ebeff);

}

namespace dmkhime {

    NavDownButton::NavDownButton()
        : ind_anime_(nullptr)
    {
        ind_anime_.SetSlideDuration(40 * 6);
        ind_anime_.SetTweenType(ui::Tween::EASE_IN);

        float s_x = D2DManager::GetInstance()->GetDpiX() / 96;
        text_tl_.SetWrap(false);
        text_tl_.SetFontSize(12 * s_x);
        text_tl_.SetMaxWidth(text_tl_.GetTextWidth());
    }

    void NavDownButton::Show() {
        ind_anime_.Show();
    }

    void NavDownButton::Hide() {
        ind_anime_.Hide();
    }

    void NavDownButton::SetText(const string16& text) {
        float s_x = D2DManager::GetInstance()->GetDpiX() / 96;

        text_tl_.Init(text, 0);
        text_tl_.SetWrap(false);
        text_tl_.SetFontSize(12 * s_x);
        text_tl_.SetMaxWidth(text_tl_.GetTextWidth());
    }

    bool NavDownButton::Hit(const gfx::Point& p) const {
        if (!ind_anime_.is_animating() &&
            ind_anime_.IsShowing() &&
            !nav_down_btn_rect_.IsEmpty())
        {
            // 中间的矩形
            auto radius = nav_down_btn_rect_.height() / 2.f;
            auto mid_rect = nav_down_btn_rect_;
            mid_rect.Inset(radius, 0, radius, 0);
            if (mid_rect.Contains(p)) {
                return true;
            }

            // 左半圆
            gfx::Point l_center(mid_rect.x(), mid_rect.y() + mid_rect.height() / 2.f);
            if (std::pow((p.x() - l_center.x()) / radius, 2) +
                std::pow((p.y() - l_center.y()) / radius, 2) <= 1)
            {
                return true;
            }

            // 右半圆
            gfx::Point r_center(mid_rect.right(), mid_rect.y() + mid_rect.height() / 2.f);
            if (std::pow((p.x() - r_center.x()) / radius, 2) +
                std::pow((p.y() - r_center.y()) / radius, 2) <= 1)
            {
                return true;
            }
        }
        return false;
    }

    bool NavDownButton::IsAnimating() const {
        return ind_anime_.is_animating();
    }

    void NavDownButton::OnDrawOver(Sculptor* s, const gfx::Rect& bounds) {
        if (ind_anime_.is_animating() || ind_anime_.IsShowing()) {
            float s_x = D2DManager::GetInstance()->GetDpiX() / 96;
            float s_y = D2DManager::GetInstance()->GetDpiY() / 96;

            int width = text_tl_.GetTextWidth()
                + (kLeftPadding + kRightPadding + kMiddlePadding + kArrowWidth) * s_x;
            int height = text_tl_.GetTextHeight() + kVertPadding * 2 * s_y;

            int cy_hide = bounds.bottom() + height;
            int cy_show = bounds.bottom() - height - kBottomMargin * s_y;
            int cy = cy_hide - (cy_hide - cy_show) * ind_anime_.GetCurrentValue();

            int x = bounds.x() + (bounds.width() - width) / 2.f;
            auto rect = D2D1::RectF(x, cy, x + width, cy + height);

            // 圆角背景
            s->GetPaint()->SetStyle(Paint::FILL);
            s->GetPaint()->SetColor(clrBgColor);
            s->DrawRoundRect(rect, height / 2.f);

            int cx = rect.left + kLeftPadding * s_x;

            // 文字
            s->GetPaint()->SetColor(D2D1::ColorF(D2D1::ColorF::White));
            s->DrawTextLayout(text_tl_, cx, cy + kVertPadding * s_y);

            cx += text_tl_.GetTextWidth() + kMiddlePadding * s_x;

            // 箭头
            s->GetPaint()->SetStrokeWidth(1 * s_x);
            s->GetPaint()->SetColor(D2D1::ColorF(D2D1::ColorF::White));

            auto p1 = D2D1::Point2F(cx, cy + height / 2 - 2 * s_y);
            auto p2 = D2D1::Point2F(cx + kArrowWidth / 2.f * s_x, cy + height / 2 + 4 * s_y);
            auto p3 = D2D1::Point2F(cx + kArrowWidth * s_x, p1.y);
            s->DrawLine(p1, p2);
            s->DrawLine(p2, p3);

            nav_down_btn_rect_.SetRect(
                rect.left, rect.top, width, height);
        }
    }

}
