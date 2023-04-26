#ifndef BILILIVE_BILILIVE_LIVEHIME_MAIN_VIEW_LIVEHIME_MAIN_CLOSE_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_MAIN_VIEW_LIVEHIME_MAIN_CLOSE_PRESENTER_IMPL_H_

#include "base/basictypes.h"

#include "bililive/bililive/livehime/main_view/livehime_main_close_contract.h"

class LivehimeClosePresenterImpl
    : public contracts::LivehimeClosePresenter
{
public:
    LivehimeClosePresenterImpl();

    ~LivehimeClosePresenterImpl() = default;

    // contracts::LivehimeClosePresenter.
    int GetExitStyle() override;

    bool GetIsRemember() override;

    void SaveExitStyle(int exit_style) override;

    void SaveIsRemember(bool is_remember) override;

private:
    DISALLOW_COPY_AND_ASSIGN(LivehimeClosePresenterImpl);
};


#endif  // BILILIVE_BILILIVE_LIVEHIME_MAIN_VIEW_LIVEHIME_MAIN_CLOSE_PRESENTER_IMPL_H_