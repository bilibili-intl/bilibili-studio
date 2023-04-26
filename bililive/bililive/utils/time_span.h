#ifndef BILILIVE_BILILIVE_UTILS_TIME_SPAN_H_
#define BILILIVE_BILILIVE_UTILS_TIME_SPAN_H_

#include "base/timer/timer.h"


namespace bililive {
    SYSTEMTIME TimetToSystemTime(time_t t);

    // h:m:s
    std::wstring TimeStampToString(int64_t time_stamp);

    // 年月日-时分秒毫秒
    std::wstring TimeFormatFriendlyDate(const base::Time& time);

    // 获取当前日期，in format：YYYYMMDD，十进制，如20200229，2020/02/29
    int GetCurrentDate();

    int GetCurrentDate(int64_t time_stamp);

    class TimeSpan {
    public:
        TimeSpan(const base::TimeDelta &delta)
            : m_timeSpan(delta.InMilliseconds()) {}
        ~TimeSpan() {}

        inline LONGLONG GetDays() const throw()
        {
            return (m_timeSpan / (24 * 3600 * 1000));
        }

        inline LONGLONG GetTotalHours() const throw() {
            return (m_timeSpan / (3600 * 1000));
        }

        inline LONG GetHours() const throw() {
            return (LONG(GetTotalHours() - (GetDays() * 24)));
        }

        inline LONGLONG GetTotalMinutes() const throw() {
            return (m_timeSpan / (60 * 1000));
        }

        inline LONG GetMinutes() const throw() {
            return (LONG(GetTotalMinutes() - (GetTotalHours() * 60)));
        }

        inline LONGLONG GetTotalSeconds() const throw() {
            return (m_timeSpan / 1000);
        }

        inline LONG GetSeconds() const throw() {
            return (LONG(GetTotalSeconds() - (GetTotalMinutes() * 60)));
        }

        inline LONGLONG GetTotalMilliseconds() const throw() {
            return (m_timeSpan);
        }

        inline LONG GetMilliseconds() const throw() {
            return (LONG(GetTotalMilliseconds() % 1000));
        }

        inline __time64_t GetTimeSpan() const throw() {
            return (m_timeSpan);
        }


    private:
        __time64_t m_timeSpan;
    };
}

#endif  // BILILIVE_BILILIVE_UTILS_TIME_SPAN_H_