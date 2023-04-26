#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_COLIVE_COLIVE_PREVIEW_CONTROL_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_COLIVE_COLIVE_PREVIEW_CONTROL_H_

#include "bililive/bililive/ui/views/menu/colive_scene_item_menu.h"
#include "bililive/bililive/ui/views/preview/livehime_obs_preview_control.h"


class ColivePreviewControl :
    public LivehimeOBSPreviewControl,
    ColiveSceneItemMenu
{
public:
    explicit ColivePreviewControl(obs_proxy_ui::PreviewType preview_type);
    ~ColivePreviewControl();

protected:
    // View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;

    // NativeControl
    void OnDestroy() override;

private:
    DISALLOW_COPY_AND_ASSIGN(ColivePreviewControl);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_COLIVE_COLIVE_PREVIEW_CONTROL_H_