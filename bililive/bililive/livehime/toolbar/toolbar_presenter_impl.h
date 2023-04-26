#ifndef BILILIVE_BILILIVE_LIVEHIME_TOOLBAR_TOOLBAR_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_TOOLBAR_TOOLBAR_PRESENTER_IMPL_H_

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"
#include "bililive/public/common/refcounted_dictionary.h"

#include "bililive/bililive/livehime/toolbar/toolbar_contract.h"
//#include "bililive/bililive/livehime/voice_link/voice_link_ui_presenter.h"


class ToolbarPresenterImpl
    : public contracts::ToolbarPresenter
{
public:
    explicit ToolbarPresenterImpl(contracts::ToolbarView* view);

    ~ToolbarPresenterImpl();

private:
    contracts::ToolbarView* view_;
    base::WeakPtrFactory<ToolbarPresenterImpl> weak_ptr_factory_;
    DISALLOW_COPY_AND_ASSIGN(ToolbarPresenterImpl);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_TOOLBAR_TOOLBAR_PRESENTER_IMPL_H_