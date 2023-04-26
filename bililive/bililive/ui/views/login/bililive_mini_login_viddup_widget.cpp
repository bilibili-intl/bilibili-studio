#include "bililive/bililive/ui/views/login/bililive_mini_login_viddup_widget.h"

#include "base/command_line.h"
#include "base/ext/callable_callback.h"
#include "base/file_util.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/icon_util.h"
#include "ui/views/focus/accelerator_handler.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/app/bililive_dll_resource.h"
#include "bililive/bililive/login/bililive_login_presenter_impl.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_list_state_banner.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_widget.h"
#include "bililive/bililive/ui/views/login/livehime_login_util.h"
#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/bililive/utils/net_util.h"
#include "bililive/common/bililive_context.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/bililive_paths.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/utils/bililive_crypto.h"
#include "bililive_browser/public/bililive_browser_ipc_messages.h"

#include "cef_proxy_dll/public/livehime_cef_proxy_calldata.h"
#include "bililive/bililive/utils/fast_forward_url_convert.h"

namespace
{
    const int kMinWidth = 517;
    const int kMinHeight = 674;
    const int kWebVerifyWidth = 1020;
    const int kWebVerifyHeight = 830;
    const int kMobileSecValidWidth = 360;
    const int kMobileSecValidHeight = 500;

    // const char kUrlMiniLogin[] = "https://www.bilibili.tv/en/oauth/video-up // use different login page according language
    const char kUrlMiniLoginEN[] = "https://www.bilibili.tv/en/oauth/video-up";
    const char kUrlMiniLoginZH[] = "https://www.bilibili.tv/en/oauth/video-up";
    const char kUrlMiniLoginID[] = "https://www.bilibili.tv/id/oauth/video-up";
    const char kUrlMiniLoginTH[] = "https://www.bilibili.tv/th/oauth/video-up";

    const char kProtocolUrl[] = "https://www.bilibili.tv/marketing/activity-6lyoHX1ZlO.html";
    const char kNetApi_checkUpdate[] = "https://api.biliintl.com/intl/materials/pc/upgrade?version=";
    const char kLogoutUrl[] = "https://passport.bilibili.tv/x/intl/passport-login/web/login/exit";
    const char kAvatarViddupUrl[] = "https://api.bilibili.tv/x/intl/member/web/account?platform=web&s_locale=en_US";//uat is http

    std::string GetMiniLoginUrl()
    {
        static std::string url = CommandLine::ForCurrentProcess()->GetSwitchValueASCII("mini-login-url");
        if (url.empty())
        {
            std::string language = GetBililiveProcess()->GetApplicationLocale();
            if (language == "en-US") {
                return kUrlMiniLoginEN;
            }
            if (language == "zh-CN") {
                return kUrlMiniLoginZH;
            }
            if (language == "th-TH") {
                return kUrlMiniLoginTH;
            }
            if (language == "id-ID") {
                return kUrlMiniLoginID;
            }
            return kUrlMiniLoginEN; // default return en
        }
        return url;
    }

    void QuitMessageLoop();
    class MiniWebDestoryWatcher : CefProxyObserver
    {
    public:
        MiniWebDestoryWatcher(const cef_proxy::browser_bind_data& bind_data)
            : bind_data_(bind_data)
        {
            CefProxyWrapper::GetInstance()->AddObserver(this);
        }

        ~MiniWebDestoryWatcher()
        {
            CefProxyWrapper::GetInstance()->RemoveObserver(this);
        }

    protected:
        // �ض�ģ����ص�֪ͨ
        void OnCefProxyMsgReceived(const cef_proxy::browser_bind_data& bind_data,
            const std::string& msg_name, const cef_proxy::calldata* data) override
        {
            if (bind_data == bind_data_)
            {
                if (msg_name == ipc_messages::kLivehimeCefClientDestoryed)
                {
                    LOG(INFO) << "[login] login client destoryed.";

                    if (base::MessageLoop::current())
                    {
                        base::MessageLoop::current()->PostTask(FROM_HERE, base::Bind(QuitMessageLoop));
                    }
                }
            }
        }

    private:
        cef_proxy::browser_bind_data bind_data_;
    };
    std::unique_ptr<MiniWebDestoryWatcher> g_web_dst_watcher;

    void QuitMessageLoop()
    {
        g_web_dst_watcher.reset();
        base::MessageLoop::current()->Quit();
    }

    cef_proxy::cookies g_cookies;
    bool g_profile_cookies = false;
}

void BililiveMiniLoginViddupView::DoModal(LoginResultCode* result_code)
{
    BililiveMiniLoginViddupView *login_view = new BililiveMiniLoginViddupView(result_code);

    views::Widget *widget = new views::Widget();
    views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
    params.native_widget = new BililiveNativeWidgetWin(widget);
    params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
    BililiveWidgetDelegate::ShowWidget(login_view, widget, params);
    widget->UpdateWindowIcon();

    views::AcceleratorHandler accelerator_handler;
    base::RunLoop run_loop(&accelerator_handler);
    run_loop.Run();
}

BililiveMiniLoginViddupView::BililiveMiniLoginViddupView(LoginResultCode* result_code)
    : result_code_(result_code),
    weak_factory_(this)
{
    presenter_ = std::make_unique<BililiveLoginPresenterImpl>(nullptr);
}

BililiveMiniLoginViddupView::~BililiveMiniLoginViddupView() {
}

// WidgetDelegate
string16 BililiveMiniLoginViddupView::GetWindowTitle() const {
    return ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_LOGIN_TITLE);
}

gfx::ImageSkia BililiveMiniLoginViddupView::GetWindowIcon() {
    scoped_ptr<SkBitmap> bitmap = IconUtil::CreateSkBitmapFromIconResource(
        GetModuleHandle(bililive::kBililiveResourcesDll), IDR_MAINFRAME, 0);
    return gfx::ImageSkia(gfx::ImageSkiaRep(*bitmap, ui::GetScaleFactorFromScale(ui::win::GetDeviceScaleFactor())));
}

views::NonClientFrameView *BililiveMiniLoginViddupView::CreateNonClientFrameView(views::Widget* widget) {
    return new BililiveNonTitleBarFrameView(nullptr);
}

void BililiveMiniLoginViddupView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) {
    if (details.is_add && details.child == this) {
        InitViews();
    }
}

void BililiveMiniLoginViddupView::InitViews()
{
    title_label_ = new LivehimeTitleLabel(GetWindowTitle());
    title_label_->SetAutoColorReadabilityEnabled(true);
    title_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);

    close_button_ = new BililiveImageButton(this);
    close_button_->SetImage(views::Button::STATE_NORMAL, GetImageSkiaNamed(IDR_LIVEMAIN_JOINMIC_CLOSE));
    close_button_->SetImage(views::Button::STATE_HOVERED, GetImageSkiaNamed(IDR_LIVEMAIN_JOINMIC_CLOSE_HV));
    close_button_->SetImage(views::Button::STATE_PRESSED, GetImageSkiaNamed(IDR_LIVEMAIN_JOINMIC_CLOSE_HV));
    close_button_->SetTooltipText(GetLocalizedString(IDS_UGC_CLOSE));

    state_view_ = new LivehimeListStateBannerView();
    state_view_->SetStateText(
        livehime::ListState::ListState_Loading, GetLocalizedString(IDS_LUCKYGIFT_INFO_STATUS_LOADING));
    state_view_->SetStateText(
        livehime::ListState::ListState_Faild, GetLocalizedString(IDS_LUCKYGIFT_INFO_STATUS_LOADING_FAILED));

    web_browser_view_ = new LivehimeWebBrowserView(cef_proxy::client_handler_type::mini_login,
        GetMiniLoginUrl(), this);

    AddChildView(title_label_);
    AddChildView(state_view_);
    AddChildView(close_button_);
    AddChildView(web_browser_view_);

    int color = SK_ColorWHITE;
    {
        color |= 0xFF000000;
        set_background(views::Background::CreateSolidBackground(color));
        state_view_->SetAutoColorReadabilityEnabled(true, color);
    }

    // ��ҳδ�������ǰ����WebView��visible��ô�仯������ʾ�������ϵ�ԭ���Ӵ���
    web_browser_view_->NativeControlIgnoreViewVisibilityChanged(true, false);
}

void BililiveMiniLoginViddupView::InitData()
{
    auto info = GetBililiveProcess()->secret_core()->GetLastValidLoginInfo();
    if (info)
    {
        // �������OBS��������ô�����ס������ľ�ֱ�ӵ�¼��
        // ������ζ������Ƕ��˺ŵ��û�������Ҫ�ڽ��뵽������֮����ע����¼����ʽ���л��˻����ƺ�Ҳ���ⲻ��
        bool auto_login = info->auto_login;
        auto_login |= CommandLine::ForCurrentProcess()->HasSwitch(bililive::kSwitchOBSPluginStartLive);

        if (auto_login && !info->cookies.empty())
        {
            cef_proxy::cookies cookies = livehime::StringToCefCookies(info->cookies);
            if (cookies.empty())
            {
                return;
            }

            LOG(INFO) << "[login] last login cookie not empty, try auto login with cookie validity check.";

            // UI�л�����¼��...
            state_view_->SetStateText(livehime::ListState::ListState_Loading, GetLocalizedString(IDS_WEB_MINI_LOGIN_AUTO_LOGIN));
            state_view_->SetState(livehime::ListState::ListState_Loading);
            ShowWebView(false);

            g_cookies = std::move(cookies);
            g_profile_cookies = true;

            // ע��net�㣬����ӿ�
            // CEF���ע����û���Ϣ��ȷ��ȡ��ע
            GetBililiveProcess()->secret_core()->SetAuthCookie(g_cookies);

            // ������Ϣ
            nav_by_initdata_ = true;
            GetUserInfo();
        }
    }
}

// WidgetObserver
void BililiveMiniLoginViddupView::WindowClosing()
{
    // ��ֹĿǰ��v3.40.0����ֻ�е�¼ģ���п��ܷ������������
    // ���û�ȡ����¼����ģ������ִ���˳��������̣��������ִ�кܿ죬
    // ��ô���ܵ���CefClientʵ���ò�����ʱ�ͷţ��Ӷ�����DCHECK��
    // ����������Ҫ�ȴ�cef��ʵ����ʵ�������˳�����MiniWebDestoryWatcher������
    if (!g_web_dst_watcher)
    {
        base::MessageLoop::current()->Quit();
    }
}

void BililiveMiniLoginViddupView::OnWidgetBoundsChanged(views::Widget* widget, const gfx::Rect& new_bounds)
{
    static int kRadix = 16;
    gfx::Rect rect = widget->GetWindowBoundsInScreen();
    HRGN rgn = ::CreateRoundRectRgn(0, 0, rect.width(), rect.height(), kRadix, kRadix);
    widget->SetShape(rgn);
}

// View
gfx::Size BililiveMiniLoginViddupView::GetPreferredSize() {
    static gfx::Size size(GetLengthByDPIScale(kMinWidth), GetLengthByDPIScale(kMinHeight));
    return size;
}

void BililiveMiniLoginViddupView::Layout()
{
    gfx::Rect rect = GetContentsBounds();
    LayoutInBounds(rect);
}

void BililiveMiniLoginViddupView::LayoutInBounds(gfx::Rect rect)
{
    gfx::Size title_pref_size = title_label_->GetPreferredSize();
    gfx::Size close_pref_size = close_button_->GetPreferredSize();
    int title_cy = std::max(title_pref_size.height(), close_pref_size.height());
    title_label_->SetBounds(rect.x() + kPaddingColWidthForGroupCtrls,
        rect.y(),
        rect.width() - kPaddingColWidthForGroupCtrls * 2 - close_pref_size.width(), title_cy);

    close_button_->SetBounds(rect.right() - close_pref_size.width(),
        rect.y() + (title_cy - close_pref_size.height()) / 2,
        close_pref_size.width(), close_pref_size.height());

    state_view_->SetBounds(rect.x(), close_button_->bounds().bottom(),
        rect.width(), rect.height() - close_button_->bounds().height());

    if (show_titlebar_)
    {
        // ��Ҫչʾ�رհ�ť�Ļ���
        rect.Inset(0, close_pref_size.height(), 0, 0);
    }
    web_browser_view_->SetBoundsRect(rect);
}

void BililiveMiniLoginViddupView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    GetWidget()->Close();
}

// LivehimeWebBrowserDelegate
void BililiveMiniLoginViddupView::OnWebBrowserCoreInvalid()
{
    state_view_->SetStateText(
        livehime::ListState::ListState_Faild, GetLocalizedString(IDS_WEB_CORE_INVALID));
    state_view_->SetState(livehime::ListState::ListState_Faild);
}

void BililiveMiniLoginViddupView::OnPreWebBrowserWindowCreate()
{
    // ��Bվ��cookieɾ�����ѵ�¼̬cookieɾ��
    CefProxyWrapper::GetInstance()->DeleteAuthCookies();

    g_web_dst_watcher.reset(new MiniWebDestoryWatcher(*web_browser_view_->browser_bind_data()));
}

void BililiveMiniLoginViddupView::OnWebBrowserLoadStart(const std::string& url)
{
    if (url.find(GetMiniLoginUrl()) != std::string::npos) {
        webview_loading_start_time_ = base::Time::Now();
    }
}

void BililiveMiniLoginViddupView::OnWebBrowserLoadEnd(const std::string& url, int http_status_code)
{
    if (HTTP_STATUS_OK == http_status_code)
    {
        // ������ҳʱ��Cef���ܻ��ȼ��ء�about:blank����ԭ����飩�ټ���kUrlMiniLogin��
        // ����������жϣ�ֱ���ڡ�about:blank��ʱ�Ͱ�WebViewչʾ��������ô��InitData()��⵽��Ҫ�Զ���¼ʱ,
        // ���ڲ����WebView��������������nav�ӿڣ���ʱ���ܷ����ӿ�����û����ǰkUrlMiniLogin�ļ������֪ͨ�͵��ˣ�
        // UI�Ͼͻ��mini��¼����չʾ�����������û���û���ü��������ʱnav�ӿ����ݻ����˽�������Զ���¼���û��ͺ��±��ˡ�
        // ���������ֻ�е�ҳ�������ɵ�֪ͨ����kUrlMiniLogin������ɵĲŽ����Զ���¼���

        std::string ff_url = LoginUrlSplit(bililive::FastForwardChangeEnv(GetMiniLoginUrl()));
        if (url.find(ff_url) != std::string::npos){
            web_browser_view_->NativeControlIgnoreViewVisibilityChanged(false, true);

            // ҳ�����������پ���Ҫ��Ҫʹ���ϴγɹ����˻�����¼
            static bool inited = false;
            if (!inited)
            {
                inited = true;
                InitData();
            }
        }

        if (url.find(GetMiniLoginUrl()) != std::string::npos) {

        }
    }
    else{
        OnWebBrowserLoadError(url, http_status_code, "");
    }
}

void BililiveMiniLoginViddupView::OnWebBrowserLoadError(const std::string& failed_url, int http_status_code, const std::string& error_text)
{
    state_view_->SetState(livehime::ListState::ListState_Faild);

    if (!error_text.empty()) {
        
    }
}

WebViewPopupType BililiveMiniLoginViddupView::OnWebBrowserPopup(const std::string& url, int target_disposition)
{
    return WebViewPopupType::System;
}

bool BililiveMiniLoginViddupView::OnWebBrowserMsgReceived(const std::string& msg_name, const cef_proxy::calldata* data)
{
    if (msg_name == ipc_messages::kMiniLoginRestore)
    {
        LOG(INFO) << "[login] login restore.";

        // ����UI����ʼ״̬
        web_browser_view_->Navigate(GetMiniLoginUrl());
        ShowWebView(true);

        return true;
    }
    if (msg_name == ipc_messages::kMiniLoginSuccess)
    {
        is_login_success = true;
        std::string details = data->at("details").str_;

        LOG(INFO) << "[login] login auth check success, " << details << ", will check auth validity.";

        // ��һ��UI״̬���ȴ��ײ��cookie�������ٽ�����֤
        state_view_->SetStateText(livehime::ListState::ListState_Loading, GetLocalizedString(IDS_WEB_MINI_LOGIN_INFO));
        state_view_->SetState(livehime::ListState::ListState_Loading);
        ShowWebView(false);

        // --todo
        // data=details(�ײ�data�����Ѿ���������)
        // data={"userid":"1750131179","successData":{"type":{"type":"login","by":""}},"cookie":"buvid3=18f048a2-3d8d-4ec1-8dda-10ba0b38e53e16108infoc; bili_jct=2838aee64bfd4028a25d8884de237333; DedeUserID=1750131179"}
        // ����BLoginManager::OnCefAppMsgReceived�е�dataType == "login"�߼�
        // FetchCookie();
        //utils::SaveUserDat(BasicVariables::loginDataFile, data.toUtf8());
        //TRACEI("Save login data into file {}", BasicVariables::loginDataFile);

        //auto doc = QJsonDocument::fromJson(data.toUtf8());
        //if (!doc.isNull() && doc.isObject()) {
        //    auto obj = doc.object();
        //    auto userid = obj["userid"].toString();
        //    auto cookie = obj["cookie"].toString();
        //    auto stringList = cookie.split(":");
        //    if (stringList.size() >= 2)
        //        cookie = stringList[1];
        //    UserProfile::GetProfile()->ReparseProfile();
        //    auto file = UserProfile::GetProfile()->GetProfile();
        //    QString sessData = file->loginData.SESSDATA;
        //    if (sessData != "") {
        //        cookie = cookie + "; SESSDATA=" + sessData;
        //    }
        //    else
        //    {
        //        TRACEE("SESSDATA is empty,can't login!!")
        //    }
        //    MakeGetRequest(REQ_TYPE::Profile, BasicVariables::avatarViddupUrl, cookie.toUtf8());
        //}
        // getUserInfo();

        // test-�����Ⱥ��ԣ�ֱ����Ϊ��¼�ɹ����ؽ�������ܽ���
       /* *result_code_ = LoginResultCode::LOGIN_SUCCESS;
        GetWidget()->Close();*/

        return true;
    }
    else if (msg_name == ipc_messages::kMiniLoginCancel)
    {
        if (is_login_success) 
        {
            return true;
        }
        else 
        {
            LOG(INFO) << "[login] user cancel login.";
            GetWidget()->Close();
            return true;
        }
    }
    else if (msg_name == ipc_messages::kMiniLoginChangeLoginMode)
    {
        int w = data->at("width").numeric_union.int_;
        int h = data->at("height").numeric_union.int_;

        LOG(INFO) << "[login] login view size change, size=" << w << "x" << h;

        // ��ȶ���420���߶���΢Сһ�㣬�����dpi�´������±߾���ܳ���mini������Һ�ɫ��䱳��
        ResetWindowSize(gfx::Size(GetLengthByDPIScale(kMinWidth), GetLengthByDPIScale(std::max(h, kMinHeight))),
            false);

        return true;
    }
    else if (msg_name == ipc_messages::kMiniLoginCookies)
    {
        std::string details = data->at("details").str_;
        LOG(INFO) << "[login] check auth validity, " << details;

        g_cookies = data->at("cookie").cookies_;
        g_profile_cookies = false;
        GetBililiveProcess()->secret_core()->SetAuthCookie(g_cookies);

        //// �Ӷ�����֤ҳ�������ˣ�����֪����֤ͨ��û�У����ӿ�У��һ��cookie�Ƿ���Ч
        //// H5��¼�ɹ���ֻ��cookie��¼̬��û��ϸ�µ��û���Ϣ����Ҫ���󵥶��Ľӿ�����ȡϸ�µ��û���Ϣ��
        //// �û���Ϣ��ȡ���˲Űѵ�¼���˳���ʼ����������
        GetUserInfo();

        return true;
    }
    else if (msg_name == ipc_messages::kLivehimeCefOnTitleChanged)
    {
        std::string title = data->at("title").str_;
        title_label_->SetText(base::UTF8ToWide(title));
        InvalidateLayout();
        Layout();

        return true;
    }
    else if (msg_name == ipc_messages::kMiniLoginUnexpectedPage)
    {
        std::string url = data->at("url").str_;
        LOG(WARNING) << "[login] unexpected page \"" << url << "\".";

        // δ֪״̬��ҳ�棬�Ѵ�������չʾ
        //static gfx::Size size(GetLengthByDPIScale(kWebVerifyWidth), GetLengthByDPIScale(kWebVerifyHeight));
        //ResetWindowSize(size, true);

        return true;
    }

    return false;
}

void BililiveMiniLoginViddupView::GetUserInfo()
{
    //nav_by_initdata_ = true;
    GetBililiveProcess()->secret_core()->user_account_service()->GetViddupUserInfo({},
        base::MakeCallable(base::Bind(&BililiveMiniLoginViddupView::OnGetUserInfo, weak_factory_.GetWeakPtr()))).Call();
}

//���⿪�� - �û���Ϣ�ص�����
void BililiveMiniLoginViddupView::OnGetUserInfo(bool valid_response, int code,
    const secret::UserAccountService::ViddupUserInfoData& info)
{
    if (valid_response && 0 == code)
    {
        LOG(INFO) << "[login] nav user info success, login success.";

        // ����ĵ��ò�Ϊ��ģ�ֻΪ�˺�native�ĵ�¼����һ�£���������ý���ֻ����secret����һ���û���Ϣʵ����
        // �Ա�������������߼��ܱ����û���Ϣ�Ŀ�ָ������
        secret::LoginInfo login_info;

        login_info.mini_login = true;
        login_info.mid = info.mid;
        login_info.auto_login = true;
        login_info.cookies = livehime::CefCookiesToString(g_cookies);
      

        presenter_->SaveLoginInfo(login_info, true);

        // ��¼�ɹ�������¼̬���õ�CEF cookie��
        if (g_profile_cookies)
        {
            CefProxyWrapper::GetInstance()->SetMiniLoginCookies(g_cookies);
        }

        *result_code_ = LoginResultCode::LOGIN_SUCCESS;

        GetWidget()->Close();
    }
    else
    {
        is_login_success = false;
        LOG(WARNING) << "[login] nav user info failed, code=" << code;

        // ��ʾһ�£��û���Ϣ��ȡʧ�ܣ����¼��ص�¼ҳ��
        GetBililiveProcess()->SetTokenInvalid();
        // ��Bվ��cookieɾ�����ѵ�¼̬cookieɾ��
        CefProxyWrapper::GetInstance()->DeleteAuthCookies();
      
        state_view_->SetStateText(livehime::ListState::ListState_Faild, GetLocalizedString(IDS_UGC_MSGBOX_GETUSERINFO_FAILED));
        state_view_->SetState(livehime::ListState::ListState_Faild);

        base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
            base::Bind(&BililiveMiniLoginViddupView::ShowWebView, weak_factory_.GetWeakPtr(), true),
            base::TimeDelta::FromSeconds(3));
    }
}

void BililiveMiniLoginViddupView::ShowWebView(bool show)
{
    web_browser_view_->SetVisible(show);
}

void BililiveMiniLoginViddupView::ResetWindowSize(const gfx::Size& size, bool show_titlebar, SkColor bk_clr/* = SK_ColorWHITE*/)
{
    set_background(views::Background::CreateSolidBackground(bk_clr));
    gfx::Size tmp(size);
    show_titlebar_ = show_titlebar;
    if (show_titlebar_)
    {
        // �Ѵ��ڳߴ������󣬼���native�رհ�ť�ĸ߶�
        gfx::Size pref_size = close_button_->GetPreferredSize();
        tmp.Enlarge(0, pref_size.height());
    }
    GetWidget()->CenterWindow(tmp);
}

std::string BililiveMiniLoginViddupView::LoginUrlSplit(const std::string& url)
{
    std::vector<std::string> split_root;
    base::SplitString(url, '?', &split_root);
    if (split_root.size() >= 1) {
        return split_root[0];
    }
    return url;
}
