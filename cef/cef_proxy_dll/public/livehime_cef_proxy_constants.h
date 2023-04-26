#pragma once

namespace cef_proxy
{
    // CEFģ���ʼ��ʧ�ܵ������
    // cef���dll���ڣ�����CefInitialize�Ѻ�ʧ���ˣ�
    // cef���dll���ڣ���cef_proxy.dll�ļ��汾���ԣ�ȱ���°���Ҫ�ĵ���������
    // cef���dll���ڣ�����CefInitialize�����ˣ�
    // cef����ļ�ȱʧ������LoadLibraryʧ���ˣ�
    enum class CefInitResult
    {
        CefInitializeSuccess,
        ModuleMissing,
        ModuleInvalid,
        CefInitializeFailed,
        CefInitializeCrash,
    };

    const char kCefLocale[] = "en-US";
	const char kCefBrowserTitle[] = "www.bilibili.tv";

    const wchar_t kCefSubProcesses[] = L"bililive_browser";    
    const char kCefDefaultUserAgent[] = "Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/105.0.5195.127 Safari/537.36";
    const std::string kCefAndroidUserAgentMasquerading = "Android";
    const std::string kCefAndroidUserAgent = std::string(kCefDefaultUserAgent) + " " + kCefAndroidUserAgentMasquerading;
    const wchar_t kBililiveBrowserLogFileName[] = L"bililive_browser_debug.log";
    const wchar_t kBililiveBrowserCacheDirName[] = L"Browser Cache 3770";
}