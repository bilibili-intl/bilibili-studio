#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_BROWSER_PROPERTY_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_BROWSER_PROPERTY_VIEW_H_

#include "bililive/bililive/livehime/sources_properties/source_browser_property_contract.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_property_base_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_volume_control_view.h"

class SourceFilterBaseView;
class BrowserPropDetailView;

class BrowserCaptureDetailView : public livehime::BasePropertyView {
public:
    BrowserCaptureDetailView(obs_proxy::SceneItem* scene_item);
    ~BrowserCaptureDetailView();
private:
    void InitView() override;
    bool SaveSetupChange() override;
    void PostSaveSetupChange(obs_proxy::SceneItem* scene_item) override;
    bool Cancel() override;

    gfx::ImageSkia* GetSkiaIcon() override;
    std::wstring GetCaption() override;

    gfx::Size GetPreferredSize() override;
    bool CheckSetupValid() override;
private:
    obs_proxy::SceneItem* scene_item_;
    std::unique_ptr<BrowserPropDetailView> main_view_;
};

class BrowserPropDetailView
    : public livehime::BaseScrollbarView,
    public views::ButtonListener,
    public BilibiliNativeEditController
{
private:
    enum EffectiveControl
    {
        BROWSER_INVALID = -1,
        BROWSER_URL,
        BROWSER_WIDTH,
        BROWSER_HEIGHT,
        BROWSER_CSS,
        BROWSER_REFRESH,
        BROWSER_REFRESH_CHECK
    };

public:
    explicit BrowserPropDetailView(obs_proxy::SceneItem* scene_item);

    virtual ~BrowserPropDetailView();

    void ContentsChanged(BilibiliNativeEditView* sender, const string16& new_contents) override;

    bool PreHandleMSG(BilibiliNativeEditView* sender, UINT msg, WPARAM wParam, LPARAM lParam) override;

    bool SaveSetupChange() override;
    void PostSaveSetupChange(obs_proxy::SceneItem* scene_item) override;

    bool Cancel() override;

    gfx::ImageSkia* GetSkiaIcon() override;

    std::wstring GetCaption() override;

private:
    void InitView() override;

    void InitData() override;

    bool CheckSetupValid() override;

    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    //gfx::Size GetPreferredSize() override;

private:
    void EffectiveImmediately(EffectiveControl effective_control);
private:
    bool data_loaded_;

    obs_proxy::SceneItem* scene_item_;
    SourceFilterBaseView* source_filter_base_view_;

    int64_t plugin_id_ = 0;

    LivehimeNativeEditView* name_edit_;

    LivehimeNativeEditView* url_edit_;

    LivehimeNativeEditView* width_edit_;

    LivehimeNativeEditView* height_edit_;

    LivehimeNativeEditView* css_edit_;

    LivehimeFunctionLabelButton* refresh_button_;

    //LivehimeCheckbox* refresh_check_ = nullptr;

    SouceVolumeControlView* volume_control_view_ = nullptr;

    std::unique_ptr<contracts::SourceBrowserPropertyPresenter> presenter_;

    DISALLOW_COPY_AND_ASSIGN(BrowserPropDetailView);

};

#endif // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_BROWSER_PROPERTY_VIEW_H_