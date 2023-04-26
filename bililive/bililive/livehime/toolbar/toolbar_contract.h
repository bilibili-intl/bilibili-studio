#ifndef BILILIVE_BILILIVE_LIVEHIME_TOOLBAR_TOOLBAR_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_TOOLBAR_TOOLBAR_CONTRACT_H_
#include "bililive/public/secret/bililive_secret.h"

namespace gfx {
    class ImageSkia;
}

namespace contracts {

class ToolbarPresenter {
public:
    virtual ~ToolbarPresenter() {}
};

class ToolbarView {
public:
    virtual ~ToolbarView() {}
};

}   // namespace contracts

#endif  // BILILIVE_BILILIVE_LIVEHIME_TOOLBAR_TOOLBAR_CONTRACT_H_