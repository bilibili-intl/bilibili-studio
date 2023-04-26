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
        Succeeded,   // Ӳ��������Ⱦ�ɹ� ^_^
        NeedRetry,   // �����˴��󣬵��ô����ѱ��޸�����Ҫ���� *_*
        Failed       // û���ˣ��е������Ⱦ x_x
    };

    void CreateHardwareSurface();
    HWRenderResult HardwareRender(gfx::Canvas* canvas);
    void SoftwareRender(gfx::Canvas* canvas);

    void FallbackToSoftwareRender();

    base::win::ScopedComPtr<ID2D1RenderTarget> hw_rt_;
    bool using_hardware_acceleration_ = true;

};
