#ifndef BILILIVE_BILILIVE_UTILS_BILILIVE_GDIPLUS_IMAGE_H_
#define BILILIVE_BILILIVE_UTILS_BILILIVE_GDIPLUS_IMAGE_H_

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


class GdiplusImage
{
public:
    GdiplusImage();
    virtual ~GdiplusImage();

    // 加载图片
    bool LoadFromFile(LPCTSTR szPath);
    bool LoadFromResource(HINSTANCE hInst, UINT uID, LPCTSTR szRscType = RT_BITMAP);
    bool LoadFromBuffer(LPVOID pBuffer, UINT nLen);

    bool IsValid() const { return m_pImage ? true : false; }
    int GetWidth();
    int GetHeight();
    // 获取图片总帧数
    long GetNumFrames() const { return m_lFrameCount; }
    // GIF时获取图片帧数间隔
    DWORD GetFrameDelay() const;
    // 将GIF指定到指定帧
    GdiplusImage* GetFrame(long nFrame);

    Gdiplus::Image* image() const { return m_pImage; }

    operator Gdiplus::Image*() const
    {
        return m_pImage;
    }

protected:
    void DeleteImage();
    void GetImageAttributes();

private:
    Gdiplus::Image *m_pImage;
    //Gdiplus::Rect m_rtInitValid;	// 图片的初始有效尺寸，不因图片的旋转而更改
    HGLOBAL m_hGlobal;	// 从缓冲区加载存储数据的全局内存句柄
    long m_lFrameCount;	// 图片总帧数
    long m_lCurFrame;	// 当前显示的帧
    long *m_pFrameTimes;// 帧数时间间隔数组

    DISALLOW_COPY_AND_ASSIGN(GdiplusImage);
};

#endif