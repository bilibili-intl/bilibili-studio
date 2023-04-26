#pragma once

#include "ui/views/layout/layout_manager.h"
#include "ui/views/view.h"


class MeshLayout :
    public views::LayoutManager
{
public:
    MeshLayout();
    ~MeshLayout();

    void SetInset(const gfx::Insets& inset);
    void SetUnitSize(const gfx::Size& size);
    void SetUnitInterval(const gfx::Insets& inset);
    gfx::Size GetUnitSize() const;

    void Layout(views::View* host) override;
    gfx::Size GetPreferredSize(views::View* host) override;
    int GetPreferredHeightForWidth(views::View* host, int width) override;

private:
    int             col_size = 0;
    gfx::Insets     layout_insets;
    gfx::Size       unit_size_;
    gfx::Insets     unit_interval_;
};
