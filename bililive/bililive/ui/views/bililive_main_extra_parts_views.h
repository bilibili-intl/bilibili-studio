#ifndef BILILIVE_BILILIVE_UI_VIEWS_BILILIVE_MAIN_EXTRA_PARTS_VIEWS_H_
#define BILILIVE_BILILIVE_UI_VIEWS_BILILIVE_MAIN_EXTRA_PARTS_VIEWS_H_

#include "base/basictypes.h"

#include "bililive/public/bililive/bililive_main_extra_parts.h"

class BililiveMainExtraPartsViews
    : public BililiveMainExtraParts {
public:
    BililiveMainExtraPartsViews();

    void ToolkitInitialized() override;

    void PostBililiveStart() override;

private:
    DISALLOW_COPY_AND_ASSIGN(BililiveMainExtraPartsViews);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_BILILIVE_MAIN_EXTRA_PARTS_VIEWS_H_