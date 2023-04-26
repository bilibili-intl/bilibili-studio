#include "time_span.h"

#include "base/strings/stringprintf.h"
#include "base/time/time.h"


namespace
{
    int GetDateFromTime(base::Time time)
    {
        base::Time::Exploded exploded;
        time.LocalExplode(&exploded);

        return exploded.year * 10000 + exploded.month * 100 + exploded.day_of_month;
    }
}

namespace bililive {
    SYSTEMTIME TimetToSystemTime(time_t t) {
        tm temptm = *localtime(&t);
        SYSTEMTIME st = { 1900 + temptm.tm_year,
            1 + temptm.tm_mon,
            temptm.tm_wday,
            temptm.tm_mday,
            temptm.tm_hour,
            temptm.tm_min,
            temptm.tm_sec,
            0 };

        return st;
    }

    std::wstring TimeStampToString(int64_t time_stamp)
    {
        if (time_stamp <= 0) {
            return L"00:00:00";
        }

        base::Time::Exploded exploded;
        base::Time::FromTimeT(time_stamp).LocalExplode(&exploded);

        auto time = base::StringPrintf(L"%02d:%02d:%02d",
            exploded.hour,
            exploded.minute,
            exploded.second);

        return time;
    }

    std::wstring TimeFormatFriendlyDate(const base::Time& time)
    {
        base::Time::Exploded exploded;
        time.LocalExplode(&exploded);
        
        auto str = base::StringPrintf(L"%d%02d%02d-%02d%02d%02d%03d",
            exploded.year,
            exploded.month,
            exploded.day_of_month,
            exploded.hour,
            exploded.minute,
            exploded.second,
            exploded.millisecond);

        return str;
    }

    int GetCurrentDate()
    {
        return GetDateFromTime(base::Time::NowFromSystemTime());
    }

    int GetCurrentDate(int64_t time_stamp)
    {
        auto time = base::Time::FromTimeT(time_stamp);
        base::Time::Exploded exploded;
        time.LocalExplode(&exploded);
        return exploded.year * 10000 + exploded.month * 100 + exploded.day_of_month;
    }
}