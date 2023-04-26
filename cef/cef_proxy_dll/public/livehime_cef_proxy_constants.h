#pragma once

namespace cef_proxy
{
    // CEF模块初始化失败的情况：
    // cef相关dll存在，但是CefInitialize友好失败了；
    // cef相关dll存在，但cef_proxy.dll文件版本不对，缺少新版需要的导出函数；
    // cef相关dll存在，但是CefInitialize崩溃了；
    // cef相关文件缺失，导致LoadLibrary失败了；
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