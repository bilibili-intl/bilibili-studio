#include "bililive_gdiplus_image.h"

//using namespace Gdiplus;


GdiplusImage::GdiplusImage(void)
    : m_pImage(NULL)
    , m_hGlobal(NULL)
    , m_lFrameCount(1)
    , m_pFrameTimes(NULL)
    , m_lCurFrame(0)
{
}

GdiplusImage::~GdiplusImage(void)
{
    DeleteImage();
}

void GdiplusImage::DeleteImage()
{
    if (m_pImage)
    {
        delete m_pImage;
        m_pImage = NULL;
    }
    if (m_hGlobal)
    {
        GlobalFree(m_hGlobal);
        m_hGlobal = NULL;
    }
    if (m_pFrameTimes)
    {
        delete[] m_pFrameTimes;
        m_pFrameTimes = 0;
    }
    m_lFrameCount = 1;
    m_lCurFrame = 0;
}

bool GdiplusImage::LoadFromFile(LPCTSTR szPath)
{
    if (!szPath)
    {
        return false;
    }
    DeleteImage();
    m_pImage = Gdiplus::Image::FromFile(szPath);
    if (m_pImage)
    {
        GetImageAttributes();
    }
    return m_pImage ? true : false;
}

bool GdiplusImage::LoadFromResource(HINSTANCE hInst, UINT uID, LPCTSTR szRscType)
{
    DeleteImage();
    HRSRC hRsrc = ::FindResource(hInst, MAKEINTRESOURCE(uID), szRscType); // type
    if (hRsrc)
    {
        // load resource into memory  
        DWORD len = SizeofResource(hInst, hRsrc);
        HGLOBAL hGlobal = LoadResource(hInst, hRsrc);
        if (hGlobal)
        {
            LPVOID pvdata = LockResource(hGlobal);
            if (pvdata)
            {
                LoadFromBuffer(pvdata, len);
            }
            FreeResource(hGlobal);
        }
    }
    return m_pImage ? true : false;
}

bool GdiplusImage::LoadFromBuffer(LPVOID pBuffer, UINT nLen)
{
    if (!pBuffer || 0 == nLen)
    {
        return false;
    }
    DeleteImage();
    //在堆上分配空间
    m_hGlobal = GlobalAlloc(GMEM_MOVEABLE, nLen);
    //锁定堆空间,获取指向堆空间的指针
    LPVOID pvdata = GlobalLock(m_hGlobal);
    memcpy_s(pvdata, nLen, pBuffer, nLen);
    GlobalUnlock(m_hGlobal);
    //在堆中创建流对象
    IStream* datastream = NULL;
    CreateStreamOnHGlobal(m_hGlobal, TRUE, &datastream);
    if (datastream)
    {
        m_pImage = Gdiplus::Image::FromStream(datastream);
        if (m_pImage)
        {
            GetImageAttributes();
        }
        datastream->Release();
    }
    return m_pImage ? true : false;
}

void GdiplusImage::GetImageAttributes()
{
    if (m_pImage)
    {
        //获取总帧数
        GUID pageGuid = Gdiplus::FrameDimensionTime;
        m_lFrameCount = m_pImage->GetFrameCount(&pageGuid);
        // 获取每帧时间间隔
        if (m_lFrameCount > 1)
        {
            int nSize = m_pImage->GetPropertyItemSize(PropertyTagFrameDelay);
            char *pFrameTimesBuffer = new char[nSize];
            Gdiplus::PropertyItem *pItem = (Gdiplus::PropertyItem*)pFrameTimesBuffer;
            m_pImage->GetPropertyItem(PropertyTagFrameDelay, nSize, pItem);
            m_pFrameTimes = new long[m_lFrameCount];
            CopyMemory(m_pFrameTimes, pItem->value, m_lFrameCount * sizeof(LONG));
            delete pFrameTimesBuffer;

            // 获取的时间单位是10微秒，乘以10转换成毫秒
            for (long i = 0; i < m_lFrameCount; i++)
            {
                m_pFrameTimes[i] *= 10;
            }
        }
        else
        {
            m_lFrameCount = 1;
        }
    }
}

int GdiplusImage::GetWidth()
{
    if (IsValid())
    {
        return m_pImage->GetWidth();
    }
    return 0;
}

int GdiplusImage::GetHeight()
{
    if (IsValid())
    {
        return m_pImage->GetHeight();
    }
    return 0;
}

GdiplusImage* GdiplusImage::GetFrame(long nFrame)
{
    if (m_pImage && (nFrame >= 0))
    {
        if ((m_lFrameCount > 1) && (nFrame < m_lFrameCount))
        {
            m_lCurFrame = nFrame;
            GUID pageGuid = Gdiplus::FrameDimensionTime;
            m_pImage->SelectActiveFrame(&pageGuid, nFrame);
        }
        return this;
    }
    return NULL;
}

DWORD GdiplusImage::GetFrameDelay() const
{
    if (m_pFrameTimes)
    {
        return m_pFrameTimes[m_lCurFrame];
    }
    return 0;
}
