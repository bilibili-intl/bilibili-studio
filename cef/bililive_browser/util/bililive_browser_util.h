#pragma once

// 获取B站js脚本postMesssage方法的参数的字符串形式，有时候传的就是json字串有时候传的又是object
CefString GetBilibiliPostMessageJsonString(CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Value> value,
    CefRefPtr<CefV8Value>* out_callback = nullptr);

// 根据callbackId从BiliJsBridge.callbacks中把对应的回调函数callback拿到
CefRefPtr<CefV8Value> GetBiliJsBridgeCallback(CefRefPtr<CefV8Context> context, int callbackId);

// 由renderer进程收到基础bridge时的同步调用callback，未处理的交给browser进程业务侧来异步调用
void InvokeJsCallbackSync(CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Value> callback, CefRefPtr<CefV8Value> param);

// 由renderer进程使用的用于处理browser进程发过来的通用ipc消息
bool ScheduleCommonBrowserIPCMessage(CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> message);