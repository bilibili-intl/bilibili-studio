
#include "bililive/bililive/log_report/app_integrity_checking.h"

#include "base/ext/callable_callback.h"
#include "base/file_util.h"
#include "base/files/file_enumerator.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/threading/thread_restrictions.h"

#include "bililive/common/bililive_context.h"
#include "bililive/common/bililive_logging.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/bililive_paths.h"
#include "bililive/public/secret/bililive_secret.h"

namespace
{
    const std::vector<std::string> check_files
    {
        // obs
        R"(plugins\obs-outputs.dll)",   // 无法推流

        // cef-proxy
        // cef-proxy相关dll、exe缺失可能导致无法登录（需要验证码时）
        R"(plugins\cef_proxy.dll)",
        R"(plugins\bililive_browser.exe)",

        // CEF
        // dll缺失CEF整体无法加载
        R"(plugins\libcef.dll)",
        R"(plugins\chrome_elf.dll)",
        // .dat缺失CefInitialize会崩
        R"(plugins\icudtl.dat)",
        // .bin缺失不会蹦，CefInitialize返回成功，但CEF不会渲染页面
        //R"(plugins\natives_blob.bin)",    // 5195版本已移除，没有影响
        R"(plugins\snapshot_blob.bin)",
        R"(plugins\v8_context_snapshot.bin)",
        // .pak缺失CefInitialize会崩
        //R"(plugins\cef.pak)",   // CefInitialize会崩    // 5195版本已移除，没有影响
        R"(plugins\resources.pak)",   // CefInitialize会崩
        //R"(plugins\cef_100_percent.pak)",   // 没有明显影响
        //R"(plugins\cef_200_percent.pak)",   // 没有明显影响
        //R"(plugins\cef_extensions.pak)",   // 没有明显影响
        //R"(plugins\devtools_resources.pak)",// 调试工具无法使用  // 5195版本已移除，没有影响
        //R"(plugins\locales\en-US.pak)",   // 没有明显影响
        //R"(plugins\locales\zh-CN.pak)",   // 没有明显影响
        //缺失无法推流
        R"(plugins\bvc-srt-stream.dll)",

    };

}   // namespace

namespace livehime
{
    std::vector<std::string> FilesIntegrityChecking()
    {
        static bool checked = false;
        static std::vector<std::string> miss;

        if (!checked)
        {
            auto main_dir = BililiveContext::Current()->GetMainDirectory();

            for (auto& iter : check_files)
            {
                auto path = main_dir.AppendASCII(iter);
                if (!base::PathExists(path))
                {
                    miss.push_back(iter);
                }
            }

            checked = true;
        }
        return miss;
    }
}