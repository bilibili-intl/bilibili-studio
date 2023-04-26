#ifndef BILILIVE_BILILIVE_UI_VIEWS_TABAREA_TAB_PANE_DANMAKU_CONTRACT_H_
#define BILILIVE_BILILIVE_UI_VIEWS_TABAREA_TAB_PANE_DANMAKU_CONTRACT_H_
#include "base/strings/string16.h"
#include "third_party/skia/include/core/SkColor.h"

#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"

namespace contracts {

enum UpdateDataType {
    TOTOP,
    TOBOTTOM,
};

struct BarrageDetails {
    SkColor crTitleColor;
    base::string16 title;
    base::string16 stime;
    SkColor crContentColor;
    SkColor crSTime;
    base::string16 content;
    int64_t uid;
    int id;
    std::string uname;
    std::string danmaku;
    int64_t ts;
    std::string ct;

    BarrageDetails() : uid(0), id(0), ts(0) {}
};

class TabPaneDanmakuPresenter {
public:
    virtual ~TabPaneDanmakuPresenter() {}

    virtual void InitDanmaku() = 0;

    virtual void LoadDanmaku(UpdateDataType type) = 0;

    virtual void EventTracking() = 0;

    virtual void ReportDanmaku(const SelectedDanmakuDetails& details) = 0;

    virtual void SearchDanmaku(const std::string& content, int64 time_range) = 0;

    virtual void LocateDanmaku(int id) = 0;

    virtual void RoomAdminAdd(int64_t id) = 0;
};

class TabPaneDanmakuView {
public:
    virtual ~TabPaneDanmakuView() {}

    virtual void OnInitDanmaku(
        std::vector<contracts::BarrageDetails> list) = 0;

    virtual void OnLoadDanmaku(
        std::vector<contracts::BarrageDetails> list,
        int count,
        UpdateDataType type) = 0;

    virtual void OnLoadStoped() = 0;

    virtual void OnReportDanmaku(bool success) = 0;

    virtual void OnSearchDanmaku(std::vector<contracts::BarrageDetails> list) = 0;

    virtual void OnLocateDanmaku(std::vector<contracts::BarrageDetails> list) = 0;

    virtual void OnRoomAdminAdd(bool valid, int code, const std::string& error_msg) = 0;
};

}   // namespace contracts

#endif  // BILILIVE_BILILIVE_UI_VIEWS_TABAREA_TAB_PANE_DANMAKU_CONTRACT_H_