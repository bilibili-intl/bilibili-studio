#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_CAMERA_PROPERTY_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_CAMERA_PROPERTY_VIEW_H_

#include "base/prefs/pref_service.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/livehime/sources_properties/source_camera_property_contract.h"
#include "bililive/bililive/ui/views/controls/navigation_bar.h"
#include "bililive/bililive/ui/views/controls/tab_select_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_tabbed.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_property_base_view.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"

#include "bililive/bililive/ui/views/controls/bililive_floating_view.h"
#include "bililive/bililive/ui/views/controls/tab_bar_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/beauty_base_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_camera_base_settings_view.h"
#include "bililive/bililive/ui/views/livehime/pre_start_live/pre_start_live_source_preview_view.h"
#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/secret/public/event_tracking_service.h"

#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"


class BeautyCameraControlPreviewView;
class BililiveViewWithFloatingScrollbar;
class BeautyContentView;

// …„œÒÕ∑ Ù–‘“≥
class LivehimeCameraSrcPropView :
    public livehime::BasePropertyView,
    public TabBarViewListener
{
public:
    explicit LivehimeCameraSrcPropView(obs_proxy::SceneItem* scene_item, const std::string& tab_type);

    virtual ~LivehimeCameraSrcPropView();

protected:
    // View
    gfx::Size GetPreferredSize() override;

    //TabBarViewListener
    bool TabItemSelect(int index, TabBarView* tab_view) override;

private:
    void InitView() override;
    void UninitView() override;

    void InitData() override;

    bool CheckSetupValid() override;

    bool SaveSetupChange() override;
    void PostSaveSetupChange(obs_proxy::SceneItem* scene_item) override;

    bool Cancel() override;

    gfx::ImageSkia* GetSkiaIcon() override;

    std::wstring GetCaption() override;

    void SelectDefTab(int index);

    //void UpadteUIState(const BeautyMaterialData& material_data, BeautyItemView::UpdateState state, bool new_select);

private:
    obs_proxy::SceneItem*                                       item_ = nullptr;
    int                                                         def_tabindex_ = -1;
    TabBarView*                                                 tab_view_ = nullptr;
    PreStartLiveSourcePreviewView*                              preview_view_ = nullptr;
    BililiveFloatingView*                                       float_view_ = nullptr;
    BeautyCameraControlPreviewView*                             preview_control_view_ = nullptr;

    TabSelectView*                                              content_view_ = nullptr;

    std::string                                                 tab_type_ = "beauty";
    SourceCameraBaseSettingsView*                               common_settings_view_ = nullptr;

    BeautyBaseView*                                             beauty_settings_view_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(LivehimeCameraSrcPropView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_CAMERA_PROPERTY_VIEW_H_
