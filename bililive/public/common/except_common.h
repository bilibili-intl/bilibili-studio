#pragma once
#include <mutex>
#include <type_traits>
#include <string>
#include <vector> 
#include <ShlObj_core.h>

namespace bililive {

    std::mutex g_except_report_files_sync;

    void WriteExceptFile(const std::vector<std::string>& func_name) {
        std::lock_guard<std::mutex> guard(g_except_report_files_sync);

        if (func_name.empty()) {
            return;
        }

        const size_t kBufSize = MAX_PATH + 1;
        char path_buf[kBufSize]{ 0 };
        if (FAILED(SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr,
            SHGFP_TYPE_CURRENT, path_buf))) {
            return;
        }

        if (strlen(path_buf) > 0) {
            std::string file_path(path_buf);
            file_path.append("\\bililive\\User Data\\Crash Reports\\except_file");

            FILE* fp = nullptr;
            fopen_s(&fp, file_path.c_str(), "a+");

            if (fp) {
                for (auto& iter : func_name) {
                    if (fgetc(fp) == EOF) {
                        fwrite(iter.data(), 1, iter.length(), fp);
                    }
                    else {
                        fseek(fp, 0, SEEK_END);
                        std::string content = "\n||";
                        content.append(iter.data());
                        fwrite(content.c_str(), 1, content.length(), fp);
                    }
                }

                fclose(fp);
            } 
        }
    }


    template<typename T,
        typename = typename std::enable_if<std::is_same<int, T>::value || std::is_same<long, T>::value || std::is_same<std::string, T>::value>::type>
        std::string RecordSingleExcept(const T& arg)
    {
        return RecordSingleExcept(arg);
    }
    
    template<>
    std::string RecordSingleExcept(const std::string& arg)
    {
        return arg;
    }

    template<>
    std::string RecordSingleExcept(const int& arg)
    {
        return std::to_string(arg);
    }

    template<>
    std::string RecordSingleExcept(const long& arg)
    {
        return std::to_string(arg);
    }


    void RecordExcepts(std::vector<std::string>& result) {
    }

    template<typename T, typename... Types>
    void RecordExcepts(std::vector<std::string>& result, const T& firstArg, const Types&... args)
    {
        result.push_back(RecordSingleExcept(firstArg));
        RecordExcepts(result, args...);
    }

}



//typename std::enable_if < std::is_same<int, T>::value || std::is_same<long, T>::value, void >::type