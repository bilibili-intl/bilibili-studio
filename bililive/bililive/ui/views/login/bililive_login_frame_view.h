#ifndef BILILIVE_SECRET_UI_VIEWS_LOGIN_LOGIN_FRAME_VIEW_H
#define BILILIVE_SECRET_UI_VIEWS_LOGIN_LOGIN_FRAME_VIEW_H

#include "ui/gfx/path.h"
#include "ui/views/window/non_client_view.h"

#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"


class BililiveLoginMainView;

class BililiveLoginFrameView
    : public BililiveFrameView {
public:
    explicit BililiveLoginFrameView(BililiveLoginMainView *login_view);

protected:
    gfx::Rect GetBoundsForClientView() const override {
        return gfx::Rect(0, 0, width(), height());
    }

    gfx::Rect GetWindowBoundsForClientBounds(
        const gfx::Rect &client_bounds) const override {
        return client_bounds;
    }

    int NonClientHitTest(const gfx::Point &point) override;

    void GetWindowMask(const gfx::Size &size, gfx::Path *window_mask) override {
        window_mask->moveTo(0, 0);
        window_mask->lineTo(SkIntToScalar(size.width()), 0);
        window_mask->lineTo(SkIntToScalar(size.width()), SkIntToScalar(size.height()));
        window_mask->lineTo(0, SkIntToScalar(size.height()));
        window_mask->lineTo(0, 0);
        window_mask->close();

        return;
    }

    void ResetWindowControls() override {}

    void UpdateWindowIcon() override {}

    void UpdateWindowTitle() override {}

private:
    BililiveLoginMainView *login_view_;
};

#endif  //BILILIVE_SECRET_UI_VIEWS_LOGIN_LOGIN_FRAME_VIEW_H