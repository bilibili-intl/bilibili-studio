#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_WIC_BITMAPS_READ_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_WIC_BITMAPS_READ_H_

#include <vector>

#include <d2d1.h>
#include <wincodec.h>

#include "base/memory/singleton.h"
#include "base/strings/string16.h"
#include "base/win/scoped_comptr.h"


struct WICFrame {
    int left;
    int top;
    int width;
    int height;
    int disposal;
    bool interlace;
    bool sort;
    int frame_interval;
    base::win::ScopedComPtr<IWICBitmapSource> bitmap;

    enum DisposalMethods {
        UNSPECIFIED = 0,
        NONE,
        BACKGROUND,
        PREVIOUS
    };

    WICFrame()
        :left(0), top(0),
        width(0), height(0), disposal(0),
        interlace(false), sort(false),
        frame_interval(0) {}
};

struct WICBitmaps {
    int width;
    int height;
    int loop_count;
    D2D1_COLOR_F bg_color;
    std::vector<WICFrame> frames;
    //不为空开启内存缓存优化(如:头衔),默认为空不开启
    std::string source_address;
    WICBitmaps()
        :width(0), height(0), loop_count(0),
        bg_color(D2D1::ColorF(0, 0)), source_address("") {}

    bool empty() const {
        return frames.empty();
    }

    base::win::ScopedComPtr<IWICBitmapSource> front() {
        return frames.front().bitmap;
    }
};

//支持BYTE流 gif的解析
class WICBitmapsRead {
public:
    static WICBitmapsRead* GetInstance();

    base::win::ScopedComPtr<IWICBitmap> CreateWICBitmap(int width, int height);
    base::win::ScopedComPtr<IWICBitmap> CreateWICBitmap(unsigned int width,
        unsigned int height,
        unsigned int stride,
        unsigned int bufferSize,
        BYTE* data);

    WICBitmaps DecodeImage(const string16& file_name);
    WICBitmaps DecodeImage(const BYTE* buffer, size_t size);
    WICBitmaps DecodeImageWebp(const BYTE* buffer, size_t size);

private:
    WICBitmapsRead();

    void GetGlobalMetadata(
        base::win::ScopedComPtr<IWICBitmapDecoder> decoder,
        WICBitmaps& bmps);

    void GetFrameMetadata(
        base::win::ScopedComPtr<IWICBitmapFrameDecode> decoder,
        WICFrame& frame);

    WICBitmaps ProcessDecoder(
        base::win::ScopedComPtr<IWICBitmapDecoder> decoder);

    base::win::ScopedComPtr<IWICImagingFactory> wic_factory_;

    friend struct DefaultSingletonTraits<WICBitmapsRead>;
    DISALLOW_COPY_AND_ASSIGN(WICBitmapsRead);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_WIC_MANAGER_H_