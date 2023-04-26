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

    // ����ͼƬ
    bool LoadFromFile(LPCTSTR szPath);
    bool LoadFromResource(HINSTANCE hInst, UINT uID, LPCTSTR szRscType = RT_BITMAP);
    bool LoadFromBuffer(LPVOID pBuffer, UINT nLen);

    bool IsValid() const { return m_pImage ? true : false; }
    int GetWidth();
    int GetHeight();
    // ��ȡͼƬ��֡��
    long GetNumFrames() const { return m_lFrameCount; }
    // GIFʱ��ȡͼƬ֡�����
    DWORD GetFrameDelay() const;
    // ��GIFָ����ָ��֡
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
    //Gdiplus::Rect m_rtInitValid;	// ͼƬ�ĳ�ʼ��Ч�ߴ磬����ͼƬ����ת������
    HGLOBAL m_hGlobal;	// �ӻ��������ش洢���ݵ�ȫ���ڴ���
    long m_lFrameCount;	// ͼƬ��֡��
    long m_lCurFrame;	// ��ǰ��ʾ��֡
    long *m_pFrameTimes;// ֡��ʱ��������

    DISALLOW_COPY_AND_ASSIGN(GdiplusImage);
};

#endif