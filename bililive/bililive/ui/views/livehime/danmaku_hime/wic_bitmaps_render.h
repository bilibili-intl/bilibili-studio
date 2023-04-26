#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_WIC_BITMAPS_RENDER_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_WIC_BITMAPS_RENDER_H_

#include "bililive/bililive/ui/views/livehime/danmaku_hime/d2d_manager.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/wic_bitmaps_read.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/sculptor.h"

/// <summary>
/// 接收由 WIC 组件解码得到的图片或图片序列，
/// 随后外部通过 PresentFrame() 方法和 Render() 方法即可正确地渲染接收的图片或图片序列。
/// </summary>
class WICBitmapsRender {
public:
    /// <summary>构造函数</summary>
    /// <param name="sources">
    /// 由 WIC 组件解码得到的图片或图片序列。
    /// 虽然代码中也会检查，但请确保其中至少含有一张位图。
    /// </param>
    WICBitmapsRender(
        const WICBitmaps& sources, float dpi_x = 96.f, float dpi_y = 96.f);

    /// <summary>
    /// 在进行任何动作之前，请先调用该方法设置渲染目标。
    /// 该渲染目标用于从 WIC 位图创建 D2D 位图，也用于之后的绘制。
    /// 无论调用该方法多少次，创建 D2D 位图的过程只会在当前无任何已创建的 D2D 位图时执行。
    /// </summary>
    /// <param name="rt">渲染目标。不能为空。</param>
    /// <returns>
    /// 若所有从 WIC 位图创建 D2D 位图的操作均失败，返回 false;
    /// 但只要有至少一张 D2D 位图创建成功，返回 true.
    /// </returns>
    bool SetRenderTarget(base::win::ScopedComPtr<ID2D1RenderTarget> rt, bool hw_acc);

    void ResetRenderTargetSource(const WICBitmaps& sources);

    size_t GetCurrentIndex();

    /// <summary>装载下一个还未被绘制的帧，准备绘制。</summary>
/// <returns>若 D2D 位图序列长度小于2或到达 GIF 指定的循环次数，返回 false, 否则返回 true.</returns>
    bool PresentFrame();

    /// <summary>
    /// 根据WICBitmaps 中的宽高创建一个目标设备
    /// </summary>
    void CreateRenderTarget();

    //用来输出bitmap下一帧
    base::win::ScopedComPtr<IWICBitmap> GetNextFrame();

    HRESULT SaveBitmapToPng(PCWSTR uri, ID2D1Bitmap* pBitmap, ID2D1RenderTarget* pRenderTarget);
    /// <summary>
    /// 绘制已装载的帧到指定渲染目标。
    /// </summary>
    void Render(const D2D1_RECT_F& rect);
    void RenderProcess(const D2D1_RECT_F& rect);

    void ReleaseBitmaps();

private:
    void DisposeFrame();
    void DisposeToBackground();
    void DisposeToPrevious();

    void SaveFrame();
    void RestoreFrame();
    void ComposeFrame();

    float GetImageWidth();   // 单位为dip
    float GetImageHeight();  // 单位为dip

    float dpi_x_;
    float dpi_y_;

    int loop_count_;
    size_t frame_index_;
    bool hardware_acceleration_ = false;

    WICBitmaps sources_;
    base::win::ScopedComPtr<ID2D1Bitmap> cur_bitmap_;
    base::win::ScopedComPtr<ID2D1Bitmap> saved_bitmap_;
    std::unique_ptr<Sculptor> sculptor_;
    std::vector<base::win::ScopedComPtr<ID2D1Bitmap>> bitmaps_;

    bool hw_acc_rendered_;
    base::win::ScopedComPtr<ID2D1RenderTarget> rt_;
    base::win::ScopedComPtr<ID2D1BitmapRenderTarget> brt_;

    DISALLOW_COPY_AND_ASSIGN(WICBitmapsRender);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_WIC_BITMAPS_RENDER_H_