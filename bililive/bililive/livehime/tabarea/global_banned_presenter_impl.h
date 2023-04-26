#ifndef BILILIVE_BILILIVE_LIVEHIME_TABAREA_GLOBAL_BANNED_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_TABAREA_GLOBAL_BANNED_PRESENTER_IMPL_H_

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"

#include "bililive/bililive/livehime/tabarea/global_banned_contract.h"

class GlobalBannedPresenterImpl : public contracts::GlobalBannedPresenter
{
public:
    explicit GlobalBannedPresenterImpl(contracts::GlobalBannedView* view);

    ~GlobalBannedPresenterImpl() = default;

    void GetBannedInfo() override;

    void GlobalBanned(bool banned, int minute, const std::string& type, int level) override;

private:
    void OnBannedInfo(bool valid_response, int code, int second);

    void OnGlobalBannedOn(bool valid_response, int code, const std::string& error_msg);

    void OnGlobalBannedOff(bool valid_response, int code, const std::string& error_msg);

    DISALLOW_COPY_AND_ASSIGN(GlobalBannedPresenterImpl);

private:
    contracts::GlobalBannedView* view_;
    base::WeakPtrFactory<GlobalBannedPresenterImpl> weak_ptr_factory_;
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_TABAREA_GLOBAL_BANNED_PRESENTER_IMPL_H_