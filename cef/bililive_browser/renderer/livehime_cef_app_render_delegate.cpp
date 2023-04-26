#include "stdafx.h"
#include "livehime_cef_app_render.h"

#include "cef_core/include/wrapper/cef_message_router.h"

#include "public/bililive_browser_ipc_messages.h"
#include "public/bililive_browser_js_values.h"
#include "public/bililive_cef_headers.h"

#include "bilibili/livehime_bilibili_render_delegate.h"
#include "biliyun/livehime_biliyun_render_delegate.h"
#include "mini_login/livehime_login_render_viddup_delegate.h"
#include "obs_studio/livehime_obs_studio_render_delegate.h"


namespace
{
    // Must match the value in client_handler.cc.
    const char kFocusedNodeChangedMessage[] = "LivehimeCefAppRenderDelegate.FocusedNodeChanged";

    const char kWindowClose[] = "close";// window.close()

    class LivehimeCefV8ArrayBufferReleaseCallback : public CefV8ArrayBufferReleaseCallback
    {
    public:
        explicit LivehimeCefV8ArrayBufferReleaseCallback(bool need_free)
            : need_free_(need_free)

        {
        }

        void ReleaseBuffer(void* buffer) override
        {
            if (need_free_)
            {
                ::free(buffer);
            }
        }

    private:
        bool need_free_ = false;

        // Include the default reference counting implementation.
        IMPLEMENT_REFCOUNTING(LivehimeCefV8ArrayBufferReleaseCallback);
        DISALLOW_COPY_AND_ASSIGN(LivehimeCefV8ArrayBufferReleaseCallback);
    };

    //class LogV8ValueKeysTask : public CefTask
    //{
    //public:
    //    LogV8ValueKeysTask(CefRefPtr<CefBrowser> browser)
    //        : browser_(browser)
    //    {
    //    }

    //    void Execute() override
    //    {
    //        CefRefPtr<CefV8Context> context = browser_->GetMainFrame()->GetV8Context();
    //        if (context)
    //        {
    //            context->Enter();

    //            CefRefPtr<CefV8Value> globalObj = context->GetGlobal();

    //            DLOG(INFO) << "Print JS Keys Begin ---------------------";
    //            CefString indent;
    //            CefRefPtr<CefV8Value> trace_obj = TraceObject(globalObj, false, "browser", false, indent);
    //            if (trace_obj)
    //            {
    //                indent = indent.ToString() + "  ";
    //                trace_obj = TraceObject(trace_obj, false, "version", true, indent);
    //            }
    //            DLOG(INFO) << "Print JS Keys End ---------------------";

    //            context->Exit();
    //        }
    //    }

    //private:
    //    CefRefPtr<CefV8Value> TraceObject(CefRefPtr<CefV8Value> object, bool print_all_keys,
    //        const CefString& trace_key, bool print_trace_key_subkeys,
    //        const CefString& indent)
    //    {
    //        CefRefPtr<CefV8Value> trace_value;
    //        std::vector<CefString> keys;
    //        if (object->GetKeys(keys))
    //        {
    //            for (auto& iter : keys)
    //            {
    //                if (iter == trace_key)
    //                {
    //                    DLOG(INFO) << indent.ToString() << iter.ToString();

    //                    trace_value = object->GetValue(trace_key);
    //                    if (trace_value)
    //                    {
    //                        if (print_trace_key_subkeys)
    //                        {
    //                            CefString sub_indent = indent.ToString() + indent.ToString();
    //                            std::vector<CefString> sub_keys;
    //                            if (trace_value->GetKeys(sub_keys))
    //                            {
    //                                for (auto& sub_iter : sub_keys)
    //                                {
    //                                    auto sub_value = trace_value->GetValue(sub_iter);
    //                                    if (sub_value->IsBool())
    //                                    {
    //                                        DLOG(INFO) << sub_indent.ToString() << sub_iter.ToString() << " : "
    //                                            << (sub_value->GetBoolValue() ? "true" : "false");
    //                                    }
    //                                    else if (sub_value->IsString())
    //                                    {
    //                                        DLOG(INFO) << sub_indent.ToString() << sub_iter.ToString() << " : "
    //                                            << sub_value->GetStringValue().ToString();
    //                                    }
    //                                    else if (sub_value->IsInt())
    //                                    {
    //                                        DLOG(INFO) << sub_indent.ToString() << sub_iter.ToString() << " : "
    //                                            << sub_value->GetIntValue();
    //                                    }
    //                                    else if (sub_value->IsDouble())
    //                                    {
    //                                        DLOG(INFO) << sub_indent.ToString() << sub_iter.ToString() << " : "
    //                                            << sub_value->GetDoubleValue();
    //                                    }
    //                                    else if (sub_value->IsArray())
    //                                    {
    //                                        DLOG(INFO) << sub_indent.ToString() << sub_iter.ToString() << " is Array.";
    //                                    }
    //                                }
    //                            }
    //                        }
    //                    }

    //                    if (!print_all_keys)
    //                    {
    //                        break;
    //                    }
    //                }
    //                else
    //                {
    //                    if (print_all_keys)
    //                    {
    //                        DLOG(INFO) << indent.ToString() << iter.ToString();
    //                    }
    //                }
    //            }
    //        }
    //        return trace_value;
    //    }

    //private:
    //    CefRefPtr<CefBrowser> browser_;

    //    IMPLEMENT_REFCOUNTING(LogV8ValueKeysTask);
    //    DISALLOW_COPY_AND_ASSIGN(LogV8ValueKeysTask);
    //};

    class LivehimeCefAppRenderDelegate :
        public LivehimeCefAppRender::Delegate,
        public CefV8Handler
    {
    public:
        LivehimeCefAppRenderDelegate() = default;
        virtual ~LivehimeCefAppRenderDelegate() = default;

        // LivehimeCefAppRender::Delegate
        void OnWebKitInitialized(CefRefPtr<LivehimeCefAppRender> app) override
        {
            // Create the renderer-side router for query handling.
            CefMessageRouterConfig config;
            message_router_ = CefMessageRouterRendererSide::Create(config);
        }

        void OnContextCreated(CefRefPtr<LivehimeCefAppRender> app,
                              CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              CefRefPtr<CefV8Context> context) override
        {
            CefRefPtr<CefV8Value> globalObj = context->GetGlobal();

#ifdef _DEBUG
            // for test[
            CefRefPtr<CefV8Value> func = CefV8Value::CreateFunction(js_values::kLivehimeCefJsMsgForTest, this);
            globalObj->SetValue(js_values::kLivehimeCefJsMsgForTest, func, V8_PROPERTY_ATTRIBUTE_NONE);
            // ]
#endif
            message_router_->OnContextCreated(browser, frame, context);

            // 重写js的window.close，不允许其按默认流程关闭，因为默认关闭之后CEF底层会清除掉一些资源，
            // （猜测是frame相关的，会关掉相关的render进程，如果再次在该CefBrowser上LoadUrl则引发Layer的崩溃）
            // 暂未找到优雅的解决方法，直接覆写window.close算了
            CefRefPtr<CefV8Value> close_func = CefV8Value::CreateFunction(kWindowClose, this);
            globalObj->SetValue(kWindowClose, close_func, V8_PROPERTY_ATTRIBUTE_NONE);
        }

        void OnContextReleased(CefRefPtr<LivehimeCefAppRender> app,
                               CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               CefRefPtr<CefV8Context> context) override
        {
            message_router_->OnContextReleased(browser, frame, context);
        }

        void OnFocusedNodeChanged(CefRefPtr<LivehimeCefAppRender> app,
                                  CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefDOMNode> node) override
        {
            bool is_editable = (node.get() && node->IsEditable());
            if (is_editable != last_node_is_editable_ && frame)
            {
                // Notify the browser of the change in focused element type.
                last_node_is_editable_ = is_editable;
                CefRefPtr<CefProcessMessage> message =
                    CefProcessMessage::Create(kFocusedNodeChangedMessage);
                message->GetArgumentList()->SetBool(0, is_editable);
                frame->SendProcessMessage(PID_BROWSER, message);
            }
        }

        bool OnProcessMessageReceived(CefRefPtr<LivehimeCefAppRender> app,
            CefRefPtr<CefBrowser> browser,
            CefRefPtr<CefFrame> frame,
            CefProcessId source_process,
            CefRefPtr<CefProcessMessage> message) override
        {
            bool handled = false;
            std::string message_name = message->GetName();
            if (message_name == ipc_messages::kLivehimeCefExexuteJsFunction)
            {
                CefRefPtr<CefListValue> args = message->GetArgumentList();

                size_t args_size = args->GetSize();
                if(args_size >= 1)
                {
                    CefString func_name = args->GetString(0);

                    if(!func_name.empty())
                    {
                        CefRefPtr<CefV8Context> context =
                            browser->GetMainFrame()->GetV8Context();

                        context->Enter();

                        CefRefPtr<CefV8Value> window_obj = context->GetGlobal();
                        CefRefPtr<CefV8Value> jsFunction = window_obj->GetValue(func_name);

                        if (jsFunction && jsFunction->IsFunction())
                        {
                            CefV8ValueList arguments;
                            for (size_t i = 1; i < args_size; ++i)
                            {
                                // 目前直接调用js而不通过具体业务对象额外处理的话我们只支持以下六种类型（2019/03/06）：
                                // VTYPE_NULL,
                                // VTYPE_BOOL,
                                // VTYPE_INT,
                                // VTYPE_DOUBLE,
                                // VTYPE_STRING,
                                // VTYPE_BINARY,
                                switch (args->GetType(i))
                                {
                                case CefValueType::VTYPE_NULL:
                                    arguments.push_back(CefV8Value::CreateNull());
                                    break;
                                case CefValueType::VTYPE_BOOL:
                                    arguments.push_back(CefV8Value::CreateBool(args->GetBool(i)));
                                    break;
                                case CefValueType::VTYPE_INT:
                                    arguments.push_back(CefV8Value::CreateInt(args->GetInt(i)));
                                    break;
                                case CefValueType::VTYPE_DOUBLE:
                                    arguments.push_back(CefV8Value::CreateDouble(args->GetDouble(i)));
                                    break;
                                case CefValueType::VTYPE_STRING:
                                    arguments.push_back(CefV8Value::CreateString(args->GetString(i)));
                                    break;
                                case CefValueType::VTYPE_BINARY:
                                {
                                    CefRefPtr<CefV8ArrayBufferReleaseCallback> release_callback =
                                        new LivehimeCefV8ArrayBufferReleaseCallback(true);
                                    CefRefPtr<CefBinaryValue> bv = args->GetBinary(i);
                                    void* data = ::malloc(bv->GetSize());
                                    bv->GetData(data, bv->GetSize(), 0);
                                    arguments.push_back(CefV8Value::CreateArrayBuffer(data, bv->GetSize(), release_callback));
                                }
                                break;
                                default:
                                    NOTREACHED() << "unsupport value type!";
                                    break;
                                }
                            }
                            jsFunction->ExecuteFunction(nullptr, arguments);

                            handled = true;
                        }

                        context->Exit();
                    }
                }
            }

            if (handled)
            {
                return true;
            }

            return message_router_->OnProcessMessageReceived(browser, browser->GetMainFrame(), source_process, message);
        }

        // CefV8Handler
        bool Execute(const CefString& name,
                     CefRefPtr<CefV8Value> object,
                     const CefV8ValueList& arguments,
                     CefRefPtr<CefV8Value>& retval,
                     CefString& exception) override
        {
            if (name == kWindowClose)
            {
                // render虽然不处理window.close，但是应该把这个发给browser，由其决定是否进行相应处理
                CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();

                LOG(INFO) << "[LB] window.close invoked, page=" << browser->GetMainFrame()->GetURL().ToString();

                CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kLivehimeCefOnWindowClose);
                SendBrowserProcessMessage(browser, PID_BROWSER, msg);

                return true;
            }
#ifdef _DEBUG
            // for test [
            else if (name == js_values::kLivehimeCefJsMsgForTest)
            {
                CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kLivehimeCefJsMsgForTest);
                CefRefPtr<CefListValue> args = msg->GetArgumentList();
                //args->SetInt(0, callbackId);
                retval = CefV8Value::CreateString("native return value to js: \"hello world!\"");

                CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
                SendBrowserProcessMessage(browser, PID_BROWSER, msg);
                return true;
            }
            // ]
#endif

            return false;
        }

    private:
        bool last_node_is_editable_ = false;
        // Handles the renderer side of query routing.
        CefRefPtr<CefMessageRouterRendererSide> message_router_;

        // Include the default reference counting implementation.
        IMPLEMENT_REFCOUNTING(LivehimeCefAppRenderDelegate);
        DISALLOW_COPY_AND_ASSIGN(LivehimeCefAppRenderDelegate);
    };
}


void LivehimeCefAppRender::AddDelegates(DelegateSet& delegates)
{
    delegates.insert(new LivehimeCefAppRenderDelegate());

    // extension
    delegates.insert(new LivehimeBilibiliRenderDelegate());// B站页面公共事件处理
    delegates.insert(new LivehimeOBSStudioRenderDelegate());// obs-studio
    delegates.insert(new LivehimeBiliyunRenderDelegate()); // biliyun上传
    delegates.insert(new LivehimeLoginRenderViddupDelegate()); // mini登录框-国际化

}
