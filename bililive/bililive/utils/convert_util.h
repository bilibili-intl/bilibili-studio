#ifndef BILILIVE_BILILIVE_UTILS_CONVERT_UTIL_H
#define BILILIVE_BILILIVE_UTILS_CONVERT_UTIL_H

#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"

#include "SkColor.h"


namespace bililive
{
    // 将服务端接口返回的时间戳转成时间字符串xxxx-xx-xx xx:xx:xx
    base::string16 TimeStampToStandardString(int64_t time_stamp);
    // 将SYSTEMTIME转时间戳time_t
    time_t SystemTimeToTimeStamp(LPSYSTEMTIME pSystime);
    SYSTEMTIME TimeStampToSystemTime(time_t tt);
    // 将byte速率转为合适的速率
    base::string16 ByteSpeedToSuitablySpeed(int64_t speed_int_byte);

    /**
     * 数值大于一万时，单位改为万，四舍五入保留一位小数。
     * 若小数部分为0，则不显示小数。
     */
    string16 FriendlyGiftCost(int64_t total_cost);

    /**
    * 数值大于一万时，单位改为万，四舍五入保留3位小数。
    */
    string16 GiftCost(int64_t total_cost);

    /**
     * 数值大于一万时，单位改为万，四舍五入保留一位小数。
     * 数值大于一亿时，单位改为亿，四舍五入保留一位小数。
     * 若小数部分为0，则不显示小数。
     */
    string16 SpreadCount(int64_t total_count);

    // 将服务器时间戳转换为 hh::mm 的形式
    string16 TimeStampToHHMM(int64_t time_stamp);

    //将服务器时间戳转换为 月-日 小时:分钟 的形式
    string16 TimeStampToLLDDHHMM(int64_t time_stamp);

    // 将 #AARRGGBB 或 #RRGGBB 转换为 SkColor
    bool ParseColorString(const std::string& str, SkColor* out);

    // double转为保留一位小数的字符串
    std::string DoubleToString(double value);

    // 通过系统错误号获取具体系统错误信息
    base::string16 GetSystemErrorMsg(DWORD code);

    //获取字符串长度，一个汉字算一个，两个数字算一个
    int CountWords(const string16& new_contents);

    //标准时间格式转化为时间戳;例如：2016:08:02 12:12:30
    int64_t standard_to_stamp(const std::string& standard_time);
}

#endif