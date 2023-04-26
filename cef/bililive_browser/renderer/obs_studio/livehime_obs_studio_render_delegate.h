#pragma once

#include "bililive_browser/renderer/livehime_cef_app_render.h"

#include "obs-browser/browser-app.hpp"

/*
 * 1、由于同一进程内有两个模块（obs-browser和cef_proxy）对cef模块进行初始化而导致相关模块都无法正常工作，
 * 所以我们禁用obs-browser的cef初始化，只采用我们自己的cef_proxy，并且cef的subprocess进程也不使用obs-studio的
 * obs-browser-page.exe，而是采用我们自己的bililive_browser.exe，这样方便我们根据自己的业务需求进行功能扩展；
 *
 * 2、当前整合的obs版本（22.0.2）的obs-browser-page.exe中会向打开的web页面中注册一些js接口以方便支持这些接口
 * 的web页面能获得程序中诸如“当前的obs核心版本、当前使用的场景、推流状态、录制状态”等信息，
 * 也能为程序提供主动调用web页面中的特定js函数的功能；
 *
 * 3、由于我们不使用obs-browser-page.exe，那么我们就需要将2中所述的obs-browser-page.exe的功能转到我们自己的
 * bililive_browser.exe中来实现，LivehimeOBSStudioRenderDelegate这个类就是用来实现这个功能的类；
 *
 */

class LivehimeOBSStudioRenderDelegate :
    public LivehimeCefAppRender::Delegate,
    public CefV8Handler
{
public:
    LivehimeOBSStudioRenderDelegate();
    virtual ~LivehimeOBSStudioRenderDelegate();

    // LivehimeCefAppRender::Delegate
    void OnBeforeCommandLineProcessing(
        CefRefPtr<LivehimeCefAppRender> app,
        const CefString &process_type,
        CefRefPtr<CefCommandLine> command_line) override;
    void OnRegisterCustomSchemes(CefRefPtr<LivehimeCefAppRender> app,
        CefRawPtr<CefSchemeRegistrar> registrar) override;
    void OnContextCreated(CefRefPtr<LivehimeCefAppRender> app,
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context) override;
    bool OnProcessMessageReceived(CefRefPtr<LivehimeCefAppRender> app,
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message) override;

    // CefV8Handler
    bool Execute(const CefString &name,
        CefRefPtr<CefV8Value> object,
        const CefV8ValueList &arguments,
        CefRefPtr<CefV8Value> &retval,
        CefString &exception) override;
private:
    CefRefPtr<BrowserApp> obs_browser_page_browserapp_;

    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(LivehimeOBSStudioRenderDelegate);
    DISALLOW_COPY_AND_ASSIGN(LivehimeOBSStudioRenderDelegate);
};