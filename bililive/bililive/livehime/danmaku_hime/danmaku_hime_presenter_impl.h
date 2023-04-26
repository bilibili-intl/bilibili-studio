#ifndef BILILIVE_BILILIVE_LIVEHIME_DANMAKU_HIME_DANMAKU_HIME_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_DANMAKU_HIME_DANMAKU_HIME_PRESENTER_IMPL_H_

#include "base/memory/weak_ptr.h"
#include "base/timer/timer.h"
#include "bililive/bililive/livehime/danmaku_hime/danmaku_hime_contract.h"

#include <mutex>

class DanmakuHimePresenterImpl
    : public contracts::DanmakuHimePresenter {
public:
    DanmakuHimePresenterImpl();

    bool IsInitialized() override;

    std::string RequestImageSync(const std::string& url) override;

private:
    int                                             gift_config_list_invoking_count_ = 0;
    int                                             title_list_invoking_count_ = 0;
    int64_t                                         gcl_invoking_start_ts_ = 0;
    int64_t                                         tl_invoking_start_ts_ = 0;

    bool                                            is_initialized_ = false;
    std::mutex                                      lock_;

    base::WeakPtrFactory<DanmakuHimePresenterImpl>  weak_ptr_factory_;
    DISALLOW_COPY_AND_ASSIGN(DanmakuHimePresenterImpl);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_DANMAKU_HIME_DANMAKU_HIME_PRESENTER_IMPL_H_