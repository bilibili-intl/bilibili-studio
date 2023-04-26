#ifndef BILILIVE_BILILIVE_UTILS_CONVERT_UTIL_H
#define BILILIVE_BILILIVE_UTILS_CONVERT_UTIL_H

#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"

#include "SkColor.h"


namespace bililive
{
    // ������˽ӿڷ��ص�ʱ���ת��ʱ���ַ���xxxx-xx-xx xx:xx:xx
    base::string16 TimeStampToStandardString(int64_t time_stamp);
    // ��SYSTEMTIMEתʱ���time_t
    time_t SystemTimeToTimeStamp(LPSYSTEMTIME pSystime);
    SYSTEMTIME TimeStampToSystemTime(time_t tt);
    // ��byte����תΪ���ʵ�����
    base::string16 ByteSpeedToSuitablySpeed(int64_t speed_int_byte);

    /**
     * ��ֵ����һ��ʱ����λ��Ϊ���������뱣��һλС����
     * ��С������Ϊ0������ʾС����
     */
    string16 FriendlyGiftCost(int64_t total_cost);

    /**
    * ��ֵ����һ��ʱ����λ��Ϊ���������뱣��3λС����
    */
    string16 GiftCost(int64_t total_cost);

    /**
     * ��ֵ����һ��ʱ����λ��Ϊ���������뱣��һλС����
     * ��ֵ����һ��ʱ����λ��Ϊ�ڣ��������뱣��һλС����
     * ��С������Ϊ0������ʾС����
     */
    string16 SpreadCount(int64_t total_count);

    // ��������ʱ���ת��Ϊ hh::mm ����ʽ
    string16 TimeStampToHHMM(int64_t time_stamp);

    //��������ʱ���ת��Ϊ ��-�� Сʱ:���� ����ʽ
    string16 TimeStampToLLDDHHMM(int64_t time_stamp);

    // �� #AARRGGBB �� #RRGGBB ת��Ϊ SkColor
    bool ParseColorString(const std::string& str, SkColor* out);

    // doubleתΪ����һλС�����ַ���
    std::string DoubleToString(double value);

    // ͨ��ϵͳ����Ż�ȡ����ϵͳ������Ϣ
    base::string16 GetSystemErrorMsg(DWORD code);

    //��ȡ�ַ������ȣ�һ��������һ��������������һ��
    int CountWords(const string16& new_contents);

    //��׼ʱ���ʽת��Ϊʱ���;���磺2016:08:02 12:12:30
    int64_t standard_to_stamp(const std::string& standard_time);
}

#endif