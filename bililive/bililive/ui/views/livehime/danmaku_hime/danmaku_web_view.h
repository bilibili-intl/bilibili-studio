#pragma once

#include "ui/views/controls/button/button.h"

#include "bililive/bililive/livehime/hybrid_ui/hybrid_ui.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/controls/browser/livehime_web_browser_view.h"

class BililiveImageButton;
class LivehimeListStateBannerView;
class LivehimeTitleLabel;

class DanmakuWebView : 
    public BililiveWidgetDelegate,
    public LivehimeWebBrowserDelegate,
    public views::ButtonListener,
    BililiveNonTitleBarFrameViewDelegate
{
public:
    static const int kCloseButtonID = 1;

    static void ShowWindow(
        views::Widget* parent,
        const std::string& web_url,
        bool show_native_titlebar = false,
        const string16& title = {},
        WebViewPopupType allow_popup = WebViewPopupType::NotAllow,
        cef_proxy::client_handler_type browser_type = cef_proxy::client_handler_type::bilibili,
        const std::string& post_data = "");

    static bool IsWebCreated();

    ~DanmakuWebView();

protected:
    DanmakuWebView(const hybrid_ui::LivehimeWebUrlDetails& url_ex,
        bool show_native_titlebar,
        WebViewPopupType allow_popup,
        cef_proxy::client_handler_type browser_type = cef_proxy::client_handler_type::bilibili,
        const std::string& post_data = "");

    bool allow_popup() const { return allow_popup_ != WebViewPopupType::NotAllow; }
    LivehimeWebBrowserView* web_browser_view() { return web_browser_view_; }
    void SetDefaultPrefSize(const gfx::Size& size) { default_pref_size_ = size; }
    hybrid_ui::LivehimeWebUrlDetails hybrid_details() const { return web_url_ex_; }

    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    gfx::Size GetPreferredSize() override;
    void Layout() override;

    // WidgetDelegate
    views::View* GetContentsView() override { return this; }
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }
    views::NonClientFrameView* CreateNonClientFrameView(views::Widget* widget) override;

    // BililiveNonTitleBarFrameViewDelegate
    int NonClientHitTest(const gfx::Point& point) override;

    // BililiveWidgetDelegate
    void SetWindowTitle(const base::string16& title) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // LivehimeWebBrowserDelegate
    void OnWebBrowserCoreInvalid() override;
    void OnWebBrowserLoadStart(const std::string& url) override;
    void OnWebBrowserLoadEnd(const std::string& url, int http_status_code) override;
    void OnWebBrowserLoadError(const std::string& failed_url, int http_status_code,
        const std::string& error_text) override;
    WebViewPopupType OnWebBrowserPopup(const std::string& url, int target_disposition) override;

    virtual void InitViews();
    void LayoutInBounds(gfx::Rect rect);

private:
    static int is_web_broswer_created_;

    bool show_native_titlebar_ = false;
    bool native_close_always_show_ = false;
    WebViewPopupType allow_popup_ = WebViewPopupType::NotAllow;
    hybrid_ui::LivehimeWebUrlDetails web_url_ex_;
    cef_proxy::client_handler_type browser_type_ = cef_proxy::client_handler_type::bilibili;
    gfx::Size default_pref_size_;
    LivehimeWebBrowserView* web_browser_view_ = nullptr;
    std::string post_data_;

    LivehimeTitleLabel* title_label_ = nullptr;
    BililiveImageButton* close_button_ = nullptr;
    LivehimeListStateBannerView* state_view_ = nullptr;
    

    DISALLOW_COPY_AND_ASSIGN(DanmakuWebView);
};
