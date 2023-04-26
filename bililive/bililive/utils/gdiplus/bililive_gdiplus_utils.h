#ifndef BILILIVE_BILILIVE_UTILS_BILILIVE_GDIPLUS_UTILS_H_
#define BILILIVE_BILILIVE_UTILS_BILILIVE_GDIPLUS_UTILS_H_

#include <vector>

#include "base\strings\string16.h"

#include "ui\gfx\font.h"

#include "third_party\skia\include\core\SkColor.h"

#include <comdef.h>

#ifndef min
#define min(x,y) ((x)>(y)?(x):(y))
#endif

#ifndef max
#define max(x,y) ((x)<(y)?(x):(y))
#endif

#include <gdiplus.h>
#undef min
#undef max

#pragma comment(lib, "gdiplus.lib")

#include "bililive_gdiplus_image.h"

namespace bililive_gdiplus
{
    extern const Gdiplus::Graphics *graphics_nullptr;

    Gdiplus::Color ConvertColor(SkColor color);

    std::vector<base::string16> SplitStringInFixedWidth(
        const base::string16& text, const Gdiplus::Font& font,
        int fix_w, int flags = 0, int allway_fix = -1);

    int GetStringWidth(
        Gdiplus::Graphics &graphics, const base::string16& text, const Gdiplus::Font& font, const Gdiplus::StringFormat &format);

    void SizeString(
        Gdiplus::Graphics &graphics, const base::string16& text, const Gdiplus::Font& font,
        Gdiplus::StringFormat &format, int* width, int* height);

    void DrawRoundRect(
        Gdiplus::Graphics &graphics, SkColor color,
        int x, int y, int width, int height, int radius_x, int radius_y);

    void FillRoundRect(
        Gdiplus::Graphics &graphics, SkColor color,
        int x, int y, int width, int height, int radius_x, int radius_y);

    void DrawString(
        Gdiplus::Graphics &graphics, const base::string16& text, const Gdiplus::Font& font,
        float x, float y, float width, float height, Gdiplus::Brush &brush, Gdiplus::StringFormat &format);
}

class CGdiplusObject
{
public:
    CGdiplusObject()
    {
        m_Status = Gdiplus::GdiplusStartup(&m_pGdiToken, &m_gdiplusStartupInput, NULL);
    }

    ~CGdiplusObject()
    {
        if (Gdiplus::Ok == m_Status)
        {
            Gdiplus::GdiplusShutdown(m_pGdiToken);
        }
    }

    Gdiplus::Status StartupStatus(){ return m_Status; }

private:
    Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
    ULONG_PTR m_pGdiToken;
    Gdiplus::Status m_Status;

    DISALLOW_COPY_AND_ASSIGN(CGdiplusObject);
};


class GdiplusCanvas
{
public:
    explicit GdiplusCanvas(HDC hdc);
    explicit GdiplusCanvas(Gdiplus::Bitmap *bitmap);
    virtual ~GdiplusCanvas(void){}

    static int GetStringWidth(const base::string16& text, const Gdiplus::Font& font, int flags = 0);
    static void SizeStringInt(const base::string16& text, const Gdiplus::Font& font, int* width, int* height, int line_height, int flags = 0);

    void DrawStringInt(const base::string16& text, const Gdiplus::Font& font, SkColor color, float x, float y, float w, float h, int flags = 0);
    void DrawRectangle(const Gdiplus::Pen *pen, float x, float y, float w, float h);
    void DrawRoundRectange(SkColor pens, int x, int y, int width, int height, int radius);
    void DrawImage(GdiplusImage *image, int x, int y, int width, int height, bool srccopy = true);

    Gdiplus::Graphics& graphics(){ return graphics_; }

private:
    void SetGraphicsAttributes();

private:
    Gdiplus::Graphics graphics_;

    DISALLOW_COPY_AND_ASSIGN(GdiplusCanvas);
};

#endif