#pragma once

// ��ȡBվjs�ű�postMesssage�����Ĳ������ַ�����ʽ����ʱ�򴫵ľ���json�ִ���ʱ�򴫵�����object
CefString GetBilibiliPostMessageJsonString(CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Value> value,
    CefRefPtr<CefV8Value>* out_callback = nullptr);

// ����callbackId��BiliJsBridge.callbacks�аѶ�Ӧ�Ļص�����callback�õ�
CefRefPtr<CefV8Value> GetBiliJsBridgeCallback(CefRefPtr<CefV8Context> context, int callbackId);

// ��renderer�����յ�����bridgeʱ��ͬ������callback��δ����Ľ���browser����ҵ������첽����
void InvokeJsCallbackSync(CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Value> callback, CefRefPtr<CefV8Value> param);

// ��renderer����ʹ�õ����ڴ���browser���̷�������ͨ��ipc��Ϣ
bool ScheduleCommonBrowserIPCMessage(CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> message);