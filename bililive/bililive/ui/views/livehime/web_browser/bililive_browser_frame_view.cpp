#include "bililive/bililive/ui/views/livehime/web_browser/bililive_browser_frame_view.h"
#include "bililive/bililive/ui/views/livehime/web_browser/livehime_unite_browser_widget.h"

BililiveBrowserFrameView::BililiveBrowserFrameView(BililiveUniteBrowserView* browser_view)
    : browser_view_(browser_view) {

}

int BililiveBrowserFrameView::NonClientHitTest(const gfx::Point &point) {
    return browser_view_->NonClientHitTest(point);
}
