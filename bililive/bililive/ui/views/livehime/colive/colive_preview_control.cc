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
        //     * ����������ʱ��������������ҵ��֮��
        //     * ��ǰ������ʱ�ǲ���ʾ���󳡾��ģ�ֻ���ڷ������������ҵ��ʱ��ƥ�䡢���룩ʱ�Ż���ʾ��
        //     * ���Դ�ʱû�б�Ҫ�����󳡾���������������Ҳ�ܹ�ֱܷ��ʸı�������󳡾���С��ͬ�������⡣
        //     */
        //    //livehime::colive::OpenColiveScene();
        //}
    }
}

void ColivePreviewControl::OnDestroy()
{
    // ���Ƴ�obs_preview��obs�ײ��Ǽ����ģ���obs_preview�Ͳ�������Render�е�������scene
    __super::OnDestroy();

    // ���Ƴ�����scene
    //if (preview_type() == obs_proxy_ui::PreviewType::PREVIEW_COLIVE)
    //{
    //    /**
    //     * ��ǰ����ҵ�����ʱ��ر����󳡾����������������ʱ������û�ֱ�ӹرմ��ڣ�
    //     * ����Ҫ�����һ�¡�
    //     */

    //    LOG(INFO) << "ColivePreviewControl::Call CloseColiveScene";
    //    livehime::colive::CloseColiveScene();
    //}
}
