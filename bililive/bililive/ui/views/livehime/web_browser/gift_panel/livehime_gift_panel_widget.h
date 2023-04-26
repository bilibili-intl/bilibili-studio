#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_GIFT_PANEL_BROWSER_WIDGET_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_GIFT_PANEL_BROWSER_WIDGET_H_

#include "base/time/time.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/button/image_button.h"

#include "bililive/bililive/livehime/cef/cef_callback_data_presenter.h"
#include "bililive/bililive/livehime/hybrid_ui/hybrid_ui.h"
#include "bililive/bililive/ui/views/controls/browser/livehime_web_browser_view.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/controls/browser/livehime_web_browser_view.h"
#include "bililive/bililive/ui/views/livehime/web_browser/gift_panel/gift_panel_title.h"

/****ֱ������������������Ⱦ���Ժ�ͱ�����������ˣ��鷳*****/

class GiftPanelTitleView;
class LivehimeListStateBannerView;
class LivehimeWebBrowserViewOSR;

namespace {
    enum class CefType {
        Full_Screen = 0,        //H5ȫ����H5������ر�ҵ��
        Close_Title,            //�ɰ�--��ʾ�رհ�ť����ʽ���ɶ���
        Custom_Title,           //ͨ�ñ����� icon|����|�Զ���|����|����|�ر�|����ɫ H5�Զ������������
        Transparent_Title,      //H5ȫ����͸��������(����������ɫ�̶�) icon|����|�Զ���|����|����|�ر�
                                //H5�Զ������������(H5���Զ���رհ�ťicon)
        Off_Screen              //������Ⱦ���ݲ�֧�����ġ������ַ�����ϼ���
    };
}

class BililiveGiftPanelBrowserView
    : public BililiveWidgetDelegate,
    public LivehimeWebBrowserDelegate,
    GiftPanelTitleDelegate
{
public:
    explicit BililiveGiftPanelBrowserView(const hybrid_ui::LivehimeWebUrlDetails& url_ex,
        WebViewPopupType allow_popup,
        const base::string16& title = L"",
        cef_proxy::client_handler_type browser_type = cef_proxy::client_handler_type::bilibili,
        const std::string& post_data = "",
        LivehimeWebBrowserDelegate* parent_web_delegate = nullptr);
    ~BililiveGiftPanelBrowserView();

    static void ShowWindow(views::Widget* parent,
        const std::string& web_url,
        const string16& title = {},
        WebViewPopupType allow_popup = WebViewPopupType::NotAllow,
        cef_proxy::client_handler_type browser_type = cef_proxy::client_handler_type::bilibili,
        const std::string& post_data = "",
        LivehimeWebBrowserDelegate* parent_web_delegate = nullptr,
        bool window_stack = false);

    int NonClientHitTest(const gfx::Point& point);

    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;
protected:
    // WidgetDelegate
    ui::ModalType GetModalType() const { return ui::MODAL_TYPE_WINDOW; }
    views::NonClientFrameView* CreateNonClientFrameView(views::Widget* widget) override;
    views::View* GetContentsView() override { return this; }
    string16 GetWindowTitle() const override;
    bool ShouldShowWindowIcon() const override { return true; }

    gfx::Size GetPreferredSize() override;
    void Layout() override;

    void OnButtonPressed(CommonType type) override;

    void OnWidgetDestroyed(views::Widget* widget) override;

    // LivehimeWebBrowserDelegate
    void OnWebBrowserCoreInvalid() override;
    void OnWebBrowserLoadStart(const std::string& url) override;
    void OnWebBrowserLoadEnd(const std::string& url, int http_status_code) override;
    void OnWebBrowserLoadError(const std::string& failed_url, int http_status_code,
        const std::string& error_text) override;
    void OnWebBrowserLoadTitle(const base::string16& title) override;
    WebViewPopupType OnWebBrowserPopup(const std::string& url, int target_disposition) override;
    bool OnWebBrowserMsgReceived(const std::string& msg_name, const cef_proxy::calldata* data)override;

    bool OnWebBrowserPreKeyEvent(const cef_proxy::key_event_t& key_event) override;
    bool OnWebBrowserKeyEvent(const cef_proxy::key_event_t& key_event) override;
    void OnWebBrowserGeneralEvent(const cef_proxy::browser_bind_data& bind_data, const std::string& ipc_message, const cef_proxy::calldata* data) override;

private:
    void InitView();
    void InitCefType();
    void InitCefRect();

    void GeneralMsgBoxCallBack(const cef_proxy::browser_bind_data& bind_data, const std::string& ipc_message, const CefAlert& cef_alert,
        const std::pair<base::string16, base::string16>&, void* data);
    void SendTitleBtnJsbridge(const base::string16& btn_name);

    std::string GetPageUrl(const std::string& page_url);

private:
    WebViewPopupType allow_popup_ = WebViewPopupType::NotAllow;
    hybrid_ui::LivehimeWebUrlDetails web_url_ex_;
    cef_proxy::client_handler_type browser_type_ = cef_proxy::client_handler_type::bilibili;
    std::string post_data_;
    LivehimeWebBrowserDelegate* parent_web_delegate_ = nullptr;
    CefType cef_type_;
    int browser_width_ = 0;
    int browser_height_ = 0;
    base::string16 init_title_;

    CefTitleBar cef_title_bar_;
    cef_proxy::browser_bind_data title_bind_data_;

    std::map<std::string, base::Time> url_loading_maps_;
    base::Time webview_loading_start_time_;

    bool is_secondary_window_ = false;
    GiftPanelTitleView* gift_title_ = nullptr;
    LivehimeWebBrowserView* web_browser_view_ = nullptr;
    LivehimeWebBrowserViewOSR* offscreen_browser_view_ = nullptr;
    LivehimeListStateBannerView* state_view_ = nullptr;

    base::WeakPtrFactory<BililiveGiftPanelBrowserView> weak_wpf_;

};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_UNITE_BROWSER_WIDGET_H_