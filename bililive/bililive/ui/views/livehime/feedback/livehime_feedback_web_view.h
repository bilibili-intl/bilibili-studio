#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_FEEDBACK_WEB_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_FEEDBACK_WEB_VIEW_H_

#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_widget.h"



class LivehimeFeedbackWebView :
    public LivehimeHybridWebBrowserView
{
public:
    static void ShowWindow(views::Widget *parent, const std::string& web_url);

protected:
    LivehimeFeedbackWebView(const hybrid_ui::LivehimeWebUrlDetails& url_ex, bool allow_popup);
    ~LivehimeFeedbackWebView();

    // WidgetDelegate
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_NONE; }
    views::NonClientFrameView* CreateNonClientFrameView(views::Widget* widget) override;

    // LivehimeWebBrowserDelegate
    WebViewPopupType OnWebBrowserPopup(const std::string& url, int target_disposition) override;

private:
    static void ShowWindow(views::Widget *parent, const std::string& web_url, 
        bool can_resize, bool can_popup);

private:
    DISALLOW_COPY_AND_ASSIGN(LivehimeFeedbackWebView);
};

#endif