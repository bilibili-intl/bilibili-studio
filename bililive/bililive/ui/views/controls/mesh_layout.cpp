#include "mesh_layout.h"

MeshLayout::MeshLayout()
{
}

MeshLayout::~MeshLayout()
{
}

void MeshLayout::SetInset(const gfx::Insets& inset)
{
    layout_insets = inset;
}

void MeshLayout::SetUnitSize(const gfx::Size& size)
{
    unit_size_ = size;
}

void MeshLayout::SetUnitInterval(const gfx::Insets& inset)
{
    unit_interval_ = inset;
}

gfx::Size MeshLayout::GetUnitSize() const
{
    return unit_size_;
}

void MeshLayout::Layout(views::View* host)
{
    if(host->child_count() <= 0)
    {
        return;
    }

    auto rt = host->GetContentsBounds();
    rt.Inset(layout_insets);

    gfx::Size unit_size{ unit_size_.width() + unit_interval_.width(), unit_size_.height() + unit_interval_.height() };
    gfx::Rect unit_rt{ 0, 0, unit_size.width(), unit_size.height() };

    int col = std::max(rt.width() / unit_size.width(), 1);
    //int row = std::ceil(host->child_count() / col);
    for (int i = 0; i < host->child_count(); i++)
    {
        int c = i % col;
        int r = i / col;
        auto ch_view = host->child_at(i);

        gfx::Rect ch_rt{ c * unit_size.width() , r * unit_size.height(), unit_size.width(), unit_size.height() };
        ch_rt.Inset(unit_interval_);
        ch_view->SetBoundsRect(ch_rt);
    }
}

gfx::Size MeshLayout::GetPreferredSize(views::View* host)
{
    auto rt = host->GetContentsBounds();
    int height = GetPreferredHeightForWidth(host, rt.width());
    return { rt.width() + layout_insets.width(), height };
}

int MeshLayout::GetPreferredHeightForWidth(views::View* host, int width)
{
    int unit_count = host->child_count();
    gfx::Size unit_size{ unit_size_.width() + unit_interval_.width(), unit_size_.height() + unit_interval_.height() };
    int col = std::max(width / unit_size.width(), 1);
    int row = std::ceil((float)host->child_count() / (float)col);
    return row * unit_size.height() + layout_insets.height();
}
