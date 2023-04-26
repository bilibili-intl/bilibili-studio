#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_LAYOUT_USER_INFO_LAYOUT_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_LAYOUT_USER_INFO_LAYOUT_H_

#include <map>

#include "base/basictypes.h"
#include "ui/views/layout/layout_manager.h"


class UserInfoLayout : public views::LayoutManager {
public:
    UserInfoLayout();

    gfx::Size GetPreferredSize(views::View* host) override;
    int GetPreferredHeightForWidth(views::View* host, int width) override;
    void Layout(views::View* host) override;

    void SetLayoutParams(int pos, int left_margin, int right_margin, bool need_shrink = false);

private:
    struct LayoutParams {
        int left_margin = 0;
        int right_margin = 0;
        bool need_shrink = false;
    };

    std::map<int, LayoutParams> params_;

    DISALLOW_COPY_AND_ASSIGN(UserInfoLayout);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_LAYOUT_USER_INFO_LAYOUT_H_