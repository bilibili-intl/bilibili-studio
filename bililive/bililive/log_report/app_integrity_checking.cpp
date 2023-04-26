
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
        R"(plugins\obs-outputs.dll)",   // �޷�����

        // cef-proxy
        // cef-proxy���dll��exeȱʧ���ܵ����޷���¼����Ҫ��֤��ʱ��
        R"(plugins\cef_proxy.dll)",
        R"(plugins\bililive_browser.exe)",

        // CEF
        // dllȱʧCEF�����޷�����
        R"(plugins\libcef.dll)",
        R"(plugins\chrome_elf.dll)",
        // .datȱʧCefInitialize���
        R"(plugins\icudtl.dat)",
        // .binȱʧ����ģ�CefInitialize���سɹ�����CEF������Ⱦҳ��
        //R"(plugins\natives_blob.bin)",    // 5195�汾���Ƴ���û��Ӱ��
        R"(plugins\snapshot_blob.bin)",
        R"(plugins\v8_context_snapshot.bin)",
        // .pakȱʧCefInitialize���
        //R"(plugins\cef.pak)",   // CefInitialize���    // 5195�汾���Ƴ���û��Ӱ��
        R"(plugins\resources.pak)",   // CefInitialize���
        //R"(plugins\cef_100_percent.pak)",   // û������Ӱ��
        //R"(plugins\cef_200_percent.pak)",   // û������Ӱ��
        //R"(plugins\cef_extensions.pak)",   // û������Ӱ��
        //R"(plugins\devtools_resources.pak)",// ���Թ����޷�ʹ��  // 5195�汾���Ƴ���û��Ӱ��
        //R"(plugins\locales\en-US.pak)",   // û������Ӱ��
        //R"(plugins\locales\zh-CN.pak)",   // û������Ӱ��
        //ȱʧ�޷�����
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