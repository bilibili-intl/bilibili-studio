#include "obs/obs_proxy/core_proxy/obs_logging_hook.h"

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"

#include "obs/obs-studio/libobs/util/base.h"

namespace {

log_handler_t obs_log_handler = nullptr;
void* obs_log_param = nullptr;

void* obs_proxy_log_param = nullptr;

void OBSDefaultLogHandler(int level, const char* msg_fmt, va_list args, void* param)
{
    if (!obs_log_handler)
    {
        NOTREACHED();
        LOG(ERROR) << "obs log handler is empty!";
        return;
    }

    obs_log_handler(level, msg_fmt, args, param);
}

// FFMpeg logging utils erroneously use format specifiers that would make the application crash.
std::string NormalizeFormatSpecifiers(const std::string& fmt)
{
    std::string safe_fmt(fmt);

    ReplaceSubstringsAfterOffset(&safe_fmt, 0, "%td", "%Id");
    ReplaceSubstringsAfterOffset(&safe_fmt, 0, "%zu", "%Iu");

    return safe_fmt;
}

void OBSProxyLogHandler(int level, const char* msg_fmt, va_list args, void*)
{
    auto safe_fmt = NormalizeFormatSpecifiers(msg_fmt);
    auto log_message = base::StringPrintV(safe_fmt.c_str(), args);

    switch (level)
    {
        case LOG_DEBUG:
            DLOG(INFO) << log_message;
            break;

        case LOG_INFO:
            LOG(INFO) << log_message;
            break;

        case LOG_WARNING:
            LOG(WARNING) << log_message;
            break;

        case LOG_ERROR:
            LOG(ERROR) << log_message;
            break;

        default:
            NOTREACHED();
            break;
    }

    OBSDefaultLogHandler(level, msg_fmt, args, obs_log_param);
}

}   // namespace

namespace obs_proxy {

void HookOBSLogging()
{
    base_get_log_handler(&obs_log_handler, &obs_log_param);
    base_set_log_handler(OBSProxyLogHandler, obs_proxy_log_param);
}

}   // namespace obs_proxy