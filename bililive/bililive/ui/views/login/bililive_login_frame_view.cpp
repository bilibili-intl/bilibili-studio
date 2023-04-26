#include "bililive/bililive/ui/views/login/bililive_login_frame_view.h"
#include "bililive/bililive/ui/views/login/bililive_login_main_view.h"

BililiveLoginFrameView::BililiveLoginFrameView(BililiveLoginMainView *login_view)
    : login_view_(login_view) {

}

int BililiveLoginFrameView::NonClientHitTest(const gfx::Point &point) {
    return login_view_->NonClientHitTest(point);
}
