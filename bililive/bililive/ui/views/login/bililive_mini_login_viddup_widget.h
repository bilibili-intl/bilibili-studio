#pragma once

#include "ui/views/controls/button/button.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/label.h"
#include "ui/views/view.h"

#include "bililive/bililive/login/bililive_login_contract.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/controls/browser/livehime_web_browser_view.h"
#include "bililive/secret/public/login_info.h"


class BililiveLabel;
class BililiveImageButton;
class LivehimeListStateBannerView;

class BililiveMiniLoginViddupView
    : public BililiveWidgetDelegate,
    views::ButtonListener,
    LivehimeWebBrowserDelegate
{
public:
    static void DoModal(LoginResultCode* result_code);

    explicit BililiveMiniLoginViddupView(LoginResultCode* result_code);

    ~BililiveMiniLoginViddupView();

    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;

protected:
    // WidgetDelegate
    ui::ModalType GetModalType() const {
        return ui::MODAL_TYPE_WINDOW;
    }
    views::NonClientFrameView* CreateNonClientFrameView(views::Widget* widget) override;
    views::View* GetContentsView() override {
        return this;
    }
    string16 GetWindowTitle() const override;
    bool ShouldShowWindowIcon() const override {
        return true;
    }
    gfx::ImageSkia GetWindowIcon() override;
    void WindowClosing() override;
    bool ShouldHandleSystemCommands() const {
        return true;
    }

    // WidgetObserver
    void OnWidgetBoundsChanged(views::Widget* widget,
        const gfx::Rect& new_bounds) override;

    // View
    gfx::Size GetPreferredSize() override;
    void Layout() override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // LivehimeWebBrowserDelegate
    void OnWebBrowserCoreInvalid() override;
    void OnPreWebBrowserWindowCreate() override;
    void OnWebBrowserLoadStart(const std::string& url) override;
    void OnWebBrowserLoadEnd(const std::string& url, int http_status_code) override;
    void OnWebBrowserLoadError(const std::string& failed_url, int http_status_code,
        const std::string& error_text) override;
    WebViewPopupType OnWebBrowserPopup(const std::string& url, int target_disposition) override;
    bool OnWebBrowserMsgReceived(const std::string& msg_name, const cef_proxy::calldata* data)override;

private:
    void InitViews();
    void InitData();

    void LayoutInBounds(gfx::Rect rect);
    void GetUserInfo();

    void OnGetUserInfo(bool valid_response, int code,
        const secret::UserAccountService::ViddupUserInfoData& info);
    void ShowWebView(bool show);
    void ResetWindowSize(const gfx::Size& size, bool show_titlebar, SkColor bk_clr = SK_ColorWHITE);

    std::string LoginUrlSplit(const std::string& url);

private:
    LoginResultCode* result_code_ = nullptr;

    LivehimeWebBrowserView* web_browser_view_ = nullptr;

    BililiveLabel* title_label_ = nullptr;
    BililiveImageButton* close_button_ = nullptr;
    LivehimeListStateBannerView* state_view_ = nullptr;
    bool show_titlebar_ = false;
    bool nav_by_initdata_ = false;

    bool is_login_success = false;

    base::Time webview_loading_start_time_;

    std::unique_ptr<contracts::BililiveLoginPresenter> presenter_;

    base::WeakPtrFactory<BililiveMiniLoginViddupView> weak_factory_;

    DISALLOW_COPY_AND_ASSIGN(BililiveMiniLoginViddupView);
};
