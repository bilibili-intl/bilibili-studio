#pragma once
#include "livehime_cef_proxy_define.h"
#include "livehime_cef_proxy_calldata.h"
#include "livehime_cef_proxy_constants.h"

#include <functional>

namespace cef_proxy
{
    /* ------------------------------------------------------------------ */
    // CEF-Proxy模块的公共通知
    using cef_proxy_callback_init_status_fn = void(*)(bool success, cef_proxy::CefInitResult code);
    using cef_proxy_callback_common_event = void(*)(const std::string& msg_name, const cef_proxy::calldata* data);
    // 特定WEB模块的通知
    using cef_proxy_callback_msg_received_fn = void(*)(
        const cef_proxy::browser_bind_data& bind_data, const std::string& msg_name, const cef_proxy::calldata* data);
    // shutdown状态
    using cef_proxy_callback_shutdown_status_fn = void(*)(bool success);

    struct cef_proxy_callback
    {
        // 初始化状态
        cef_proxy_callback_init_status_fn init_status_fn = nullptr;
        // 设置cookie等与特定页面无关的公共通知
        cef_proxy_callback_common_event common_event_fn = nullptr;
        // js回调通知
        cef_proxy_callback_msg_received_fn msg_received_fn = nullptr;
        // shutdown状态
        cef_proxy_callback_shutdown_status_fn shutdown_status_fn = nullptr;
    };

    /* ------------------------------------------------------------------ */

    using cef_proxy_init_module_fn = bool(*)();

    using cef_proxy_shutdown_module_fn = void(*)();

    using cef_proxy_set_version_fn = void(*)(const std::string& version, const std::string& device_name, const std::string& device_platform, const std::string& buvid3);

    using cef_proxy_set_cookie_fn = bool(*)(int set_cookie_task_id, const std::string& cookies, const std::string& domains);
    using cef_proxy_set_cef_cookie_fn = bool(*)(int set_cookie_task_id, const cef_proxy::cookies& cookies);
    using cef_proxy_delete_cookie_fn = bool(*)();

    using cef_proxy_register_callback_fn = void(*)(const cef_proxy_callback& callbacks);

    using cef_proxy_execute_js_function_fn = bool(*)(const cef_proxy::browser_bind_data& bind_data, const std::string& func_name,
        const cef_proxy::calldata_list* const arguments);

    using cef_proxy_execute_js_function_with_kv_fn = bool(*)(const cef_proxy::browser_bind_data& bind_data, const std::string& func_name,
        const cef_proxy::calldata* const arguments);

    // execute bilibili jsbridge callback
    using cef_proxy_dispatch_js_event_fn = bool(*)(const cef_proxy::browser_bind_data& bind_data,
        const std::string& ipc_msg_name, const cef_proxy::calldata* const arguments);

    using cef_proxy_load_url_fn = bool(*)(const cef_proxy::browser_bind_data& bind_data, const std::string& url);

    using cef_proxy_create_popup_browser_fn = bool(*)(const browser_bind_data& bind_data, HWND par_hwnd, const std::string& url,
        const std::string& post_data, const std::string& headers);

    using cef_proxy_create_embedded_browser_fn = bool(*)(const browser_bind_data& bind_data, HWND par_hwnd, const RECT& rc, const std::string& url,
        const std::string& post_data, const std::string& headers);

    using cef_proxy_send_mouse_click_fn = void(*)(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent &event, cef_proxy::MouseButtonType type, bool mouse_up, uint32_t click_count);
    using cef_proxy_send_mouse_move_fn = void(*)(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, bool mouse_leave);
    using cef_proxy_send_mouse_wheel_fn = void(*)(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, int x_delta, int y_delta);
    using cef_proxy_send_focus_fn = void(*)(const cef_proxy::browser_bind_data& bind_data, bool focus);
    using cef_proxy_send_key_click_fn = void(*)(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::KeyEvent& event);
    using cef_proxy_ime_commit_text_fn = void(*)(const cef_proxy::browser_bind_data& bind_data,
        const std::wstring& text,
        int from_val,
        int to_val,
        int relative_cursor_pos);
    using cef_proxy_ime_finish_composing_text_fn = void(*)(const cef_proxy::browser_bind_data& bind_data, bool keep_selection);
    using cef_proxy_ime_cancel_composition_fn = void(*)(const cef_proxy::browser_bind_data& bind_data);

    // data in format of BGRA
    using cef_proxy_osr_onpaint = std::function<void(const void* buffer, int width, int height)>;
    using cef_proxy_create_osr_browser_fn = bool(*)(const browser_bind_data& bind_data, const RECT& rc, int fps, cef_proxy_osr_onpaint cb,
        const std::string& url, const std::string& post_data, const std::string& headers);

    using cef_proxy_close_browser_fn = void(*)(const browser_bind_data& bind_data);

    using cef_proxy_resize_browser_fn = void(*)(const browser_bind_data& bind_data, const RECT& bounds);

    using cef_proxy_repaint_browser_fn = void(*)(const browser_bind_data& bind_data);

    using cef_proxy_update_offline_switch_fn = void(*)(bool offline_switch);

    using cef_proxy_update_offline_config_fn = void(*)(const cef_proxy::offline_config* const configs, const cef_proxy::offline_config_callback_fn callback);
}