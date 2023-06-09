#ifndef BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_DANMAKU_SEARCH_NOTIFICATION_DETAILS_H
#define BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_DANMAKU_SEARCH_NOTIFICATION_DETAILS_H

namespace{

enum TimeRange {
    ALL = 0,
    WEEK,
    MONTH,
    THREE_MONTH,
};

struct DanmakuSearchDetails {
    std::string content;
    TimeRange   time_range;
};

}

#endif  // BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_DANMAKU_SEARCH_NOTIFICATION_DETAILS_H