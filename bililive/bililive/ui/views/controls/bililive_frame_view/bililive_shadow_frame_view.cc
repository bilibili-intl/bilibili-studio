#include "bililive_shadow_frame_view.h"

#include "ui/base/win/dpi.h"
#include "ui/views/bubble/bubble_border.h"
#include "ui/views/widget/widget.h"


namespace
{
    const SkColor clrShadowFrameBackground = SK_ColorWHITE;
    const int big_shadow_bottom_cy = 4;
}


BiliShadowFrameView::BiliShadowFrameView(views::View *title_bar)
    : views::BubbleFrameView(gfx::Insets())
    , title_bar_(title_bar)
    , show_border_(false)
{
    // �Ƴ�����BubbleFrameView���캯�����洴���ı���label�͹رհ�ť
    RemoveAllChildViews(true);

    SetBubbleBorder(new views::BubbleBorder(views::BubbleBorder::NONE,
        views::BubbleBorder::BIG_SHADOW,
        clrShadowFrameBackground));    // ����ɫ��������Ӱɫ

    DCHECK(title_bar_ != nullptr);
    AddChildView(title_bar_);
}

void BiliShadowFrameView::SetMinMaxSize(const gfx::Size &min_size, const gfx::Size &max_size)
{
    min_size_ = min_size;
    max_size_ = max_size;
}

int BiliShadowFrameView::NonClientHitTest(const gfx::Point& point)
{
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {
        if (title_bar_ && title_bar_->bounds().Contains(point))
        {
            return HTCAPTION;
        }
    }
    return HTNOWHERE;
}

void BiliShadowFrameView::UpdateWindowTitle()
{
    /*title_->SetText(GetWidget()->widget_delegate()->ShouldShowWindowTitle() ?
        GetWidget()->widget_delegate()->GetWindowTitle() : string16());*/
    // Update the close button visibility too, otherwise it's not intialized.
    ResetWindowControls();
}

gfx::Rect BiliShadowFrameView::GetWindowBoundsForClientBounds(const gfx::Rect& client_bounds) const
{
    // �ó����������ͻ����ı߿�ȫ�ߴ�
    gfx::Rect rect = const_cast<BiliShadowFrameView*>(this)->GetUpdatedWindowBounds(
        gfx::Rect(), client_bounds.size(), false);
    rect.Inset(-GetInsets());
    // �ӱ������ߴ�
    if (title_bar_)
    {
        rect.Inset(0, -title_bar_->GetPreferredSize().height(), 0, 0);
    }
    // �����Ƿ���ʾ��Ӱ�ڱ߿���гߴ�����
    rect.Inset(GetShadowInsets());
    return rect;
}

gfx::Size BiliShadowFrameView::GetMaximumSize()
{
    if (!max_size_.IsEmpty())
    {
        return max_size_;
    }
    return GetWidget()->client_view()->GetMaximumSize();
}

gfx::Size BiliShadowFrameView::GetMinimumSize()
{
    if (!min_size_.IsEmpty())
    {
        return min_size_;
    }
    return GetWidget()->client_view()->GetMinimumSize();
}

void BiliShadowFrameView::Layout()
{
    if (title_bar_)
    {
        gfx::Rect title_bounds = CalculateTitleBarAreaBounds();
        title_bar_->SetBoundsRect(title_bounds);
    }

    views::View *client_view = GetWidget()->client_view();
    if (client_view)
    {
        gfx::Rect client_view_bounds_ = CalculateClientAreaBounds();
        client_view->SetBoundsRect(client_view_bounds_);
    }
}

gfx::Rect BiliShadowFrameView::CalculateNonClientFrameValidBounds() const
{
    gfx::Rect client_bounds = GetLocalBounds();
    client_bounds.Inset(GetInsets());
    if (border())
    {
        client_bounds.Inset(border()->GetInsets());
        if (!show_border_)
        {
            static auto scale = ui::win::GetDeviceScaleFactor();
            client_bounds.Inset(-1 * scale, -1 * scale, -1 * scale, -big_shadow_bottom_cy * scale);
        }
    }
    return client_bounds;
}

gfx::Rect BiliShadowFrameView::CalculateTitleBarAreaBounds() const
{
    gfx::Rect valid_bounds = CalculateNonClientFrameValidBounds();
    gfx::Rect title_bounds;
    if (title_bar_)
    {
        title_bounds.SetRect(valid_bounds.x(), valid_bounds.y(), valid_bounds.width(), title_bar_->GetPreferredSize().height());
    }
    return title_bounds;
}

gfx::Rect BiliShadowFrameView::CalculateClientAreaBounds() const
{
    gfx::Rect valid_bounds = CalculateNonClientFrameValidBounds();
    gfx::Rect title_bounds = CalculateTitleBarAreaBounds();
    gfx::Rect client_bounds(valid_bounds.x(), title_bounds.bottom(), valid_bounds.width(), valid_bounds.height() - title_bounds.height());
    return client_bounds;
}

gfx::Insets BiliShadowFrameView::GetShadowInsets() const
{
    // BIG_SHADOW��View��Χ��1�������Ա߿�Ϊȥ���߿��ñ������Ϳͻ�����ס�߿�
    // Ϊʹ�ͻ���View�ߴ粻���ı�����С������ܳߴ�
    gfx::Insets inset;
    if (!show_border_)
    {
        static auto scale = ui::win::GetDeviceScaleFactor();
        // BIG_SHADOW�ײ��ȿͻ������4���ص���Ӱ
        inset = gfx::Insets(1 * scale, 1 * scale, big_shadow_bottom_cy * scale, 1 * scale);
    }
    return inset;
}

gfx::Insets BiliShadowFrameView::GetInsets() const
{
    return gfx::Insets();
}

gfx::Size BiliShadowFrameView::GetPreferredSize()
{
    gfx::Size pref_size = title_bar_->GetPreferredSize();
    const gfx::Size client(GetWidget()->client_view()->GetPreferredSize());
    gfx::Size size(GetUpdatedWindowBounds(gfx::Rect(), client, false).size());
    size.SetToMax(gfx::Size(pref_size.width(), 0));
    size.Enlarge(0, pref_size.height());
    return size;
}



// BililiveShadowFrameView
BililiveShadowFrameView::BililiveShadowFrameView(gfx::ImageSkia *skia, base::string16 caption, int buttons /*= TBB_CLOSE*/)
    : BiliShadowFrameView(new BililiveFrameTitleBarView(skia, caption, buttons))
{
}

BililiveShadowFrameView::BililiveShadowFrameView(int buttons /*= TBB_CLOSE*/)
    : BiliShadowFrameView(new BililiveFrameTitleBarView(&gfx::ImageSkia(), L"", buttons))
{
}

BililiveShadowFrameView::BililiveShadowFrameView(views::View *title_bar)
    : BiliShadowFrameView(title_bar)
{
}
