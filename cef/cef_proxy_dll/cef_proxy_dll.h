
// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� CEF_PROXY_DLL_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// CEF_PROXY_DLL_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef CEF_PROXY_DLL_EXPORTS
#define CEF_PROXY_DLL_API __declspec(dllexport)
#else
#define CEF_PROXY_DLL_API __declspec(dllimport)
#endif

#include "public/livehime_cef_proxy_exports.h"
//// �����Ǵ� cef_proxy_dll.dll ������
//class CEF_PROXY_DLL_API Ccef_proxy_dll {
//public:
//	Ccef_proxy_dll(void);
//	// TODO:  �ڴ�������ķ�����
//};
//
//extern CEF_PROXY_DLL_API int ncef_proxy_dll;
//
//CEF_PROXY_DLL_API int fncef_proxy_dll(void);

extern "C"
{
    CEF_PROXY_DLL_API void cef_proxy_init_module(void);

    CEF_PROXY_DLL_API void cef_proxy_shutdown_module(void);

    CEF_PROXY_DLL_API void cef_proxy_set_version(const std::string& version, const std::string &device_name, const std::string &device_platform,  const std::string& buvid3);

    CEF_PROXY_DLL_API bool cef_proxy_set_cookie(int set_cookie_task_id, const std::string& cookies, const std::string& domains);
    CEF_PROXY_DLL_API bool cef_proxy_set_cef_cookie(int set_cookie_task_id, const cef_proxy::cookies& cookies);
    CEF_PROXY_DLL_API bool cef_proxy_delete_cookie();

    // ע��proxy���ϲ�Ļص�
    CEF_PROXY_DLL_API void cef_proxy_register_callback(const cef_proxy::cef_proxy_callback& callbacks);

    // ָ����ҳ��ִ��js����
    CEF_PROXY_DLL_API bool cef_proxy_execute_js_function(const cef_proxy::browser_bind_data& bind_data, const std::string& func_name,
                                                         const cef_proxy::calldata_list* const arguments);

    CEF_PROXY_DLL_API bool cef_proxy_execute_js_function_with_kv(const cef_proxy::browser_bind_data& bind_data, const std::string& func_name,
        const cef_proxy::calldata* const arguments);

    CEF_PROXY_DLL_API bool cef_proxy_dispatch_js_event(const cef_proxy::browser_bind_data& bind_data,
        const std::string& ipc_msg_name, const cef_proxy::calldata* const arguments);

    CEF_PROXY_DLL_API bool cef_proxy_load_url(const cef_proxy::browser_bind_data& bind_data, const std::string& url);

    CEF_PROXY_DLL_API bool cef_proxy_create_popup_browser(const cef_proxy::browser_bind_data& bind_data, HWND par_hwnd, const std::string& url,
                                                          const std::string& post_data = "", const std::string& headers = "");

    CEF_PROXY_DLL_API bool cef_proxy_create_embedded_browser(const cef_proxy::browser_bind_data& bind_data, HWND par_hwnd, const RECT& rc, const std::string& url,
                                                             const std::string& post_data = "", const std::string& headers = "");

    CEF_PROXY_DLL_API bool cef_proxy_create_osr_browser(const cef_proxy::browser_bind_data& bind_data, const RECT& rc,
        int fps, cef_proxy::cef_proxy_osr_onpaint cb,
        const std::string& url,
        const std::string& post_data = "", const std::string& headers = "");
     
	CEF_PROXY_DLL_API void cef_proxy_send_mouse_click(const cef_proxy::browser_bind_data& bind_data, 
		const cef_proxy::MouseEvent& event, cef_proxy::MouseButtonType type, bool mouse_up, uint32_t click_count);

    CEF_PROXY_DLL_API void cef_proxy_send_mouse_move(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, bool mouse_leave);

    CEF_PROXY_DLL_API void cef_proxy_send_mouse_wheel(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, int x_delta, int y_delta);

    CEF_PROXY_DLL_API void cef_proxy_send_focus(const cef_proxy::browser_bind_data& bind_data, bool focus);

    CEF_PROXY_DLL_API void cef_proxy_send_key_click(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::KeyEvent& event);

    CEF_PROXY_DLL_API void cef_proxy_ime_commit_text(const cef_proxy::browser_bind_data& bind_data,
        const std::wstring& text,
        int from_val,
        int to_val,
        int relative_cursor_pos);

    CEF_PROXY_DLL_API void cef_proxy_ime_finish_composing_text(const cef_proxy::browser_bind_data& bind_data, bool keep_selection);

    CEF_PROXY_DLL_API void cef_proxy_ime_cancel_composition(const cef_proxy::browser_bind_data& bind_data);

    CEF_PROXY_DLL_API void cef_proxy_close_browser(const cef_proxy::browser_bind_data& bind_data);

    CEF_PROXY_DLL_API void cef_proxy_resize_browser(const cef_proxy::browser_bind_data& bind_data, const RECT& bounds);

    CEF_PROXY_DLL_API void cef_proxy_repaint_browser(const cef_proxy::browser_bind_data& bind_data);

    CEF_PROXY_DLL_API void cef_proxy_update_offline_switch(bool offline_switch);

    CEF_PROXY_DLL_API void cef_proxy_update_offline_config(const cef_proxy::offline_config* const configs, const cef_proxy::offline_config_callback_fn callback);
}

namespace internal
{
    void BrowserManagerThread();
    bool IsCefInitializeSucceeded();
}