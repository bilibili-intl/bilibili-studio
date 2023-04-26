#pragma once

#include <string>
#include <map>
#include <vector>
#include <list>

#ifdef __cplusplus
extern "C" {
#endif

    namespace cef_proxy
    {
        const char kCallbackTypeField[] = "callback_type";

        enum class client_handler_type
        {
            unspecified,

            dev_tools,// 调试工具单独弹窗

            // 不指定具体业务模块的B站页面，便于弱交互的页面无需重写CefClientBrowserDelegate即可响应JS的统一通知，
            // 如关闭、加载完成、业务埋点、模块唤起等
            bilibili,

            // 极验
            geetest,

            // 二次验证
            sec_sign_verify,

            // 幸运礼物
            lucky_gift,
            lucky_gift_award,

			// 主播抽奖
			lottery,

            // biliyun资源上传
            biliyun,

            // mini登录框
            mini_login,
        };

        struct browser_bind_data
        {
            browser_bind_data(__int64 id, client_handler_type type = client_handler_type::unspecified)
            {
                this->type = type;
                this->id = id;
            }

            browser_bind_data()
            {
            }

            client_handler_type type = client_handler_type::unspecified;
            __int64 id = -1;

            bool operator==(const browser_bind_data &rhs) const
            {
                return (this->type == rhs.type && this->id == rhs.id);
            }
        };

        // like _cef_cookie_t
        struct cookie_t
        {
            ///
            // The cookie name.
            ///
            std::string name;

            ///
            // The cookie value.
            ///
            std::string value;

            ///
            // If |domain| is empty a host cookie will be created instead of a domain
            // cookie. Domain cookies are stored with a leading "." and are visible to
            // sub-domains whereas host cookies are not.
            ///
            std::string domain;

            ///
            // If |path| is non-empty only URLs at or below the path will get the cookie
            // value.
            ///
            std::string path;

            ///
            // If |secure| is true the cookie will only be sent for HTTPS requests.
            ///
            int secure = 0;

            ///
            // If |httponly| is true the cookie will only be sent for HTTP requests.
            ///
            int httponly = 0;

            ///
            // The cookie creation date. This is automatically populated by the system on
            // cookie creation.
            ///
            time_t creation = 0;

            ///
            // The cookie last access date. This is automatically populated by the system
            // on access.
            ///
            time_t last_access = 0;

            ///
            // The cookie expiration date is only valid if |has_expires| is true.
            ///
            int has_expires = 0;
            time_t expires = 0;
        };
        typedef std::vector<cookie_t> cookies;

        // like cef_key_event_type_t
        typedef enum
        {
            ///
            // Notification that a key transitioned from "up" to "down".
            ///
            KEYEVENT_RAWKEYDOWN = 0,

            ///
            // Notification that a key was pressed. This does not necessarily correspond
            // to a character depending on the key and language. Use KEYEVENT_CHAR for
            // character input.
            ///
            KEYEVENT_KEYDOWN,

            ///
            // Notification that a key was released.
            ///
            KEYEVENT_KEYUP,

            ///
            // Notification that a character was typed. Use this for text input. Key
            // down events may generate 0, 1, or more than one character event depending
            // on the key, locale, and operating system.
            ///
            KEYEVENT_CHAR
        } key_event_type_t;

        // like cef_event_flags_t
        typedef enum
        {
            EVENTFLAG_NONE = 0,
            EVENTFLAG_CAPS_LOCK_ON = 1 << 0,
            EVENTFLAG_SHIFT_DOWN = 1 << 1,
            EVENTFLAG_CONTROL_DOWN = 1 << 2,
            EVENTFLAG_ALT_DOWN = 1 << 3,
            EVENTFLAG_LEFT_MOUSE_BUTTON = 1 << 4,
            EVENTFLAG_MIDDLE_MOUSE_BUTTON = 1 << 5,
            EVENTFLAG_RIGHT_MOUSE_BUTTON = 1 << 6,
            // Mac OS-X command key.
            EVENTFLAG_COMMAND_DOWN = 1 << 7,
            EVENTFLAG_NUM_LOCK_ON = 1 << 8,
            EVENTFLAG_IS_KEY_PAD = 1 << 9,
            EVENTFLAG_IS_LEFT = 1 << 10,
            EVENTFLAG_IS_RIGHT = 1 << 11,
        } event_flags_t;

        // like _cef_key_event_t
        struct key_event_t
        {
            ///
            // The type of keyboard event.
            ///
            key_event_type_t type;

            ///
            // Bit flags describing any pressed modifier keys. See
            // event_flags_t for values.
            ///
            unsigned int  modifiers = 0;

            ///
            // The Windows key code for the key event. This value is used by the DOM
            // specification. Sometimes it comes directly from the event (i.e. on
            // Windows) and sometimes it's determined using a mapping function. See
            // WebCore/platform/chromium/KeyboardCodes.h for the list of values.
            ///
            int windows_key_code = 0;

            ///
            // The actual key code genenerated by the platform.
            ///
            int native_key_code = 0;

            ///
            // Indicates whether the event is considered a "system key" event (see
            // http://msdn.microsoft.com/en-us/library/ms646286(VS.85).aspx for details).
            // This value will always be false on non-Windows platforms.
            ///
            int is_system_key = 0;

            ///
            // The character generated by the keystroke.
            ///
            wchar_t character = 0;

            ///
            // Same as |character| but unmodified by any concurrently-held modifiers
            // (except shift). This is useful for working out shortcut keys.
            ///
            wchar_t unmodified_character = 0;

            ///
            // True if the focus is currently on an editable field on the page. This is
            // useful for determining if standard key events should be intercepted.
            ///
            int focus_on_editable_field = 0;
        };

        enum class callback_type
        {
            // 定义回调类型，与前端h5保持一致（h5历史设计的坑），统一由上层根据业务场景来调用
            // 字段名为：kCallbackTypeField

            ///
            // 业务侧收到bridge后，不需要回调数据，可以在相关资源初始化后调用作为应答，必须且只能调用一次
            // 定义为业务侧同步场景（bridge定义中没有回调的为该类型，如：toast）
            ///
            CALL_BACK_TYPE_CALLBACK = 0,

            ///
            // 业务侧收到bridge后，需要回调数据，拿到相关数据后进行回调，和CALL_BACK_TYPE_CALLBACK互斥，调用其中一个即可
            // 定义为业务侧同步场景（bridge定义中有callback回调的为该类型，如：get_pc_link_info）
            ///
            CALL_BACK_TYPE_CALLBACK_DATA,

            ///
            // 业务侧进行bridge应答后，在后续场景需要回调数据，可以调用多次
            // 定义为业务侧异步场景（bridge定义中有success回调的为该类型，如：alert）
            ///
            CALL_BACK_TYPE_SUCCESS_DATA,
        };

        enum class calldata_type
        {
            // 与CefValueType对应，便于上层与proxy层进行native/JavaScript互调时构建的IPC消息对象
            // CefProcessMessage进行参数填充
            CALL_DATA_TYPE_INVALID,
            CALL_DATA_TYPE_NULL,
            CALL_DATA_TYPE_BOOL,
            CALL_DATA_TYPE_INT,
            CALL_DATA_TYPE_DOUBLE,
            CALL_DATA_TYPE_STRING,
            CALL_DATA_TYPE_BINARY,
            CALL_DATA_TYPE_VOID,

            // bililive
            CALL_DATA_TYPE_COOKIES,
            CALL_DATA_TYPE_BIND_DATA,
            CALL_DATA_TYPE_KEY_EVENT,

            // CEF CefValue type
            /*VTYPE_INVALID,
            VTYPE_NULL,
            VTYPE_BOOL,
            VTYPE_INT,
            VTYPE_DOUBLE,
            VTYPE_STRING,
            VTYPE_BINARY,
            VTYPE_DICTIONARY,
            VTYPE_LIST,*/

            CALL_DATA_TYPE_WSTRING,
        };

        struct calldata_filed
        {
            calldata_filed()
            {
                reset();
            }

            void reset()
            {
                type = calldata_type::CALL_DATA_TYPE_INVALID;
                numeric_union.double_ = 0;
                str_.clear();
                wstr_.clear();
                ZeroMemory(&key_event_, sizeof(key_event_t));
                list_.clear();
            }

            calldata_type type;
            union
            {
                bool bool_;
                int int_;
                double double_;
                void* void_;
                int64_t int64_;
            }numeric_union;
            std::string str_;
            std::wstring wstr_;
            key_event_t key_event_;
            cookies cookies_;
            browser_bind_data browser_bind_data_;

            // 增加bridge请求参数list数组支持
            std::list<std::map<std::string, calldata_filed>> list_;
        };

        typedef enum
        {
            // 离线状态开关，0：关闭，1：开启
            OFFLINE_STATUS_CLOSE = 0,
            OFFLINE_STATUS_OPEN = 1,
        } offline_statue_type_t;

        struct offline_config_filed
        {
            std::string module_name;
            int offline_status = OFFLINE_STATUS_CLOSE;
            std::string zip_url;
            std::string page_url;   // discard，pageUrl会存在多页应用的情况，所以把pageUrl及其跟html文件的对应关系直接放到了files字段中
            std::string hash;
            std::string build_time;
            std::string url;
            std::wstring file_path;
            std::string mime_type;
        };

        typedef enum
        {
            EVENT_OFFLINE_INTERCEPT_FAILED = 0,  // webview离线化拦截失败
            EVENT_WEBVIEW_USE_OFFLINE,           // webview使用离线资源
            EVENT_WEBVIEW_USE_ONLINE,            // webview使用线上资源
        } offline_callback_enevt_t;

        typedef std::map<std::string, calldata_filed> calldata;
        typedef std::list<calldata_filed> calldata_list;
        typedef std::map<std::string, offline_config_filed> offline_config;
        using offline_config_callback_fn = void(*)(const offline_callback_enevt_t& event, const std::string& msg, const cef_proxy::offline_config_filed& filed);

        // get funcs -------------------------------------------------------------------------
        static inline bool calldata_bool(const calldata* data, const char *name)
        {
            bool val = false;
            if (data && data->find(name) != data->end())
            {
                const calldata_filed& filed = data->at(name);
                if (filed.type == calldata_type::CALL_DATA_TYPE_BOOL)
                {
                    val = filed.numeric_union.bool_;
                }
            }
            return val;
        }

        static inline int calldata_int(const calldata* data, const char *name)
        {
            int val = 0;
            if (data && data->find(name) != data->end())
            {
                const calldata_filed& filed = data->at(name);
                if (filed.type == calldata_type::CALL_DATA_TYPE_INT)
                {
                    val = filed.numeric_union.int_;
                }
            }
            return val;
        }

        static inline double calldata_double(const calldata* data, const char *name)
        {
            double val = 0.0;
            if (data && data->find(name) != data->end())
            {
                const calldata_filed& filed = data->at(name);
                if (filed.type == calldata_type::CALL_DATA_TYPE_DOUBLE)
                {
                    val = filed.numeric_union.double_;
                }
            }
            return val;
        }

        static inline const char* calldata_string(const calldata* data,
                                                   const char *name)
        {
            const char * val = nullptr;
            if (data && data->find(name) != data->end())
            {
                const calldata_filed& filed = data->at(name);
                if (filed.type == calldata_type::CALL_DATA_TYPE_STRING)
                {
                    val = filed.str_.c_str();
                }
            }
            return val;
        }

        static inline void *calldata_ptr(const calldata* data, const char *name)
        {
            void *val = nullptr;
            if (data && data->find(name) != data->end())
            {
                const calldata_filed& filed = data->at(name);
                if (filed.type == calldata_type::CALL_DATA_TYPE_VOID)
                {
                    val = filed.numeric_union.void_;
                }
            }
            return val;
        }

        // ------------------------------------------------------------------------------------

    }

#ifdef __cplusplus
}
#endif
