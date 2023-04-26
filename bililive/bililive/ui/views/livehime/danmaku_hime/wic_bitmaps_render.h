#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_WIC_BITMAPS_RENDER_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_WIC_BITMAPS_RENDER_H_

#include "bililive/bililive/ui/views/livehime/danmaku_hime/d2d_manager.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/wic_bitmaps_read.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/sculptor.h"

/// <summary>
/// ������ WIC �������õ���ͼƬ��ͼƬ���У�
/// ����ⲿͨ�� PresentFrame() ������ Render() ����������ȷ����Ⱦ���յ�ͼƬ��ͼƬ���С�
/// </summary>
class WICBitmapsRender {
public:
    /// <summary>���캯��</summary>
    /// <param name="sources">
    /// �� WIC �������õ���ͼƬ��ͼƬ���С�
    /// ��Ȼ������Ҳ���飬����ȷ���������ٺ���һ��λͼ��
    /// </param>
    WICBitmapsRender(
        const WICBitmaps& sources, float dpi_x = 96.f, float dpi_y = 96.f);

    /// <summary>
    /// �ڽ����κζ���֮ǰ�����ȵ��ø÷���������ȾĿ�ꡣ
    /// ����ȾĿ�����ڴ� WIC λͼ���� D2D λͼ��Ҳ����֮��Ļ��ơ�
    /// ���۵��ø÷������ٴΣ����� D2D λͼ�Ĺ���ֻ���ڵ�ǰ���κ��Ѵ����� D2D λͼʱִ�С�
    /// </summary>
    /// <param name="rt">��ȾĿ�ꡣ����Ϊ�ա�</param>
    /// <returns>
    /// �����д� WIC λͼ���� D2D λͼ�Ĳ�����ʧ�ܣ����� false;
    /// ��ֻҪ������һ�� D2D λͼ�����ɹ������� true.
    /// </returns>
    bool SetRenderTarget(base::win::ScopedComPtr<ID2D1RenderTarget> rt, bool hw_acc);

    void ResetRenderTargetSource(const WICBitmaps& sources);

    size_t GetCurrentIndex();

    /// <summary>װ����һ����δ�����Ƶ�֡��׼�����ơ�</summary>
/// <returns>�� D2D λͼ���г���С��2�򵽴� GIF ָ����ѭ������������ false, ���򷵻� true.</returns>
    bool PresentFrame();

    /// <summary>
    /// ����WICBitmaps �еĿ�ߴ���һ��Ŀ���豸
    /// </summary>
    void CreateRenderTarget();

    //�������bitmap��һ֡
    base::win::ScopedComPtr<IWICBitmap> GetNextFrame();

    HRESULT SaveBitmapToPng(PCWSTR uri, ID2D1Bitmap* pBitmap, ID2D1RenderTarget* pRenderTarget);
    /// <summary>
    /// ������װ�ص�֡��ָ����ȾĿ�ꡣ
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

    float GetImageWidth();   // ��λΪdip
    float GetImageHeight();  // ��λΪdip

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