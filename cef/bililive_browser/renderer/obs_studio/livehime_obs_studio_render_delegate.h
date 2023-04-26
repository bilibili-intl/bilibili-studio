#pragma once

#include "bililive_browser/renderer/livehime_cef_app_render.h"

#include "obs-browser/browser-app.hpp"

/*
 * 1������ͬһ������������ģ�飨obs-browser��cef_proxy����cefģ����г�ʼ�����������ģ�鶼�޷�����������
 * �������ǽ���obs-browser��cef��ʼ����ֻ���������Լ���cef_proxy������cef��subprocess����Ҳ��ʹ��obs-studio��
 * obs-browser-page.exe�����ǲ��������Լ���bililive_browser.exe�������������Ǹ����Լ���ҵ��������й�����չ��
 *
 * 2����ǰ���ϵ�obs�汾��22.0.2����obs-browser-page.exe�л���򿪵�webҳ����ע��һЩjs�ӿ��Է���֧����Щ�ӿ�
 * ��webҳ���ܻ�ó��������硰��ǰ��obs���İ汾����ǰʹ�õĳ���������״̬��¼��״̬������Ϣ��
 * Ҳ��Ϊ�����ṩ��������webҳ���е��ض�js�����Ĺ��ܣ�
 *
 * 3���������ǲ�ʹ��obs-browser-page.exe����ô���Ǿ���Ҫ��2��������obs-browser-page.exe�Ĺ���ת�������Լ���
 * bililive_browser.exe����ʵ�֣�LivehimeOBSStudioRenderDelegate������������ʵ��������ܵ��ࣻ
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