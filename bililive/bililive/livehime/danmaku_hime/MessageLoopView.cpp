#include "MessageLoopView.h"

#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "danmakuhime_data_handler.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/text/text_elider.h"
#include "ui/gfx/canvas.h"
#include "skia/include/effects/SkGradientShader.h"


views::Widget* MessageLoopView::MESSAGE_LOOP_WIDGET = nullptr;
MessageLoopView* MessageLoopView::MESSAGE_LOOP_VIEW = nullptr;
static int s_open_view_count = 0;

MessageLoopView::MessageLoopView() :
    linear_animation_{ {30, this },{ 30, this } }
{
    items_[0].process = 0.0f;
    items_[0].is_using = false;
    items_[0].color[0] = SkColorSetARGB(242, 143, 20, 255);
    items_[0].color[1] = SkColorSetARGB(242, 255, 104, 183);
    items_[0].roll_horz = true;

    items_[1].process = 0.0f;
    items_[1].is_using = false;
    items_[1].color[0] = SkColorSetARGB(242, 137, 128, 255);
    items_[1].color[1] = SkColorSetARGB(242, 86, 168, 255);
    items_[1].roll_horz = false;

    auto& rb = ui::ResourceBundle::GetSharedInstance();
    placeholder_image_ = *rb.GetImageSkiaNamed(IDR_LIVEHIME_DANMAKU_CORE_USER_PLACEHOLDER);
    background_image_ = *rb.GetImageSkiaNamed(IDR_LIVEHIME_DANMAKU_CORE_USER_BG);
}

MessageLoopView::~MessageLoopView()
{
	MESSAGE_LOOP_WIDGET = nullptr;
	MESSAGE_LOOP_VIEW = nullptr;
}

void MessageLoopView::OnPaint(gfx::Canvas* canvas)
{
    auto rt = GetContentsBounds();
    auto count = GetItemCount();
    //画背景
    if (rt.height() < (background_image_.height() - GetLengthByDPIScale(30)))
    {
		int half_h = rt.height() / 2;
		canvas->DrawImageInt(background_image_, 0, 0, background_image_.width(), half_h, 0, 0, rt.width(), half_h, false);
		canvas->DrawImageInt(background_image_, 0, background_image_.height() - half_h, background_image_.width(), half_h, 0, half_h, rt.width(), half_h, false);
    }
    else
    {
		canvas->DrawImageInt(background_image_, 0, 0, background_image_.width(), background_image_.height(), 0, 0, rt.width(), rt.height(), false);
    }

    if (count == 0)
    {
        int x = rt.width() / 2 - placeholder_image_.width() / 2;
        int y = GetLengthByDPIScale(16);
        canvas->DrawImageInt(placeholder_image_, x, y);
        y += placeholder_image_.height() + GetLengthByDPIScale(4);
		canvas->DrawStringInt(L"一大批用户正在路上~", ftTwelve, SkColorSetRGB(0x8D, 0x8D, 0x8D),
            0, y,rt.width(), ftTwelve.GetHeight(), gfx::Canvas::TEXT_ALIGN_CENTER);
    }
    else
    {
        rt.Inset(GetLengthByDPIScale(17), GetLengthByDPIScale(16));

        gfx::Rect item_rt;
        item_rt.set_x(rt.x());
        item_rt.set_y(rt.y());
        item_rt.set_width(rt.width());
        item_rt.set_height(item_height_);

        int gay_y = GetLengthByDPIScale(4);
        for (auto& it : items_)
        {
            if (it.is_using)
            {
                DrawItem(canvas, it, item_rt);
                item_rt.set_y(rt.y() + item_height_ + gay_y);
            }
        }
    }
}

gfx::Size MessageLoopView::GetPreferredSize()
{
    auto count = GetItemCount();
    if (count == 2)
    {
        return two_item_pref_size_;
    }
    return one_item_pref_size_;
}

void MessageLoopView::RollItem(int index, const std::wstring& name, const std::wstring& text)
{
    auto& animation = linear_animation_[index];
    auto& item = items_[index];
    if (!animation.is_animating())
    {
        item.roll_name = name;
        item.roll_text = text;
        item.process = 0.0f;
        item.is_using = true;

        auto rt = GetContentsBounds();
        if (item.roll_horz)
        {
            animation.SetDuration(1000 * (rt.width() / 300.0f));
        }
        else
        {
            animation.SetDuration(1000 * (rt.height() / 300.0f));
        }
        animation.Start();

        AdjustItemPos();
    }
}

void MessageLoopView::SetBasePosition(const gfx::Point& pt)
{
    base_pt_ = pt;
}

void MessageLoopView::SetItem(int index, const std::wstring& name, const std::wstring text)
{
    auto& animation = linear_animation_[index];
    auto& item = items_[index];

    if (!animation.is_animating())
    {
        item.name = name;
        item.text = text;
        item.process = 0.0f;
        item.is_using = true;
    }
    AdjustItemPos();
    SchedulePaint();
}

void MessageLoopView::RemoveItem(int index)
{
    items_[index].is_using = false;
    items_[index].name.clear();
    items_[index].text.clear();
    items_[index].roll_name.clear();
    items_[index].roll_text.clear();

    AdjustItemPos();
    SchedulePaint();
}

void MessageLoopView::AnimationEnded(const ui::Animation* animation)
{
    int index = 1;
    if (&linear_animation_[0] == animation)
    {
        index = 0;
    }
    auto& item = items_[index];
    item.name = item.roll_name;
    item.text = item.roll_text;
    item.roll_name = L"";
    item.roll_text = L"";
    item.process = 0.0f;

    SchedulePaint();
}

void MessageLoopView::AnimationProgressed(const ui::Animation* animation)
{
    int index = 1;
    if (&linear_animation_[0] == animation)
    {
        index = 0;
    }
    auto& item = items_[index];
    item.process = animation->GetCurrentValue();
    SchedulePaint();
}

void MessageLoopView::AnimationCanceled(const ui::Animation* animation)
{
    SchedulePaint();
}

void MessageLoopView::OnWidgetDestroyed(views::Widget* widget)
{
    if (MESSAGE_LOOP_WIDGET)
    {
        MESSAGE_LOOP_WIDGET = nullptr;
        MESSAGE_LOOP_VIEW = nullptr;
    }
}

void MessageLoopView::DrawItem(gfx::Canvas* canvas, const Item& item, const gfx::Rect& rect)
{
    SkColor color[2];
    color[0] = item.color[0];
    color[1] = item.color[1];

    SkPoint pts[2];
    pts[0] = { 0.0f, 0.0f };
    pts[1] = { (float)rect.width(), 0.f };

    SkScalar pos[2];
    pos[0] = 0.f;
    pos[1] = 1.f;

    skia::RefPtr<SkShader> s = skia::AdoptRef(SkGradientShader::CreateLinear(
        pts, color, pos, 2, SkShader::kClamp_TileMode, 0));

    SkPaint item_paint;
    item_paint.setAntiAlias(true);
    item_paint.setStyle(SkPaint::kFill_Style);
    item_paint.setShader(s.get());
    canvas->DrawRoundRect(rect, GetLengthByDPIScale(4), item_paint);

    int animation_x{}, animation_y{}, roll_animation_x{}, roll_animation_y{};
    if (item.roll_horz)
    {
        animation_x = item.process * rect.width();
        roll_animation_x = rect.width() - animation_x;
    }
    else
    {
        animation_y = item.process * rect.height();
        roll_animation_y = rect.height() - animation_y;
    }

    DrawItemText(canvas, item.name, item.text, rect, -animation_x, -animation_y);

    DrawItemText(canvas, item.roll_name, item.roll_text, rect, roll_animation_x, roll_animation_y);
}

void MessageLoopView::DrawItemText(gfx::Canvas* canvas, const std::wstring& name, const std::wstring& text, const gfx::Rect& rect, int offset_x, int offset_y)
{
    int gap_x = GetLengthByDPIScale(6);
    int gap_y = GetLengthByDPIScale(8);
    int name_gap = GetLengthByDPIScale(2);
    int line_gap = GetLengthByDPIScale(4);

    canvas->Save();
    canvas->ClipRect(rect);

    int draw_width = rect.width() - gap_x * 2;
    int draw_left = rect.x() + gap_x + offset_x;
    int draw_top = rect.y() + gap_y + offset_y;

    std::wstring name_text1, name_text2;
    bool is_wrap = SplitString(name, ftTwelveBold, draw_width, name_text1, name_text2);
    if (is_wrap)
    {
		canvas->DrawStringInt(name_text1, ftTwelveBold, SkColorSetRGB(255, 255, 0),
            draw_left, draw_top, draw_width, ftTwelveBold.GetHeight());
		canvas->DrawStringInt(name_text2, ftTwelveBold, SkColorSetRGB(255, 255, 0),
            draw_left, draw_top + line_gap + ftTwelveBold.GetHeight(), draw_width, ftTwelveBold.GetHeight());

        int name_width = gfx::Canvas::GetStringWidth(name_text2, ftTwelveBold);
        int content_width = draw_width - name_width - name_gap;
        auto content_text = ui::ElideText(text, ftTwelve, content_width, ui::ELIDE_AT_END);
        canvas->DrawStringInt(content_text, ftTwelve, SkColorSetRGB(255, 255, 255),
            draw_left + name_gap + name_width, draw_top + line_gap + ftTwelveBold.GetHeight(), content_width, ftTwelve.GetHeight());
    }
    else
    {
        canvas->DrawStringInt(name, ftTwelveBold, SkColorSetRGB(255, 255, 0),
            draw_left, draw_top, draw_width, ftTwelveBold.GetHeight());

        int name_width = gfx::Canvas::GetStringWidth(name, ftTwelveBold);
        int content_width = draw_width - name_width - name_gap;
		std::wstring content_text1, content_text2;
        if (SplitString(text, ftTwelve, content_width, content_text1, content_text2))
        {
            canvas->DrawStringInt(content_text1, ftTwelve, SkColorSetRGB(255, 255, 255),
                draw_left + name_gap + name_width, draw_top, content_width, ftTwelve.GetHeight());
            auto content_text = ui::ElideText(content_text2, ftTwelve, draw_width, ui::ELIDE_AT_END);
	        canvas->DrawStringInt(content_text, ftTwelve, SkColorSetRGB(255, 255, 255),
                draw_left, draw_top + line_gap + ftTwelve.GetHeight(), draw_width, ftTwelve.GetHeight());
        }
        else
        {
            canvas->DrawStringInt(text, ftTwelve, SkColorSetRGB(255, 255, 255),
                draw_left + name_gap + name_width, draw_top, content_width, ftTwelve.GetHeight());
        }
    }
    canvas->Restore();
}

void MessageLoopView::AdjustItemPos()
{
    auto size = GetPreferredSize();

    gfx::Rect rt;
    rt.set_x(base_pt_.x() - size.width() + GetLengthByDPIScale(5));
    rt.set_y(base_pt_.y() - size.height() + GetLengthByDPIScale(5));
    rt.set_size(size);

    GetWidget()->SetBounds(rt);
}

int MessageLoopView::GetItemCount()
{
    int count{};
    for (auto& it : items_)
    {
        if (it.is_using)
        {
            count++;
        }
    }

    return count;
}

bool MessageLoopView::SplitString(const std::wstring& text, const gfx::Font& font, int length, std::wstring& text1, std::wstring& text2)
{
    auto char_width = font.platform_font()->GetFontSize();
    if (length < char_width)
    {
        text2 = text;
        return true;
    }
    if (gfx::Canvas::GetStringWidth(text, font) < length)
    {
        return false;
    }
    size_t text_size = text.size();
    for (size_t size = text_size - 1; size > 0; size--)
    {
        std::wstring temp = text.substr(0, size);
        if (gfx::Canvas::GetStringWidth(temp, font) < length)
        {
            text1 = temp;
            text2 = text.substr(size, text_size - size);
            return true;
        }
    }
    return false;
}

void MessageLoopView::ShowWindow(int x, int y, gfx::NativeView parent_view)
{
	if (!MESSAGE_LOOP_WIDGET)
	{
		MESSAGE_LOOP_WIDGET = new views::Widget();

		MESSAGE_LOOP_VIEW = new MessageLoopView();

		auto size = MESSAGE_LOOP_VIEW->GetPreferredSize();

		BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(MESSAGE_LOOP_WIDGET);
		native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_NONE);

		views::Widget::InitParams params(views::Widget::InitParams::TYPE_POPUP);
		params.bounds = { x, y, size.width(), size.height() };
		params.native_widget = native_widget;
		params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;
		params.parent = parent_view;
		params.remove_taskbar_icon = true;
		params.remove_standard_frame = true;

		MESSAGE_LOOP_WIDGET->set_frame_type(views::Widget::FRAME_TYPE_FORCE_NATIVE);
		MESSAGE_LOOP_VIEW->SetBasePosition({ x, y });

		BililiveWidgetDelegate::ShowWidget(MESSAGE_LOOP_VIEW, MESSAGE_LOOP_WIDGET, params, false);
	}

	if (MESSAGE_LOOP_VIEW && MESSAGE_LOOP_WIDGET)
	{
		if (IsWindowHide())
		{
            s_open_view_count++;
		}
        MoveWindow(x,y);
        MESSAGE_LOOP_VIEW->SetVisible(true);
		MESSAGE_LOOP_WIDGET->Show();
	}
}

void MessageLoopView::HideWindow()
{
    if (MESSAGE_LOOP_WIDGET && MESSAGE_LOOP_VIEW)
    {
        MESSAGE_LOOP_WIDGET->Hide();
        MESSAGE_LOOP_VIEW->SetVisible(false);//主窗口最小化时，Hide会无效，这里再次使用SetVisible(false)
    }
}

bool MessageLoopView::IsWindowHide()
{
    if (MESSAGE_LOOP_WIDGET && MESSAGE_LOOP_VIEW)
    {
        return !MESSAGE_LOOP_WIDGET->IsVisible() || !MESSAGE_LOOP_VIEW->visible();
    }
    return true;
}

void MessageLoopView::MoveWindow(int x, int y)
{
    if (MESSAGE_LOOP_VIEW)
    {
        MESSAGE_LOOP_VIEW->SetBasePosition({ x, y });
        MESSAGE_LOOP_VIEW->AdjustItemPos();
        MESSAGE_LOOP_VIEW->SchedulePaint();
    }
}

void MessageLoopView::SetParent(gfx::NativeView parent_view)
{
    if (MESSAGE_LOOP_WIDGET)
    {
        auto native_view = MESSAGE_LOOP_WIDGET->GetNativeView();
        MESSAGE_LOOP_WIDGET->ReparentNativeView(native_view, parent_view);
    }
}

void MessageLoopView::CloseWindow()
{
    if (MESSAGE_LOOP_WIDGET)
    {
        MESSAGE_LOOP_WIDGET->Close();
    }
}

MessageLoopView* MessageLoopView::GetView()
{
    return MESSAGE_LOOP_VIEW;
}
