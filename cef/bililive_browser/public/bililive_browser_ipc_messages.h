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

    // Bվҵ�����ҳ����õ�֪ͨ
    const char kBilibiliPageFinished[] = "BilibiliPageFinished";// H5ҳ���߼��������
    const char kBilibiliPageClosed[] = "BilibiliPageClosed";// �û��ر���H5ҳ��
    const char kBilibiliPageReport[] = "BilibiliPageReport";// H5ҳ��ͨ��ֱ����������㣬���ı���v2��ҵ�������������Ǿ��еļ�������
    const char kBilibiliPageTecReport[] = "BilibiliPageTecReport";// H5ҳ��ͨ��ֱ����������㣬���ı������Ǿ��еļ�������
    const char kBilibiliPagePolarisReport[] = "BilibiliPagePolarisReport";// H5ҳ��ͨ��ֱ����������㣬���ı����Ǳ�
    const char kBilibiliPageArouse[] = "BilibiliPageArouseModule";// H5ҳ��ͨ��ֱ��������ģ��
    const char kBilibiliPageProjection[] = "BilibiliPageProjection";// H5ҳ��ֱ֪ͨ��������ҳ��Ͷ��
    const char kBilibiliPageGetInfo[] = "BilibiliPageGetInfo";// H5ҳ���ֱ����������
    const char kBilibiliPageSubBroadcast[] = "BilibiliPageSubBroadcast";// H5ҳ�涩��ֱ�����㲥����
    const char kBilibiliPageUnSubBroadcast[] = "BilibiliPageUnSubBroadcast";// H5ҳ��ȡ������ֱ�����㲥����

    const char kBilibiliPageAlert[] = "BilibiliPageAlert";// H5ҳ��ͨ��ֱ�������𵯴�
    const char kBilibiliPageToast[] = "BilibiliPageToast";// H5ҳ��ͨ��ֱ��������toast��ʾ
    const char kBilibiliPageSetTitle[] = "BilibiliPageSetTitle";// H5ҳ��ͨ��ֱ��������titleֵ
    const char kBilibiliPageSetNavMenu[] = "BilibiliPageSetNavMenu";// H5ҳ��ͨ��ֱ��������navbar menu
    const char kBilibiliPageTitleBar[] = "BilibiliPageTitleBar";// H5ҳ��ͨ��ֱ��������navbar��ʽ

    const char kBilibiliPageGetAllSupport[] = "BilibiliPageGetAllSupport";// H5ҳ��ͨ��ֱ������ȡ��ǰ����֧�ֵ����з���
    const char kBilibiliPageGetContainerInfo[] = "BilibiliPageGetContainerInfo";// H5ҳ��ͨ��ֱ������ȡ��ǰPCֱ����������Ϣ
    const char kBilibiliPageImport[] = "BilibiliPageImport";// H5ҳ��ͨ��ֱ������������namespace
    const char kBilibiliPageForward[] = "BilibiliPageForward";// H5ҳ��ͨ��ֱ��������ҳ��
    const char kBilibiliPageLocalCache[] = "BilibiliPageLocalCache";// H5ҳ��ͨ��ֱ��������ȫ�ֱ��ػ���

    // ������֤��
    const char kGeetestVerifyCompleted[] = "GeetestVerifyCompleted";// ������֤ͨ��
    const char kGeetestImageCaptchaVerifyCompleted[] = "GeetestImageCaptchaVerifyCompleted";// ͼƬ��֤ͨ��
    const char kGeetestClosed[] = "GeetestClosed";// �û��ر���H5ҳ��

    // ���ε�½��֤
    const char kSecondaryVerifyValidateLogin[] = "SecondaryVerifyValidateLogin";// WEB������֤ͨ��
    const char kSecondaryVerifyAccessTokenCompleted[] = "SecondaryVerifyAccessTokenCompleted";// ������֤�ɹ���ȡtoken
    const char kSecondaryVerifyCompleted[] = "SecondaryVerifyCompleted";// ������֤�����ѳɹ�����
    const char kChangePasswordCompleted[] = "ChangePasswordCompleted";// �޸������ѳɹ�
    const char kChangePasswordReLogin[] = "ChangePasswordReLogin";// �޸������ѳɹ���Ҫ�ص�¼

    // ��������
    const char kLuckyGiftClosed[] = "LuckyGiftClosed";// �û��ر���H5ҳ��
    const char kLuckyGiftInfoClosed[] = "LuckyGiftInfoClosed";// �û��ر��˽���H5ҳ��
    const char kLuckyGiftAwardClosed[] = "LuckyGiftAwardClosed";// �û��ر����н�H5ҳ��

    // �����齱
    const char kLotteryClosed[] = "LotteryClosed";
    const char kLotteryStatusChangedR[] = "LotteryStatusChangedR";       //�յ�H5��״̬�ı�
    const char kLotteryStatusChangedS[] = "LotteryStatusChangedS";       //�յ��㲥�ĸı䣬��Ҫ֪ͨH5

    // biliyun�ļ��ϴ��ص�
    const char kBiliyunBeforePreUpload[] = "BiliyunBeforePreUpload";
    const char kBiliyunBeforeUpload[] = "BiliyunBeforeUpload";
    const char kBiliyunUploadProgress[] = "BiliyunUploadProgress";
    const char kBiliyunBeforeFileUploaded[] = "BiliyunBeforeFileUploaded";  // �ϴ���������Ƭ����ʼ�׶�
    const char kBiliyunFileUploaded[] = "BiliyunFileUploaded";  // �ϴ��������ƶ�Ҳͬ�����
    const char kBiliyunUploadError[] = "BiliyunUploadError";
    // biliyun��ر��ػص�
    const char kBiliyunRenderedTerminated[] = "BiliyunRenderedTerminated";
    // ��biliyun���в���
    const char kBiliyunUploaderLocalFile[] = "BiliyunUploaderLocalFile";
    const char kBiliyunUploaderDelete[] = "BiliyunUploaderDelete";
    //const char kBiliyunUploaderPause[] = "BiliyunUploaderPause";

    // mini��¼����ص�
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

