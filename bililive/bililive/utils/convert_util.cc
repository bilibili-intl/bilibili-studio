#include "convert_util.h"

#include <iomanip>
#include <regex>

#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/base/resource/resource_bundle.h"
#include "grit/theme_resources.h"
#include "grit/generated_resources.h"


namespace bililive
{

    time_t SystemTimeToTimeStamp(LPSYSTEMTIME pSystime)
    {
        tm temptm =
        {
            pSystime->wSecond,
            pSystime->wMinute,
            pSystime->wHour,
            pSystime->wDay,
            pSystime->wMonth - 1,
            pSystime->wYear - 1900,
            pSystime->wDayOfWeek,
            0,
            0
        };
        // 将tm按本地时区转换为UTC时间戳
        time_t mt = mktime(&temptm);

        tzset();
        mt += _timezone + 28800;

        return mt;
    }

    SYSTEMTIME TimeStampToSystemTime(time_t tt)
    {
        //tm ltm = *localtime(&tt);
        // 将UTC时间加8小时转为北京时间
        tt += +28800;
        // time_t转tm
        tm utc_8 = *gmtime(&tt);
        SYSTEMTIME st =
        {
            utc_8.tm_year + 1900,
            utc_8.tm_mon + 1,
            utc_8.tm_wday,
            utc_8.tm_mday,
            utc_8.tm_hour,
            utc_8.tm_min,
            utc_8.tm_sec,
            0
        };

        return st;
    }

    base::string16 TimeStampToStandardString(int64_t time_stamp)
    {
        SYSTEMTIME st = TimeStampToSystemTime(time_stamp);
        return base::StringPrintf(L"%d-%02d-%02d %02d:%02d:%02d",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        /*time_t tick = (time_t)db;
        struct tm *ti = NULL;
        char s[128] = { 0 };

        ti = localtime(&tick);
        strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", ti);

        return base::ASCIIToWide(s);*/
    }

    base::string16 ByteSpeedToSuitablySpeed(int64_t speed_int_byte)
    {
        static const int64_t kb = 1024;
        static const int64_t mb = 1024 * kb;
        static const int64_t gb = 1024 * mb;

        if (speed_int_byte > gb)
        {
            return base::StringPrintf(L"%0.1f GB/S", speed_int_byte * 1.0f / gb);
        }
        else if (speed_int_byte > mb)
        {
            return base::StringPrintf(L"%0.1f MB/S", speed_int_byte * 1.0f / mb);
        }
        else if (speed_int_byte > kb)
        {
            return base::StringPrintf(L"%0.1f KB/S", speed_int_byte * 1.0f / kb);
        }
        else
        {
            return L"小于1KB/S";
        }
        return L"";
    }

    string16 FriendlyGiftCost(int64_t total_cost) {
        if (total_cost >= 10000) {
            int64_t wdec = total_cost / 10000;
            int thos = total_cost % 10000;
            int thdec = thos / 1000;
            int huns = thos % 1000;
            int hudec = huns / 100;

            string16 dec;
            if (hudec < 5) {
                if (thdec != 0) {
                    dec = string16(L".").append(std::to_wstring(thdec));
                }
            } else {
                ++thdec;
                if (thdec == 10) {
                    ++wdec;
                } else {
                    dec = string16(L".").append(std::to_wstring(thdec));
                }
            }

            return std::to_wstring(wdec) + dec + std::wstring(L"万");
        }

        return std::to_wstring(total_cost);
    }

    string16 GiftCost(int64_t total_cost)
    {
        std::wstringstream string_stream;
        if (total_cost < 10000)
        {
            string_stream << total_cost;
            return string_stream.str();
        }
        else
        {
            int64_t remainder = total_cost % 10000;
            if (remainder > 0)
            {
                double cost = static_cast<double>(total_cost) / 10000;

                string_stream << std::setiosflags(std::ios::fixed) << std::setprecision(3) << cost;
                string16 cost_str = string_stream.str();

                //去除多余的0
                while (cost_str.back() == L'0')
                {
                    cost_str.pop_back();
                }

                return cost_str + L"万";
            }
            else
            {
                int64_t cost = total_cost / 10000;
                string_stream << cost << L"万";
                return string_stream.str();
            }
        }

        return string_stream.str();
    }

    string16 SpreadCount(int64_t total_count) {
        if (total_count >= 100000000) {
            int64_t wdec = total_count / 100000000;
            int thos = total_count % 100000000;
            int thdec = thos / 10000000;

            string16 dec;
            if (thdec != 0) {
                dec = string16(L".").append(std::to_wstring(thdec));
            }

            return std::to_wstring(wdec) + dec + std::wstring(L"亿");
        }
        else if (total_count >= 10000) {
            int64_t wdec = total_count / 10000;
            int thos = total_count % 10000;
            int thdec = thos / 1000;

            string16 dec;
            if (thdec != 0) {
                dec = string16(L".").append(std::to_wstring(thdec));
            }

            return std::to_wstring(wdec) + dec + std::wstring(L"万");
        }

        return std::to_wstring(total_count);
    }

    string16 TimeStampToHHMM(int64_t time_stamp) {
        if (time_stamp <= 0) {
            return L"00:00";
        }

        base::Time::Exploded exploded;
        base::Time::FromTimeT(time_stamp).LocalExplode(&exploded);

        auto time = base::StringPrintf(L"%02d:%02d",
            exploded.hour,
            exploded.minute);

        return time;
    }

    string16 TimeStampToLLDDHHMM(int64_t time_stamp)
    {
        if (time_stamp <= 0) {
            return L"00:00";
        }

        base::Time::Exploded exploded;
        base::Time::FromTimeT(time_stamp).LocalExplode(&exploded);

        auto time = base::StringPrintf(L"%02d-%02d %02d:%02d",
            exploded.month,
            exploded.day_of_month,
            exploded.hour,
            exploded.minute);

        return time;
    }

    bool ParseColorString(const std::string& str, SkColor* out) {
        std::smatch match;
        std::regex regex("^#([0-9a-fA-F]{6})$|^#([0-9a-fA-F]{8})$");
        if (std::regex_match(str, match, regex)) {
            std::string result = match.str(1);
            if (result.empty()) {
                result = match.str(2);
            } else {
                result.insert(result.begin(), { 'F', 'F' });
            }

            uint64_t color;
            if (base::HexStringToUInt64(result, &color)) {
                // 请保证颜色分量的顺序是 ARGB
                *out = static_cast<SkColor>(uint32_t(color));
                return true;
            }
        }
        return false;
    }


    std::string DoubleToString(double value) {
        std::string result;
        result = base::StringPrintf("%0.1lf", value);
        auto pos = result.find(".");
        if (pos != std::string::npos) {
            std::string temp = result.substr(pos + 1, 1);
            if (temp == "0") {
                result = result.substr(0, pos);
            }
            else {
                result = result.substr(0, pos + 2);
            }
        }

        return result;
    }

    base::string16 GetSystemErrorMsg(DWORD code)
    {
        LPWSTR error_string = NULL;
        int size = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
            0,  // Use the internal message table.
            code,
            0,  // Use default language.
            (LPWSTR)&error_string,
            0,  // Buffer size.
            0);  // Arguments (unused).

        base::string16 msg(error_string);

        ::LocalFree(error_string);

        return msg;
    }

    int CountWords(const string16& new_contents)
    {
        std::string content = base::UTF16ToUTF8(new_contents);

        int utf16_len = new_contents.length();
        int utf8_len = content.length();

        int word_cha = 0;
        int word_en = 0;

        int tmp = utf8_len - utf16_len;
        word_cha = (int)(tmp / 2);
        word_en = utf16_len - word_cha;

        return (word_cha + std::ceil((double)word_en / 2));
    }

    int64_t standard_to_stamp(const std::string& str_standard_time)
    {
        if (str_standard_time.size() < 19)
        {
            return -1;
        }
        const char* standard_time = str_standard_time.c_str();

        struct tm stm;
        int iY, iM, iD, iH, iMin, iS;

        memset(&stm, 0, sizeof(stm));
        iY = atoi(standard_time);
        iM = atoi(standard_time + 5);
        iD = atoi(standard_time + 8);
        iH = atoi(standard_time + 11);
        iMin = atoi(standard_time + 14);
        iS = atoi(standard_time + 17);

        stm.tm_year = iY - 1900;
        stm.tm_mon = iM - 1;
        stm.tm_mday = iD;
        stm.tm_hour = iH;
        stm.tm_min = iMin;
        stm.tm_sec = iS;

        return (int64_t)mktime(&stm);
    }
}