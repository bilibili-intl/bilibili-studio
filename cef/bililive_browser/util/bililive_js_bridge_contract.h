#pragma once

#include "cef_proxy_dll/public/livehime_cef_proxy_calldata.h"

// @https://info.bilibili.co/pages/viewpage.action?pageId=30131705
// ҳ����֧�ֵ�Bվjs bridge method


namespace js_bridge
{
    // ��CefV8Contextע��ģ���ض��Ķ���
    bool InjectModuleObjToV8Context(CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Handler> handler,
        cef_proxy::client_handler_type type, const std::string& obj_name);

    // �ж�jsִ�еĺ����Ƿ�����ָ��ģ��Ķ���
    bool IsInvokeOnModuleObj(CefRefPtr<CefV8Value> object, cef_proxy::client_handler_type type);


    // ��ǰ��Լ����������֧�ֵĹ�����ĳ�������µĺ�����
    // Լ��Ϊ����ҵ�񶼿�ʹ�õĹ��������ŷ�������ض�ҵ�����еĶ��ƺ�����Ҫ��������
    enum class SupportJsFunctionType
    {
        postMessage,
    };

    const std::map<SupportJsFunctionType, std::string> kSupportJsFunctionNames{
        { SupportJsFunctionType::postMessage, "postMessage"},
    };

    // ������֧�ֵ�ǰ�˷�װ��bridgeͨ�����ڵ�callNative�������ε�method��
    enum class SupportBridgeMethodType
    {
        pageFinished,       // ҳ���ҵ���ʼ����ɣ���ҳ��web�ں˲��������ɲ���һ�����
        full_closeWindow,
        half_closeWindow,

        arouse,             // h5ͨ��native���л���ָ��ģ��
        bsns_report,        // h5ͨ��native����ҵ�����
        tec_report,         // h5ͨ��native���м������
        polaris_report,     // h5ͨ��native���б�����ҵ�����
        web_projection,     // h5ͨ��native����ҳ��Ͷ����Ԥ����
        get_pc_link_info,   // h5��native������
        subscribe_pc_broadcast, //h5����native�㲥����
        unsubscribe_broadcast, //h5ȡ������native�㲥����

        alert,              // h5ͨ��native���л��𵯴�
        toast,              // h5ͨ��native����toast��ʾ
        set_title,          // h5ͨ��native���и���titleֵ
        set_nav_menu,       // h5ͨ��native��������navbar menu
        title_bar,          // h5ͨ��native��������navbar��ʽ

        get_all_support,    // h5ͨ��native��ȡ��ǰ����֧�ֵ����з���
        get_container_info, // h5ͨ��native��ȡ��ǰPCֱ����������Ϣ
        import,             // h5ͨ��native��������namespace
        forward,            // h5ͨ��native����ҳ��
        local_cache,        // h5ͨ��native����ȫ�ֱ��ػ���
        open_in_app,        // H5ͨ��native��ָ��������Э��ͬarouse�е�open_in_appһ�£�
    };

    // �������js�����������˾ͷ���true����֮false��
    // ������trueʱ���Ը���handled_result��ȷ����js�����Ĵ������Ƿ�����
    bool ScheduleCommonJsFunciton(CefRefPtr<CefV8Handler> handler, std::map<std::string, SupportBridgeMethodType>& kSupportBridgeMethodNames, 
        const std::string& obj_name, const CefString& name, const CefV8ValueList& arguments, bool& handled_result);

    const std::map<std::string, SupportBridgeMethodType> kSupportBridgeMethodNamesDefalut {
        { "live_ui_half.pageFinished", SupportBridgeMethodType::pageFinished },
        { "live_ui.pageFinished", SupportBridgeMethodType::pageFinished },  // �µ�namespace�����淶
        { "live_location_full.closeWindow", SupportBridgeMethodType::full_closeWindow },
        { "live_location_half.closeWindow", SupportBridgeMethodType::half_closeWindow },
        { "live_location.closeWindow", SupportBridgeMethodType::full_closeWindow },  // �µ�namespace�����淶

        { "live_arouse_pc_link_module", SupportBridgeMethodType::arouse },
        { "live_location_event_report", SupportBridgeMethodType::bsns_report },
        { "live_location_half_report", SupportBridgeMethodType::tec_report },
        { "live_location_polaris_report", SupportBridgeMethodType::polaris_report },
        { "live_network.polarisReport", SupportBridgeMethodType::polaris_report },  // �µ�namespace�����淶
        { "live_location_web_projection", SupportBridgeMethodType::web_projection },
        { "get_pc_link_info"/*"offline.getCookie"*/, SupportBridgeMethodType::get_pc_link_info },
        { "subscribe_pc_broadcast", SupportBridgeMethodType::subscribe_pc_broadcast },
        //{ "live_stream.receiveBroadcast", SupportBridgeMethodType::subscribe_pc_broadcast },  // �µ�namespace�����淶����Ҫimport������Ч
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

    // ��h5�����namespace�¶�Ӧ��bridge���������ӽ�kSupportBridgeMethodNames��
    const std::map<std::string, std::vector<bridge_method>> kSupportBridgeNamespaceMapping {
        { "live_stream", { 
            { "live_stream.receiveBroadcast", SupportBridgeMethodType::subscribe_pc_broadcast },
            { "live_stream.open_in_app", SupportBridgeMethodType::open_in_app }
          }
        },  // �µ�namespace�����淶
    };

    // �������method��������˾ͷ���true
    bool ScheduleCommonBiliJsBridgeMethod(CefRefPtr<CefV8Handler> handler, std::map<std::string, SupportBridgeMethodType>& kSupportBridgeMethodNames, 
        const std::string& method_name, CefRefPtr<CefDictionaryValue> data, CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Value> callback);
}

