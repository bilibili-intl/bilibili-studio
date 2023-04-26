#pragma once

#include <d2d1.h>
#include "ui/gfx/canvas.h"
#include "ui/views/view.h"


class Sculptor;
struct ID2D1RenderTarget;

class DanmakuBaseRenderView : 
    public views::View
{
public:
    DanmakuBaseRenderView();
    virtual ~DanmakuBaseRenderView();

protected:
    //View
    void OnPaint(gfx::Canvas* canvas) override;
    void OnBoundsChanged(const gfx::Rect& previous_bounds) override;

    virtual void OnDraw(Sculptor& s, bool use_hardware);

    bool UsingHardware();

    ID2D1RenderTarget* GetRenderTarget();

private:
    enum class HWRenderResult {
        Succeeded,   // 硬件加速渲染成功 ^_^
        NeedRetry,   // 出现了错误，但该错误已被修复，需要重试 *_*
        Failed       // 没救了，切到软件渲染 x_x
    };

    void CreateHardwareSurface();
    HWRenderResult HardwareRender(gfx::Canvas* canvas);
    void SoftwareRender(gfx::Canvas* canvas);

    void FallbackToSoftwareRender();

    base::win::ScopedComPtr<ID2D1RenderTarget> hw_rt_;
    bool using_hardware_acceleration_ = true;

};
