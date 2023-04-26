#pragma once

#include "cef_proxy_dll/public/livehime_cef_proxy_calldata.h"

// @https://info.bilibili.co/pages/viewpage.action?pageId=30131705
// 页面所支持的B站js bridge method


namespace js_bridge
{
    // 向CefV8Context注入模块特定的对象
    bool InjectModuleObjToV8Context(CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Handler> handler,
        cef_proxy::client_handler_type type, const std::string& obj_name);

    // 判断js执行的函数是否属于指定模块的对象
    bool IsInvokeOnModuleObj(CefRefPtr<CefV8Value> object, cef_proxy::client_handler_type type);


    // 与前端约定的我们所支持的挂载在某个对象下的函数，
    // 约定为所有业务都可使用的公共函数才放在这里，特定业务特有的定制函数不要放在这里
    enum class SupportJsFunctionType
    {
        postMessage,
    };

    const std::map<SupportJsFunctionType, std::string> kSupportJsFunctionNames{
        { SupportJsFunctionType::postMessage, "postMessage"},
    };

    // 我们所支持的前端封装的bridge通过其内的callNative函数传参的method名
    enum class SupportBridgeMethodType
    {
        pageFinished,       // 页面的业务初始化完成（与页面web内核层面加载完成不是一个概念）
        full_closeWindow,
        half_closeWindow,

        arouse,             // h5通过native进行唤起指定模块
        bsns_report,        // h5通过native进行业务埋点
        tec_report,         // h5通过native进行技术埋点
        polaris_report,     // h5通过native进行北极星业务埋点
        web_projection,     // h5通过native进行页面投屏到预览区
        get_pc_link_info,   // h5从native拿数据
        subscribe_pc_broadcast, //h5订阅native广播数据
        unsubscribe_broadcast, //h5取消订阅native广播数据

        alert,              // h5通过native进行唤起弹窗
        toast,              // h5通过native进行toast提示
        set_title,          // h5通过native进行更改title值
        set_nav_menu,       // h5通过native进行设置navbar menu
        title_bar,          // h5通过native进行设置navbar样式

        get_all_support,    // h5通过native获取当前容器支持的所有方法
        get_container_info, // h5通过native获取当前PC直播姬基础信息
        import,             // h5通过native引入额外的namespace
        forward,            // h5通过native打开新页面
        local_cache,        // h5通过native操作全局本地缓存
        open_in_app,        // H5通过native打开指定弹窗（协议同arouse中的open_in_app一致）
    };

    // 处理常规的js函数，处理了就返回true，反之false；
    // 当返回true时可以根据handled_result来确定该js函数的处理结果是否正常
    bool ScheduleCommonJsFunciton(CefRefPtr<CefV8Handler> handler, std::map<std::string, SupportBridgeMethodType>& kSupportBridgeMethodNames, 
        const std::string& obj_name, const CefString& name, const CefV8ValueList& arguments, bool& handled_result);

    const std::map<std::string, SupportBridgeMethodType> kSupportBridgeMethodNamesDefalut {
        { "live_ui_half.pageFinished", SupportBridgeMethodType::pageFinished },
        { "live_ui.pageFinished", SupportBridgeMethodType::pageFinished },  // 新的namespace命名规范
        { "live_location_full.closeWindow", SupportBridgeMethodType::full_closeWindow },
        { "live_location_half.closeWindow", SupportBridgeMethodType::half_closeWindow },
        { "live_location.closeWindow", SupportBridgeMethodType::full_closeWindow },  // 新的namespace命名规范

        { "live_arouse_pc_link_module", SupportBridgeMethodType::arouse },
        { "live_location_event_report", SupportBridgeMethodType::bsns_report },
        { "live_location_half_report", SupportBridgeMethodType::tec_report },
        { "live_location_polaris_report", SupportBridgeMethodType::polaris_report },
        { "live_network.polarisReport", SupportBridgeMethodType::polaris_report },  // 新的namespace命名规范
        { "live_location_web_projection", SupportBridgeMethodType::web_projection },
        { "get_pc_link_info"/*"offline.getCookie"*/, SupportBridgeMethodType::get_pc_link_info },
        { "subscribe_pc_broadcast", SupportBridgeMethodType::subscribe_pc_broadcast },
        //{ "live_stream.receiveBroadcast", SupportBridgeMethodType::subscribe_pc_broadcast },  // 新的namespace命名规范，需要import导入生效
        { "unsubscribe_pc_broadcast", SupportBridgeMethodType::unsubscribe_broadcast },

        { "live_ui.alert", SupportBridgeMethodType::alert },
        { "live_ui.toast", SupportBridgeMethodType::toast },
        //{ "live_ui.setTitle", SupportBridgeMethodType::set_title },  // discard
        //{ "live_ui.setNavMenu", SupportBridgeMethodType::set_nav_menu },  // discard
        { "live_ui.titleBar", SupportBridgeMethodType::title_bar },

        { "global.getAllSupport", SupportBridgeMethodType::get_all_support },
        { "global.getContainerInfo", SupportBridgeMethodType::get_container_info },
        { "global.import", SupportBridgeMethodType::import },
        { "live_location.forward", SupportBridgeMethodType::forward },
        { "live_cache.localCache", SupportBridgeMethodType::local_cache },
    };

    struct bridge_method
    {
        std::string name;
        SupportBridgeMethodType type;
    };

    // 待h5导入的namespace下对应的bridge，导入后添加进kSupportBridgeMethodNames中
    const std::map<std::string, std::vector<bridge_method>> kSupportBridgeNamespaceMapping {
        { "live_stream", { 
            { "live_stream.receiveBroadcast", SupportBridgeMethodType::subscribe_pc_broadcast },
            { "live_stream.open_in_app", SupportBridgeMethodType::open_in_app }
          }
        },  // 新的namespace命名规范
    };

    // 处理常规的method，处理过了就返回true
    bool ScheduleCommonBiliJsBridgeMethod(CefRefPtr<CefV8Handler> handler, std::map<std::string, SupportBridgeMethodType>& kSupportBridgeMethodNames, 
        const std::string& method_name, CefRefPtr<CefDictionaryValue> data, CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Value> callback);
}

