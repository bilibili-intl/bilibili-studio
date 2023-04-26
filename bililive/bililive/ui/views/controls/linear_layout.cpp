#include "linear_layout.h"

#include "ui/views/view.h"


namespace bililive
{
    LinearLayout::LinearLayout(LayoutType layout_type, views::View* host):
        layout_type_(layout_type),
        host_(host)
    {
    }

    LinearLayout::~LinearLayout()
    {
    }

    void LinearLayout::Layout(views::View* host)
    {
        DCHECK(host_ == host);

        auto prefer_size = host_->GetContentsBounds();
        int leading_length{ 0 }, mid_length{ 0 }, trailing_length{ 0 };
        bool is_full{ false };

        std::vector<AlignParam> leading_views_;
        std::vector<AlignParam> mid_views_;
        std::vector<AlignParam> trailing_views;

        if (layout_type_ == LayoutType::kHoirzontal)
        {
            for (auto &param : align_params_)
            {
                auto child_view = param.child;

                if (!child_view->visible())
                {
                    continue;
                }

                if (is_full)
                {
                    child_view->SetBounds(0, 0, 0, 0);
                    continue;
                }

                auto child_pref_size = child_view->GetPreferredSize();
                int y{}, height{ child_pref_size.height() };
                y += param.top_padding;
                height += param.top_padding + param.bottom_padding;
                switch (param.vert)
                {
                    case AlignType::kLeading:
                        y += 0;
                    break;

                    case AlignType::kCenter:
                        y += std::max((prefer_size.height() - height) / 2, 0);
                    break;

                    case AlignType::kTailing:
                        y += std::max((prefer_size.height() - height), 0);
                    break;

                    default:
                        DCHECK(0);
                    break;
                }

                child_view->SetBounds(0, y, child_pref_size.width(), height);

                int child_view_width = child_pref_size.width() + param.left_padding + param.right_padding;
                switch (param.horiz)
                {
                    case AlignType::kLeading:
                        leading_length += child_view_width;
                        leading_views_.push_back(param);
                    break;

                    case AlignType::kCenter:
                        mid_length += child_view_width;
                        mid_views_.push_back(param);
                    break;

                    case AlignType::kTailing:
                        trailing_length += child_view_width;
                        trailing_views.push_back(param);
                    break;

                    default:
                        DCHECK(0);
                    break;
                }

                if (leading_length + mid_length + trailing_length >= prefer_size.width())
                {
                    is_full = true;
                }
            }

            int start_pos = Compose(leading_views_, 0, prefer_size.width(), LayoutType::kHoirzontal);

            int center_pos = (prefer_size.width() - mid_length) / 2.0;
            if (start_pos > center_pos)
            {
                start_pos = center_pos;
            }
            start_pos = Compose(mid_views_, start_pos, mid_length, LayoutType::kHoirzontal);

            int trailing_pos = prefer_size.width() - trailing_length;
            if (trailing_pos > start_pos)
            {
                start_pos = trailing_pos;
            }
            Compose(trailing_views, start_pos, trailing_length, LayoutType::kHoirzontal);

        }
        else
        {

        }
    }

    gfx::Size LinearLayout::GetPreferredSize(views::View* host)
    {
        DCHECK(host_ == host);

        if (layout_type_ == LayoutType::kHoirzontal)
        {
            int max_height{};
            int length{};
            for (auto& param : align_params_)
            {
                auto pref_size = param.child->GetPreferredSize();
                length += pref_size.width();
                max_height = std::max(max_height, pref_size.height());
            }

            return { length + inset_.width(), max_height + inset_.height() };
        }
        else
        {
            int max_width{};
            int height{};
            for (auto& param : align_params_)
            {
                auto pref_size = param.child->GetPreferredSize();
                height += pref_size.height();
                max_width = std::max(max_width, pref_size.width());
            }

            return { max_width + inset_.width(), height + inset_.height() };
        }
    }

    void LinearLayout::SetInset(const gfx::Insets& inset)
    {
        inset_ = inset;
    }

    void LinearLayout::AddView(
        views::View* child,
        AlignType horiz,
        AlignType vert,
        int left_padding,
        int top_padding,
        int right_padding,
        int bottom_padding)
    {
        align_params_.push_back({ child, horiz, vert, left_padding, top_padding, right_padding, bottom_padding });
        host_->AddChildView(child);
    }

    void LinearLayout::RemoveView(views::View* child)
    {
        std::vector<AlignParam>::iterator it;
		for (it = align_params_.begin();it != align_params_.end();it++)
		{
            if (it->child == child)
            {
                align_params_.erase(it);
                return;
            }
        }
    }

    int LinearLayout::Compose(std::vector<AlignParam>& view_params, int start_pos, int length, LayoutType type)
    {
        if (type == LayoutType::kHoirzontal)
        {
            for (auto& view_param : view_params)
            {
                auto child_view = view_param.child;
                auto pref_size = child_view->GetPreferredSize();

                auto& bounds = child_view->bounds();
                int padding_length = view_param.left_padding + view_param.right_padding;
                int offset_with_padding = start_pos + view_param.left_padding;
                int child_length = std::min(std::max(pref_size.width(), 0), length);
                child_view->SetBounds(offset_with_padding, bounds.y() + inset_.top(), child_length, bounds.height());
                start_pos += pref_size.width() + padding_length;
                length = std::max(length - pref_size.width() + padding_length, 0);
            }

            return start_pos;
        }
        else
        {
            for (auto& view_param : view_params)
            {
                auto child_view = view_param.child;
                auto pref_size = child_view->GetPreferredSize();

                auto& bounds = child_view->bounds();
                int offset_with_padding = start_pos + view_param.top_padding;
                int child_length = std::min(std::max(pref_size.height() - view_param.top_padding - view_param.bottom_padding, 0), length);
                child_view->SetBounds(bounds.x(), offset_with_padding, bounds.width(), child_length);
                start_pos += pref_size.height();
                length = std::max(length - pref_size.height(), 0);
            }

            return start_pos;
        }
    }
};


