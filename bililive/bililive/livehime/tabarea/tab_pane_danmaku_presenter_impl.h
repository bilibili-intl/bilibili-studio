#ifndef BILILIVE_BILILIVE_UI_VIEWS_TABAREA_TAB_PANE_DANMAKU_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_UI_VIEWS_TABAREA_TAB_PANE_DANMAKU_PRESENTER_IMPL_H_

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"

#include "bililive/bililive/livehime/tabarea/tab_pane_danmaku_contract.h"

class TabPaneDanmakuPresenterImpl
    : public base::SupportsWeakPtr<TabPaneDanmakuPresenterImpl>,
      public contracts::TabPaneDanmakuPresenter
{
public:
    explicit TabPaneDanmakuPresenterImpl(contracts::TabPaneDanmakuView* view);

    ~TabPaneDanmakuPresenterImpl() = default;

    void InitDanmakuFromDB();

    void LoadDanmakuFromDB(contracts::UpdateDataType type);

    contracts::BarrageDetails AppendDanmaku(const DanmakuInfo &danmaku);

    void SearchDanmakuFromDB(const std::string& content, int64 time_range);

    void LocateDanmakuFromDB(int id);

    void InitDanmaku() override;

    void LoadDanmaku(contracts::UpdateDataType type) override;

    void EventTracking() override;

    void ReportDanmaku(const SelectedDanmakuDetails& details) override;

    void SearchDanmaku(const std::string& content, int64 time_range) override;

    void LocateDanmaku(int id) override;

    void RoomAdminAdd(int64_t id) override;

private:
    void OnReportDanmaku(bool valid_response, int code, const std::string& error_msg);

    void OnRoomAdminAdd(bool valid_response, int code, const std::string& error_msg);

    DISALLOW_COPY_AND_ASSIGN(TabPaneDanmakuPresenterImpl);

private:
    contracts::TabPaneDanmakuView* view_;

    base::WeakPtrFactory<TabPaneDanmakuPresenterImpl> weak_ptr_factory_;

    std::vector<contracts::BarrageDetails> barrage_list_;

    int danmaku_begin_;

    int danmaku_end_;

    int total_;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_TABAREA_TAB_PANE_DANMAKU_PRESENTER_IMPL_H_