#include "bililive/bililive/ui/views/livehime/colive/colive_preview_control.h"

#include "ui/views/widget/widget.h"

//#include "bililive/bililive/livehime/colive/colive_utils.h"
#include "bililive/bililive/ui/bililive_obs.h"

#include "obs/obs_proxy/common/obs_proxy_constants.h"
#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"


ColivePreviewControl::ColivePreviewControl(obs_proxy_ui::PreviewType preview_type)
    : LivehimeOBSPreviewControl(preview_type),
      ColiveSceneItemMenu(this)
{
}

ColivePreviewControl::~ColivePreviewControl() {}


void ColivePreviewControl::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) {
    __super::ViewHierarchyChanged(details);

    if (details.child == this && details.is_add && GetWidget()) {
        DCHECK(GetNativeHWND());

        //if (preview_type() == obs_proxy_ui::PreviewType::PREVIEW_COLIVE)
        //{
        //    /**
        //     * 创建场景的时机放在启动连麦业务之后。
        //     * 当前打开连麦窗时是不显示连麦场景的，只有在发起了连麦相关业务时（匹配、邀请）时才会显示。
        //     * 所以此时没有必要把连麦场景创建出来，这样也能规避分辨率改变造成连麦场景大小不同步的问题。
        //     */
        //    //livehime::colive::OpenColiveScene();
        //}
    }
}

void ColivePreviewControl::OnDestroy()
{
    // 先移除obs_preview（obs底层是加锁的），obs_preview就不会在其Render中调用连麦scene
    __super::OnDestroy();

    // 再移除连麦scene
    //if (preview_type() == obs_proxy_ui::PreviewType::PREVIEW_COLIVE)
    //{
    //    /**
    //     * 当前连麦业务结束时会关闭连麦场景，但在连麦进行中时，如果用户直接关闭窗口，
    //     * 就需要这里关一下。
    //     */

    //    LOG(INFO) << "ColivePreviewControl::Call CloseColiveScene";
    //    livehime::colive::CloseColiveScene();
    //}
}
