#include "stdafx.h"
#include "livehime_biliyun_render_delegate.h"

#include <fstream>

#include "bilibase/basic_types.h"

#include "bililive_browser/public/bililive_browser_js_values.h"
#include "bililive_browser/public/bililive_browser_ipc_messages.h"

#include "cef_proxy_dll/public/livehime_cef_proxy_calldata.h"

#include "public/bililive_cef_headers.h"
#include "util/bililive_browser_util.h"

namespace
{
    // 创建SDK的上传任务时SDK并不会在上传函数直接返回其内部对于此次上传的任务id，
    // 所以我们在这里自行记录业务上层
    int64_t g_current_task_id = -1;

    const char kModuleObject[] = "livehime_biliyun";

    class BiliyunArrayBufferReleaseCallback : public CefV8ArrayBufferReleaseCallback
    {
    public:
        BiliyunArrayBufferReleaseCallback(std::string&& file_data)
            : file_data_(file_data)
        {
        }

        void ReleaseBuffer(void* buffer) override
        {
            // free file data
            LOG(INFO) << "[biliyun] free file data.";
        }

        void* data()
        {
            return (void*)file_data_.data();
        }

        size_t length() const
        {
            return file_data_.length();
        }

    private:
        std::string file_data_;

        // Include the default reference counting implementation.
        IMPLEMENT_REFCOUNTING(BiliyunArrayBufferReleaseCallback);
        DISALLOW_COPY_AND_ASSIGN(BiliyunArrayBufferReleaseCallback);
    };

    // 注册几个我们在乎的事件钩子
    enum BiliyunEventType
    {
        BeforePreUpload,// box
        BeforeUpload,   // box
        UploadProgress, // box
        BeforeFileUploaded, // box
        FileUploaded,   // box
        UploadError,    // error
    };
    const std::map<BiliyunEventType, std::string> kSupportJsFunctionNames{
        { BeforePreUpload, "BeforePreUpload"},
        { BeforeUpload, "BeforeUpload"},
        { UploadProgress, "UploadProgress"},
        { BeforeFileUploaded, "BeforeFileUploaded"},
        { FileUploaded, "FileUploaded"},
        { UploadError, "UploadError"},
    };

    CefString GetBiliyunBoxJsonString(CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Value> value)
    {
        // box文件容器对象
        // const box = {
        //   "id": String, // 文件容器的标记符 "BUploader_*" 唯一
        //   "blob" : Blob, // 源文件Blob对象
        //   "blobSize" : Number, // 源文件字节大小 byte
        //   "name" : String, // 源文件文件本地文件名
        //   "ext" : String, // 文件尾缀 "mp4 flv ..."
        //   "chunks" : [Blob, Blob] , // 分片的Blob列表，每一个元素表示分片的Blob
        //   "chunkSize" : Number, // 每段分片的字节数 byte
        //   "percentage" : Number, // 上传百分比进度 双位数 90 = 90%
        //   "uploadBytesPerSecond" : Number, // 每秒钟上传的字节数 byte
        //   "blobUploaded" : Number, // 已经上传成功的字节 byte
        // 
        //   // 云端同步数据，"BeforeUpload" 事件钩子中以及以后存在
        //   "hookSet" : {
        //     "bili_filename": String // 存储 filename 唯一
        //     "upos_uri" : 'upos://{bucket_path}/{filename}.{ext}', // 存储 bucket
        //     "biz_id" : String // 业务 cid 唯一
        //   },
        // 
        //   "profile": String, // 私有单个队列的，x 视频云 profile
        //   "uposCompleteQuery" : {}, // v2.5.5新增，upos协议，完成接口补充的query参数
        // }
        CefString msg_str;
        if (value->IsObject())
        {
            CefRefPtr<CefV8Value> id = value->GetValue("id");
            CefRefPtr<CefV8Value> blobSize = value->GetValue("blobSize");
            CefRefPtr<CefV8Value> name = value->GetValue("name");
            CefRefPtr<CefV8Value> ext = value->GetValue("ext");
            CefRefPtr<CefV8Value> chunkSize = value->GetValue("chunkSize");
            CefRefPtr<CefV8Value> percentage = value->GetValue("percentage");
            CefRefPtr<CefV8Value> uploadBytesPerSecond = value->GetValue("uploadBytesPerSecond");
            CefRefPtr<CefV8Value> blobUploaded = value->GetValue("blobUploaded");

            CefRefPtr<CefV8Value> bili_filename;
            CefRefPtr<CefV8Value> upos_uri;
            CefRefPtr<CefV8Value> biz_id;
            CefRefPtr<CefV8Value> hookSet = value->GetValue("hookSet");
            if (hookSet)
            {
                bili_filename = hookSet->GetValue("bili_filename");
                upos_uri = hookSet->GetValue("upos_uri");
                biz_id = hookSet->GetValue("biz_id");
            }

            CefRefPtr<CefV8Value> profile = value->GetValue("profile");

            CefRefPtr<CefValue> root_node = CefValue::Create();

            CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
            dict->SetString("id", id->GetStringValue());
            dict->SetInt("blobSize", blobSize->GetIntValue());
            dict->SetString("name", name->GetStringValue());
            dict->SetString("ext", ext->GetStringValue());
            dict->SetInt("chunkSize", chunkSize->GetIntValue());
            dict->SetInt("percentage", percentage->GetIntValue());
            dict->SetInt("uploadBytesPerSecond", uploadBytesPerSecond->GetIntValue());
            dict->SetInt("blobUploaded", blobUploaded->GetIntValue());

            CefRefPtr<CefDictionaryValue> hook_dict = CefDictionaryValue::Create();
            hook_dict->SetString("bili_filename", bili_filename->GetStringValue());
            hook_dict->SetString("upos_uri", upos_uri->GetStringValue());
            hook_dict->SetString("biz_id", biz_id->GetStringValue());

            dict->SetDictionary("hookSet", hook_dict);

            root_node->SetDictionary(dict);

            msg_str = CefWriteJSON(root_node, JSON_WRITER_DEFAULT);
        }
        else if (value->IsString())
        {
            msg_str = value->GetStringValue();
        }
        return msg_str;
    }

    CefString GetBiliyunErrorJsonString(CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Value> value)
    {
        CefString msg_str;
        if (value->IsObject())
        {
            CefRefPtr<CefV8Value> type = value->GetValue("type");
            CefRefPtr<CefV8Value> detail = value->GetValue("detail");

            CefRefPtr<CefValue> root_node = CefValue::Create();
            CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
            dict->SetString("type", type->GetStringValue());
            dict->SetString("detail", detail->GetStringValue());
            root_node->SetDictionary(dict);

            msg_str = CefWriteJSON(root_node, JSON_WRITER_DEFAULT);
        }
        else if (value->IsString())
        {
            msg_str = value->GetStringValue();
        }
        return msg_str;
    }

    void OnUploadErrorHandle(CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> msg)
    {
        g_current_task_id = -1;
        SendBrowserProcessMessage(browser, PID_BROWSER, msg);
    }
}

LivehimeBiliyunRenderDelegate::LivehimeBiliyunRenderDelegate()
{
}

LivehimeBiliyunRenderDelegate::~LivehimeBiliyunRenderDelegate()
{
}

void LivehimeBiliyunRenderDelegate::OnContextCreated(CefRefPtr<LivehimeCefAppRender> app,
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context)
{
    bool result = true;
    CefRefPtr<CefV8Value> globalObj = context->GetGlobal();

    CefRefPtr<CefV8Value> inject = CefV8Value::CreateObject(nullptr, nullptr);
    {
        for (auto& iter : kSupportJsFunctionNames)
        {
            CefRefPtr<CefV8Value> js_func = CefV8Value::CreateFunction(iter.second, this);
            result &= inject->SetValue(iter.second, js_func, V8_PROPERTY_ATTRIBUTE_NONE);
        }

        CefRefPtr<CefV8Value> user_data = CefV8Value::CreateInt(bilibase::enum_cast(cef_proxy::client_handler_type::biliyun));
        result &= inject->SetValue(js_values::kLivehimeUserData, user_data, V8_PROPERTY_ATTRIBUTE_NONE);
    }
    result &= globalObj->SetValue(kModuleObject, inject, V8_PROPERTY_ATTRIBUTE_NONE);
    DCHECK(result);
}

bool LivehimeBiliyunRenderDelegate::OnProcessMessageReceived(CefRefPtr<LivehimeCefAppRender> app,
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{
    CefString msg_name = message->GetName();
    if (msg_name == ipc_messages::kBiliyunUploaderLocalFile)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        CefString file_path = args->GetString(0);
        CefString file_name = args->GetString(1);
        CefString file_type = args->GetString(2);
        int task_id = args->GetInt(3);

        // 当前只允许同时一个上传任务在执行，这个事由上层来确保
        if (-1 != g_current_task_id)
        {
            LOG(WARNING) << "[biliyun] post upload task when some other task is running, running_task="
                << g_current_task_id;
            NOTREACHED();
        }

        CefRefPtr<CefV8Context> context = browser->GetMainFrame()->GetV8Context();

        context->Enter();

        CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
        CefRefPtr<CefV8Value> uploadFileToBiliyun = globalObj->GetValue("uploadFileToBiliyun");
        if (uploadFileToBiliyun && uploadFileToBiliyun->IsFunction())
        {
            if (!file_path.empty())
            {
                std::string buf;
                std::fstream fs;
                fs.open(file_path.c_str(), std::ios::binary | std::ios::in);
                if (fs.is_open())
                {
                    fs.seekg(0, std::ios_base::end);
                    unsigned long len = (unsigned long)fs.tellg();
                    fs.seekg(0, std::ios_base::beg);
                    buf.resize(len);
                    fs.read(&buf[0], len);
                    fs.close();
                }
                else
                {
                    // 文件读失败，要通知
                    LOG(WARNING) << "[biliyun] read local file failed '" << file_path << "'";

                    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBiliyunUploadError);
                    CefRefPtr<CefListValue> args = msg->GetArgumentList();
                    args->SetString(0, R"({"detail":"local file read failed.", "type":"LOCAL_ERROR_READ_FILE_FAILED"})");
                    args->SetInt(1, task_id);
                    SendBrowserProcessMessage(browser, PID_BROWSER, msg);
                }

                if (!buf.empty())
                {
                    g_current_task_id = task_id;

                    CefRefPtr<BiliyunArrayBufferReleaseCallback> release_cb(new BiliyunArrayBufferReleaseCallback(std::move(buf)));
                    CefRefPtr<CefV8Value> bytes_array = CefV8Value::CreateArrayBuffer(
                        release_cb->data(), release_cb->length(), release_cb);
                    CefRefPtr<CefV8Value> name = CefV8Value::CreateString(file_name);
                    CefRefPtr<CefV8Value> type = CefV8Value::CreateString(file_type);
                    // 根据blob来创建JS File对象
                    CefRefPtr<CefV8Value> ret_obj = uploadFileToBiliyun->ExecuteFunction(globalObj, { bytes_array, name, type });
                    if (ret_obj)
                    {
                        if (ret_obj->IsString())
                        {
                            std::string ret = ret_obj->GetStringValue().ToString();
                            if (std::string::npos != ret.find("failed", 0))
                            {
                                LOG(WARNING) << "[biliyun] upload file failed -> " << ret_obj->GetStringValue().ToString();

                                CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBiliyunUploadError);
                                CefRefPtr<CefListValue> args = msg->GetArgumentList();
                                args->SetString(0, R"({"detail":"uploadFileToBiliyun failed.", "type":"UPLOAD_FILE_TO_BILIYUN_FAILED"})");
                                args->SetInt(1, task_id);
                                OnUploadErrorHandle(browser, msg);
                            }
                            else
                            {
                                LOG(INFO) << "[biliyun] upload file success -> " << ret_obj->GetStringValue().ToString();
                            }
                        }
                    }
                    else
                    {
                        LOG(WARNING) << "[biliyun] upload file failed.";

                        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBiliyunUploadError);
                        CefRefPtr<CefListValue> args = msg->GetArgumentList();
                        args->SetString(0, R"({"detail":"uploadFileToBiliyun failed.", "type":"UPLOAD_FILE_TO_BILIYUN_FAILED"})");
                        args->SetInt(1, task_id);
                        OnUploadErrorHandle(browser, msg);
                    }
                }
            }
        }
        else
        {
            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBiliyunUploadError);
            CefRefPtr<CefListValue> args = msg->GetArgumentList();
            args->SetString(0, R"({"detail":"maybe biliyun page error, like 'Uncaught SyntaxError: ....' etc.", "type":"MAYBE_BILIYUN_PAGE_ERROR"})");
            args->SetInt(1, task_id);
            OnUploadErrorHandle(browser, msg);
        }

        context->Exit();

        return true;
    }
    else if (msg_name == ipc_messages::kBiliyunUploaderDelete)
    {
        CefRefPtr<CefV8Context> context = browser->GetMainFrame()->GetV8Context();

        context->Enter();

        CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
        CefRefPtr<CefV8Value> deleteTaskFromBiliyun = globalObj->GetValue("deleteTaskFromBiliyun");
        if (deleteTaskFromBiliyun && deleteTaskFromBiliyun->IsFunction())
        {
            CefRefPtr<CefListValue> args = message->GetArgumentList();
            int task_id = args->GetInt(0);
            CefString sdk_box_id = args->GetString(1);

            CefRefPtr<CefV8Value> box_id = CefV8Value::CreateString(sdk_box_id);
            // 根据blob来创建JS File对象
            CefRefPtr<CefV8Value> ret_obj = deleteTaskFromBiliyun->ExecuteFunction(globalObj, { box_id });
            if (ret_obj)
            {
                if (ret_obj->IsString())
                {
                    LOG(INFO) << "[biliyun] delete task result -> " << ret_obj->GetStringValue().ToString();
                }
            }
            else
            {
                LOG(WARNING) << "[biliyun] delete task failed.";
            }
        }

        context->Exit();

        return true;
    }
    return false;
}

bool LivehimeBiliyunRenderDelegate::Execute(const CefString& name,
    CefRefPtr<CefV8Value> object,
    const CefV8ValueList& arguments,
    CefRefPtr<CefV8Value>& retval,
    CefString& exception)
{
    if (js_bridge::IsInvokeOnModuleObj(object, cef_proxy::client_handler_type::biliyun))
    {
        bool handled = false;

        auto iter = std::find_if(kSupportJsFunctionNames.begin(), kSupportJsFunctionNames.end(), 
            [&](const std::map<BiliyunEventType, std::string>::value_type& method_pair)->bool {
                return (name.ToString() == method_pair.second);
            });
        if (iter != kSupportJsFunctionNames.end())
        {
            /* http://bd-mng.bilibili.co/bup-doc/
            *  ----------------------- 数据返回示例 ------------------------
            *  BeforePreUpload
            *  { "blobSize":651606,"blobUploaded" : 0,"chunkSize" : 0,"ext" : "zip","hookSet" : {"bili_filename":"","biz_id" : "","upos_uri" : ""},"id" : "BUploader_1_a4859_1623922851","name" : "biliyun-test.zip","percentage" : 0,"uploadBytesPerSecond" : 0 }
            *  
            *  BeforeUpload
            *  { "blobSize":651606,"blobUploaded" : 0,"chunkSize" : 10485760,"ext" : "zip","hookSet" : {"bili_filename":"m210617a226clsbmqsyu3z3rngl6sg1f","biz_id" : "","upos_uri" : "upos://tmp/m210617a226clsbmqsyu3z3rngl6sg1f.zip"},"id" : "BUploader_1_a4859_1623922851","name" : "biliyun-test.zip","percentage" : 0,"uploadBytesPerSecond" : 0 }
            *  
            *  UploadProgress
            *  { "blobSize":651606,"blobUploaded" : 651606,"chunkSize" : 10485760,"ext" : "zip","hookSet" : {"bili_filename":"m210617a226clsbmqsyu3z3rngl6sg1f","biz_id" : "","upos_uri" : "upos://tmp/m210617a226clsbmqsyu3z3rngl6sg1f.zip"},"id" : "BUploader_1_a4859_1623922851","name" : "biliyun-test.zip","percentage" : 100,"uploadBytesPerSecond" : 4315271 }
            *  
            *  BeforeFileUploaded
            *  { "blobSize":651606,"blobUploaded" : 651606,"chunkSize" : 10485760,"ext" : "zip","hookSet" : {"bili_filename":"m210617a226clsbmqsyu3z3rngl6sg1f","biz_id" : "","upos_uri" : "upos://tmp/m210617a226clsbmqsyu3z3rngl6sg1f.zip"},"id" : "BUploader_1_a4859_1623922851","name" : "biliyun-test.zip","percentage" : 100,"uploadBytesPerSecond" : 0 }
            *  
            *  FileUploaded
            *  { "blobSize":651606,"blobUploaded" : 651606,"chunkSize" : 10485760,"ext" : "zip","hookSet" : {"bili_filename":"m210617a226clsbmqsyu3z3rngl6sg1f","biz_id" : "","upos_uri" : "upos://tmp/m210617a226clsbmqsyu3z3rngl6sg1f.zip"},"id" : "BUploader_1_a4859_1623922851","name" : "biliyun-test.zip","percentage" : 100,"uploadBytesPerSecond" : 0 }
            *  
            *  UploadError
            *  { "detail":"biliyun-test 上传格式不被支持","type" : "EXT_NOT_ACCEPTED_ERROR" }
            *  { "detail":"upos 上传文件前同步出现错误", "type" : "BEFORE_SEND_ERROR" }
            */
            CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();

            CefRefPtr<CefBrowser> browser = context->GetBrowser();

            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("Biliyun" + iter->second);
            CefRefPtr<CefListValue> args = msg->GetArgumentList();
            switch (iter->first)
            {
            case BeforePreUpload:
            case BeforeUpload:
            case UploadProgress:
            case BeforeFileUploaded:
            case FileUploaded:
                {
                    CefString msg_str = GetBiliyunBoxJsonString(context, arguments[0]);
                    if (msg_str.empty())
                    {
                        LOG(WARNING) << "[biliyun] " << kModuleObject << "." << name.ToString() << " with empty data!";
                        NOTREACHED();
                    }

                    args->SetString(0, msg_str);
                    args->SetInt(1, g_current_task_id);

                    if (iter->first == FileUploaded)
                    {
                        g_current_task_id = -1;
                    }
                    SendBrowserProcessMessage(browser, PID_BROWSER, msg);
                }
                break;
            case UploadError:
                {
                    CefString msg_str = GetBiliyunErrorJsonString(context, arguments[0]);
                    if (msg_str.empty())
                    {
                        LOG(WARNING) << "[biliyun] " << kModuleObject << "." << name.ToString() << " with empty data!";
                        NOTREACHED();
                    }

                    args->SetString(0, msg_str);
                    args->SetInt(1, g_current_task_id);
                    OnUploadErrorHandle(browser, msg);
                }
                break;
            default:
                break;
            }
        }
        else
        {
            LOG(WARNING) << "[biliyun] " << kModuleObject << " not support js func: " << name.ToString();
        }

        return true;
    }

    return false;
}
