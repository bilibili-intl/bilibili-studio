#ifndef BILILIVE_BILILIVE_LIVEHIME_DANMAKU_HIME_DMKHIME_SOURCE_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_DANMAKU_HIME_DMKHIME_SOURCE_PRESENTER_IMPL_H_

#include "bililive/bililive/livehime/danmaku_hime/dmkhime_source_contract.h"

#include "obs/bili_plugins/obs-dmkhime/dmkhime-service.h"


class DmkhimeSourceEventHandler : public bililive::IDmkhimeTexServiceCallback {
public:
    using WPtr = std::weak_ptr<IDmkhimeTexServiceCallback>;

    explicit DmkhimeSourceEventHandler(contracts::DmkhimeSourceView* view);

    // bililive::IDmkhimeTexServiceCallback
    void OnRefreshTexture(bool first) override;
    void OnAutism() override;

private:
    contracts::DmkhimeSourceView* view_;
};

class DmkhimeSourcePresenterImpl : public contracts::DmkhimeSourcePresenter {
public:
    explicit DmkhimeSourcePresenterImpl(contracts::DmkhimeSourceView* view);

    bool Initialize() override;
    void Render(gfx::Canvas* canvas) override;

private:
    std::shared_ptr<DmkhimeSourceEventHandler> callback_;
    bililive::IDmkhimeTexService* dmktex_service_ = nullptr;
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_DANMAKU_HIME_DMKHIME_SOURCE_PRESENTER_IMPL_H_