#ifndef BILILIVE_BILILIVE_UI_VIEWS_PREVIEW_LIVEHIME_OBS_PREVIEW_CONTROL_H
#define BILILIVE_BILILIVE_UI_VIEWS_PREVIEW_LIVEHIME_OBS_PREVIEW_CONTROL_H

#include "obs/obs_proxy/ui_proxy/obs_preview/obs_preview_controller.h"
#include "obs/obs_proxy/public/proxy/obs_proxy.h"

#include "ui/views/controls/native_control.h"


class LivehimeOBSPreviewControl
    : public views::NativeControl
    , public obs_proxy_ui::OBSPreviewController
{
public:
    // NativeControl
    void SetVisible(bool is_visible) override;

    // OBSPreviewController
    HWND GetNativeHWND() override;
    gfx::Rect GetViewRect() override;
    obs_proxy_ui::OBSPreview* obs_preview() const override { return obs_preview_; }

    obs_proxy_ui::PreviewType preview_type() const { return preview_type_; }

protected:
    explicit LivehimeOBSPreviewControl(obs_proxy_ui::PreviewType preview_type);
    virtual ~LivehimeOBSPreviewControl();

    //override from View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override;
    void OnBoundsChanged(const gfx::Rect& previous_bounds) override;

    // NativeControl
    HWND CreateNativeControl(HWND parent_container) override;
    void OnDestroy() override;
    LRESULT OnNotify(int w_param, LPNMHDR l_param) override { return 0; }
    LRESULT OnCommand(UINT code, int id, HWND source) override { return 0; }

    virtual LRESULT OnMouseRange(UINT message, WPARAM w_param, LPARAM l_param);
    virtual LRESULT OnKeyEvent(UINT message, WPARAM w_param, LPARAM l_param);
    virtual LRESULT OnFocusEvent(UINT message, WPARAM w_param, LPARAM l_param);

    // ÖÐÖ¹Êó±êÍÏ×§Item²Ù×÷
    void CancelDragSceneItem();

private:
    void InitViews();
    void UninitViews();

    LRESULT CALLBACK MessageHandler(
        HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static LRESULT CALLBACK LivehimeObsPreviewWndProc(
        HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    string16 class_name_;
    obs_proxy_ui::PreviewType preview_type_;
    obs_proxy_ui::OBSPreview* obs_preview_;

    DISALLOW_COPY_AND_ASSIGN(LivehimeOBSPreviewControl);
};

#endif