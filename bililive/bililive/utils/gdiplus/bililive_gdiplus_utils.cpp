#include "bililive_gdiplus_utils.h"

#include "ui/gfx/canvas.h"

//using namespace Gdiplus;

namespace bililive_gdiplus
{
    const Gdiplus::Graphics *graphics_nullptr = NULL;

    Gdiplus::Color ConvertColor(SkColor color) {
        return Gdiplus::Color(SkColorGetA(color), SkColorGetR(color), SkColorGetG(color), SkColorGetB(color));
    }

    std::vector<base::string16> SplitStringInFixedWidth(
        const base::string16& text, const Gdiplus::Font& font, int fix_w, int flags, int allway_fix/* = -1*/)
    {
        std::vector<base::string16> vct;
        base::string16 tail;
        int cx = 0;
        base::string16::size_type start = 0;
        for (base::string16::size_type i = 1; i <= text.size(); i++)
        {
            cx = GdiplusCanvas::GetStringWidth(text.substr(start, i - start), font, flags);
            if (cx > fix_w)
            {
                if (i - 1 - start > 0)
                {
                    vct.push_back(text.substr(start, i - 1 - start));
                    start = i - 1;
                }
                if (allway_fix != -1)
                {
                    fix_w = allway_fix;
                }
            }
        }
        tail = text.substr(start, text.size() - start);
        if (!tail.empty())
        {
            vct.push_back(tail);
        }

        return vct;
    }

    int GetStringWidth(
        Gdiplus::Graphics &graphics, const base::string16& text, const Gdiplus::Font& font, const Gdiplus::StringFormat &format) {

        int cx = 0;
        Gdiplus::PointF point;
        Gdiplus::RectF rect;
        if (Gdiplus::Ok == graphics.MeasureString(text.c_str(), text.length(), &font, point, &format, &rect)) {
            cx = std::ceil(rect.Width);
        }

        return cx;
    }

    void SizeString(
        Gdiplus::Graphics &graphics, const base::string16& text, const Gdiplus::Font& font,
        Gdiplus::StringFormat &format, int* width, int* height) {

        Gdiplus::SizeF size;
        graphics.MeasureString(text.c_str(), text.length(), &font, Gdiplus::SizeF(*width, *height), &format, &size);

        *width = std::ceil(size.Width);
        *height = std::ceil(size.Height);
    }

    void DrawRoundRect(
        Gdiplus::Graphics &graphics, SkColor color,
        int x, int y, int width, int height, int radius_x, int radius_y) {

        int elWid = 2 * radius_x;
        int elHei = 2 * radius_y;

        Gdiplus::Pen p(ConvertColor(color));
        Gdiplus::GraphicsPath path;

        path.AddArc(x, y, elWid, elHei, 180, 90); // 左上角圆弧
        path.AddLine(x + radius_x, y, x + width - radius_x, y); // 上边

        path.AddArc(x + width - elWid, y, elWid, elHei, 270, 90); // 右上角圆弧
        path.AddLine(x + width, y + radius_y, x + width, y + height - radius_y);// 右边

        path.AddArc(x + width - elWid, y + height - elHei, elWid, elHei, 0, 90); // 右下角圆弧
        path.AddLine(x + width - radius_x, y + height, x + radius_x, y + height); // 下边

        path.AddArc(x, y + height - elHei, elWid, elHei, 90, 90);
        path.AddLine(x, y + radius_y, x, y + height - radius_y);

        graphics.DrawPath(&p, &path);
    }

    void FillRoundRect(
        Gdiplus::Graphics &graphics, SkColor color,
        int x, int y, int width, int height, int radius_x, int radius_y) {

        int elWid = 2 * radius_x;
        int elHei = 2 * radius_y;

        Gdiplus::GraphicsPath path;
        Gdiplus::SolidBrush brush(ConvertColor(color));

        path.AddArc(x, y, elWid, elHei, 180, 90); // 左上角圆弧
        path.AddLine(x + radius_x, y, x + width - radius_x, y); // 上边

        path.AddArc(x + width - elWid, y, elWid, elHei, 270, 90); // 右上角圆弧
        path.AddLine(x + width, y + radius_y, x + width, y + height - radius_y);// 右边

        path.AddArc(x + width - elWid, y + height - elHei, elWid, elHei, 0, 90); // 右下角圆弧
        path.AddLine(x + width - radius_x, y + height, x + radius_x, y + height); // 下边

        path.AddArc(x, y + height - elHei, elWid, elHei, 90, 90);
        path.AddLine(x, y + radius_y, x, y + height - radius_y);

        graphics.FillPath(&brush, &path);
    }

    void DrawString(
        Gdiplus::Graphics &graphics, const base::string16& text, const Gdiplus::Font& font,
        float x, float y, float width, float height, Gdiplus::Brush &brush, Gdiplus::StringFormat &format) {
        graphics.DrawString(text.c_str(), text.length(), &font, Gdiplus::RectF(x, y, width, height), &format, &brush);
    }
}

int GdiplusCanvas::GetStringWidth(const base::string16& text, const Gdiplus::Font& font, int flags)
{
    int cx = 0;
    HDC hdc = ::GetDC(NULL);
    Gdiplus::Graphics graphics(hdc);

    if (flags & gfx::Canvas::NO_SUBPIXEL_RENDERING) {
        graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
    }
    else {
        graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
    }

    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    Gdiplus::StringFormat stringFormat;
    stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
    stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
    stringFormat.GenericTypographic();
    stringFormat.SetTrimming(Gdiplus::StringTrimmingNone);
    Gdiplus::PointF point;
    Gdiplus::RectF rect;
    if (Gdiplus::Ok == graphics.MeasureString(text.c_str(), text.length(), &font, point, &stringFormat, &rect))
    {
        cx = std::ceil(rect.Width);
    }
    ::ReleaseDC(NULL, hdc);
    return cx;
}

void GdiplusCanvas::SizeStringInt(const base::string16& text, const Gdiplus::Font& font, int* width, int* height, int line_height, int flags)
{
    int max_cx = 0;
    std::vector<base::string16> vct = bililive_gdiplus::SplitStringInFixedWidth(text, font, *width, flags);
    for (std::vector<base::string16>::size_type i = 0; i < vct.size(); i++)
    {
        max_cx = std::max(max_cx, GdiplusCanvas::GetStringWidth(vct[i], font, flags));
    }
    *width = max_cx;
    *height = line_height*vct.size();
}

GdiplusCanvas::GdiplusCanvas(HDC hdc)
    : graphics_(hdc)
{
    SetGraphicsAttributes();
}

GdiplusCanvas::GdiplusCanvas(Gdiplus::Bitmap *bitmap)
    : graphics_(bitmap)
{
    SetGraphicsAttributes();
}

void GdiplusCanvas::SetGraphicsAttributes()
{
    graphics_.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
    graphics_.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    //graphics_.SetInterpolationMode(Gdiplus::InterpolationModeHighQuality);
    //graphics_.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
}

void GdiplusCanvas::DrawStringInt(const base::string16& text, const Gdiplus::Font& font, SkColor color,
    float x, float y, float w, float h, int flags)
{
    auto saved_hint = graphics_.GetTextRenderingHint();

    if (flags & gfx::Canvas::NO_SUBPIXEL_RENDERING) {
        graphics_.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
    }
    else {
        graphics_.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
    }

    Gdiplus::StringFormat stringFormat;
    stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
    stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
    stringFormat.GenericTypographic();
    stringFormat.SetTrimming(Gdiplus::StringTrimmingNone);
    Gdiplus::SolidBrush brush(Gdiplus::Color(SkColorGetR(color), SkColorGetG(color), SkColorGetB(color)));
    graphics_.DrawString(text.c_str(), text.length(), &font,
        Gdiplus::RectF(x, y, w, h),
        &stringFormat, &brush);

    graphics_.SetTextRenderingHint(saved_hint);
}

void GdiplusCanvas::DrawRectangle(const Gdiplus::Pen *pen,
    float x, float y, float w, float h)
{
    graphics_.DrawRectangle(pen, Gdiplus::RectF(x, y, w, h));
}

void GdiplusCanvas::DrawImage(GdiplusImage *image, int x, int y, int width, int height,
    bool srccopy/* = true*/)
{
    if (srccopy)
    {
        x = x + (width - image->GetWidth()) / 2;
        y = y + (height - image->GetHeight()) / 2;
        width = image->GetWidth();
        height = image->GetHeight();
    }
    graphics_.DrawImage((Gdiplus::Image*)image->image(),
        Gdiplus::Rect(x, y, width, height),
        0, 0, image->GetWidth(), image->GetHeight(), Gdiplus::UnitPixel);
}

void GdiplusCanvas::DrawRoundRectange(SkColor pens, int x, int y, int width, int height, int radius)
{
    //创建一个红色的画笔
    Gdiplus::Color clr(SkColorGetA(pens), SkColorGetR(pens), SkColorGetG(pens), SkColorGetB(pens));
    Gdiplus::Pen pen(clr, 1);

    //画矩形上面的边
    graphics_.DrawLine(&pen, x + radius, y, x + width - radius, y);

    //画矩形下面的边
    graphics_.DrawLine(&pen, x + radius, y + height, x + width - radius, y + height);

    //画矩形左面的边
    graphics_.DrawLine(&pen, x, y + radius, x, y + height - radius);

    //画矩形右面的边
    graphics_.DrawLine(&pen, x + width, y + radius, x + width, y + height - radius);

    //画矩形左上角的圆角
    graphics_.DrawArc(&pen, x, y, radius * 2, radius * 2, 180, 90);

    //画矩形右下角的圆角
    graphics_.DrawArc(&pen, x + width - radius * 2, y + height - radius * 2, radius * 2, radius * 2, 360, 90);

    //画矩形右上角的圆角
    graphics_.DrawArc(&pen, x + width - radius * 2, y, radius * 2, radius * 2, 270, 90);

    //画矩形左下角的圆角
    graphics_.DrawArc(&pen, x, y + height - radius * 2, radius * 2, radius * 2, 90, 90);
}