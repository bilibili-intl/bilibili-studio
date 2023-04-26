#pragma once

namespace ipc_messages
{
    const char kLivehimeCefJsMsgForTest[] = "livehime_cef_js_msg_for_test";
    const char kLivehimeCefExexuteJsFunction[] = "livehime_cef_execute_javascript";
    const char kLivehimeCefSetCookieTaskCompleted[] = "livehime_cef_set_cookie_task_completed";
    const char kLivehimeCefOnWindowClose[] = "livehime_cef_on_window_close";
    const char kLivehimeCefOnLoadStart[] = "livehime_cef_on_load_start";
    const char kLivehimeCefOnLoadEnd[] = "livehime_cef_on_load_end";
    const char kLivehimeCefOnLoadError[] = "livehime_cef_on_load_error";
    const char kLivehimeCefOnTitleChanged[] = "livehime_cef_on_title_changed";
    const char kLivehimeCefOnPreKeyEvent[] = "livehime_cef_on_pre_keyevent";
    const char kLivehimeCefOnKeyEvent[] = "livehime_cef_on_keyevent";
    const char kLivehimeCefOnPopup[] = "livehime_cef_on_before_popup";
    const char kLivehimeCefOnCertificateError[] = "livehime_cef_on_certificate_error";
    const char kLivehimeCefPopupDevTools[] = "livehime_cef_popup_dev_tools";
    const char kLivehimeCefClientDestoryed[] = "livehime_cef_client_destroyed";
    const char kLivehimeCefOnLoadTitle[] = "livehime_cef_on_load_title";

    // B站业务相关页面采用的通知
    const char kBilibiliPageFinished[] = "BilibiliPageFinished";// H5页面逻辑加载完毕
    const char kBilibiliPageClosed[] = "BilibiliPageClosed";// 用户关闭了H5页面
    const char kBilibiliPageReport[] = "BilibiliPageReport";// H5页面通过直播姬进行埋点，报的表是v2的业务埋点表，不是我们旧有的技术埋点表
    const char kBilibiliPageTecReport[] = "BilibiliPageTecReport";// H5页面通过直播姬进行埋点，报的表是我们旧有的技术埋点表
    const char kBilibiliPagePolarisReport[] = "BilibiliPagePolarisReport";// H5页面通过直播姬进行埋点，报的北极星表
    const char kBilibiliPageArouse[] = "BilibiliPageArouseModule";// H5页面通过直播姬唤起模块
    const char kBilibiliPageProjection[] = "BilibiliPageProjection";// H5页面通知直播姬进行页面投屏
    const char kBilibiliPageGetInfo[] = "BilibiliPageGetInfo";// H5页面从直播姬拿数据
    const char kBilibiliPageSubBroadcast[] = "BilibiliPageSubBroadcast";// H5页面订阅直播姬广播数据
    const char kBilibiliPageUnSubBroadcast[] = "BilibiliPageUnSubBroadcast";// H5页面取消订阅直播姬广播数据

    const char kBilibiliPageAlert[] = "BilibiliPageAlert";// H5页面通过直播姬唤起弹窗
    const char kBilibiliPageToast[] = "BilibiliPageToast";// H5页面通过直播姬进行toast提示
    const char kBilibiliPageSetTitle[] = "BilibiliPageSetTitle";// H5页面通过直播姬更改title值
    const char kBilibiliPageSetNavMenu[] = "BilibiliPageSetNavMenu";// H5页面通过直播姬设置navbar menu
    const char kBilibiliPageTitleBar[] = "BilibiliPageTitleBar";// H5页面通过直播姬设置navbar样式

    const char kBilibiliPageGetAllSupport[] = "BilibiliPageGetAllSupport";// H5页面通过直播姬获取当前容器支持的所有方法
    const char kBilibiliPageGetContainerInfo[] = "BilibiliPageGetContainerInfo";// H5页面通过直播姬获取当前PC直播姬基础信息
    const char kBilibiliPageImport[] = "BilibiliPageImport";// H5页面通过直播姬引入额外的namespace
    const char kBilibiliPageForward[] = "BilibiliPageForward";// H5页面通过直播姬打开新页面
    const char kBilibiliPageLocalCache[] = "BilibiliPageLocalCache";// H5页面通过直播姬操作全局本地缓存

    // 极验验证码
    const char kGeetestVerifyCompleted[] = "GeetestVerifyCompleted";// 滑块验证通过
    const char kGeetestImageCaptchaVerifyCompleted[] = "GeetestImageCaptchaVerifyCompleted";// 图片验证通过
    const char kGeetestClosed[] = "GeetestClosed";// 用户关闭了H5页面

    // 二次登陆验证
    const char kSecondaryVerifyValidateLogin[] = "SecondaryVerifyValidateLogin";// WEB二次验证通过
    const char kSecondaryVerifyAccessTokenCompleted[] = "SecondaryVerifyAccessTokenCompleted";// 二次验证成功获取token
    const char kSecondaryVerifyCompleted[] = "SecondaryVerifyCompleted";// 二次验证流程已成功结束
    const char kChangePasswordCompleted[] = "ChangePasswordCompleted";// 修改密码已成功
    const char kChangePasswordReLogin[] = "ChangePasswordReLogin";// 修改密码已成功需要重登录

    // 幸运礼物
    const char kLuckyGiftClosed[] = "LuckyGiftClosed";// 用户关闭了H5页面
    const char kLuckyGiftInfoClosed[] = "LuckyGiftInfoClosed";// 用户关闭了奖池H5页面
    const char kLuckyGiftAwardClosed[] = "LuckyGiftAwardClosed";// 用户关闭了中奖H5页面

    // 主播抽奖
    const char kLotteryClosed[] = "LotteryClosed";
    const char kLotteryStatusChangedR[] = "LotteryStatusChangedR";       //收到H5的状态改变
    const char kLotteryStatusChangedS[] = "LotteryStatusChangedS";       //收到广播的改变，需要通知H5

    // biliyun文件上传回调
    const char kBiliyunBeforePreUpload[] = "BiliyunBeforePreUpload";
    const char kBiliyunBeforeUpload[] = "BiliyunBeforeUpload";
    const char kBiliyunUploadProgress[] = "BiliyunUploadProgress";
    const char kBiliyunBeforeFileUploaded[] = "BiliyunBeforeFileUploaded";  // 上传结束，合片请求开始阶段
    const char kBiliyunFileUploaded[] = "BiliyunFileUploaded";  // 上传结束，云端也同步完毕
    const char kBiliyunUploadError[] = "BiliyunUploadError";
    // biliyun相关本地回调
    const char kBiliyunRenderedTerminated[] = "BiliyunRenderedTerminated";
    // 对biliyun进行操作
    const char kBiliyunUploaderLocalFile[] = "BiliyunUploaderLocalFile";
    const char kBiliyunUploaderDelete[] = "BiliyunUploaderDelete";
    //const char kBiliyunUploaderPause[] = "BiliyunUploaderPause";

    // mini登录结果回调
    const char kMiniLoginSuccess[] = "MiniLoginSuccess";
    const char kMiniLoginCancel[] = "MiniLoginCancel";
    const char kMiniLoginChangeLoginMode[] = "MiniLoginChangeLoginMode";
    const char kMiniLoginCookies[] = "kMiniLoginCookies";
    const char kMiniLoginPwSecVld[] = "MiniLoginPwSecVld";
    const char kMiniLoginMobileSecVld[] = "MiniLoginMobileSecVld";
    const char kMiniLoginRealNameVld[] = "MiniLoginRealNameVld";
    const char kMiniLoginSmsVld[] = "MiniLoginSmsVld";
    const char kMiniLoginSmsSecVld[] = "MiniLoginSmsSecVld";
    const char kMiniLoginSecVldResult[] = "MiniLoginSecVldResult";
    const char kMiniLoginUnexpectedPage[] = "MiniLoginUnexpectedPage";
    const char kMiniLoginRestore[] = "MiniLoginRestore";

}  // namespace ipc_messages

