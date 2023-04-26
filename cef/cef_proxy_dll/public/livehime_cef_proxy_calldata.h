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

            dev_tools,// ���Թ��ߵ�������

            // ��ָ������ҵ��ģ���Bվҳ�棬������������ҳ��������дCefClientBrowserDelegate������ӦJS��ͳһ֪ͨ��
            // ��رա�������ɡ�ҵ����㡢ģ�黽���
            bilibili,

            // ����
            geetest,

            // ������֤
            sec_sign_verify,

            // ��������
            lucky_gift,
            lucky_gift_award,

			// �����齱
			lottery,

            // biliyun��Դ�ϴ�
            biliyun,

            // mini��¼��
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
            // ����ص����ͣ���ǰ��h5����һ�£�h5��ʷ��ƵĿӣ���ͳһ���ϲ����ҵ�񳡾�������
            // �ֶ���Ϊ��kCallbackTypeField

            ///
            // ҵ����յ�bridge�󣬲���Ҫ�ص����ݣ������������Դ��ʼ���������ΪӦ�𣬱�����ֻ�ܵ���һ��
            // ����Ϊҵ���ͬ��������bridge������û�лص���Ϊ�����ͣ��磺toast��
            ///
            CALL_BACK_TYPE_CALLBACK = 0,

            ///
            // ҵ����յ�bridge����Ҫ�ص����ݣ��õ�������ݺ���лص�����CALL_BACK_TYPE_CALLBACK���⣬��������һ������
            // ����Ϊҵ���ͬ��������bridge��������callback�ص���Ϊ�����ͣ��磺get_pc_link_info��
            ///
            CALL_BACK_TYPE_CALLBACK_DATA,

            ///
            // ҵ������bridgeӦ����ں���������Ҫ�ص����ݣ����Ե��ö��
            // ����Ϊҵ����첽������bridge��������success�ص���Ϊ�����ͣ��磺alert��
            ///
            CALL_BACK_TYPE_SUCCESS_DATA,
        };

        enum class calldata_type
        {
            // ��CefValueType��Ӧ�������ϲ���proxy�����native/JavaScript����ʱ������IPC��Ϣ����
            // CefProcessMessage���в������
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

            // ����bridge�������list����֧��
            std::list<std::map<std::string, calldata_filed>> list_;
        };

        typedef enum
        {
            // ����״̬���أ�0���رգ�1������
            OFFLINE_STATUS_CLOSE = 0,
            OFFLINE_STATUS_OPEN = 1,
        } offline_statue_type_t;

        struct offline_config_filed
        {
            std::string module_name;
            int offline_status = OFFLINE_STATUS_CLOSE;
            std::string zip_url;
            std::string page_url;   // discard��pageUrl����ڶ�ҳӦ�õ���������԰�pageUrl�����html�ļ��Ķ�Ӧ��ϵֱ�ӷŵ���files�ֶ���
            std::string hash;
            std::string build_time;
            std::string url;
            std::wstring file_path;
            std::string mime_type;
        };

        typedef enum
        {
            EVENT_OFFLINE_INTERCEPT_FAILED = 0,  // webview���߻�����ʧ��
            EVENT_WEBVIEW_USE_OFFLINE,           // webviewʹ��������Դ
            EVENT_WEBVIEW_USE_ONLINE,            // webviewʹ��������Դ
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
