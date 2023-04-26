#pragma once

#include <vector>

#include "ui/gfx/insets.h"
#include "ui/gfx/size.h"
#include "ui/views/layout/layout_manager.h"


namespace bililive
{
    class LinearLayout : public views::LayoutManager
    {
    public:
        enum AlignType { kLeading, kCenter, kTailing };
        enum LayoutType { kHoirzontal, kVertical };

    public:
        LinearLayout(LayoutType layout_type, views::View* host);
        virtual ~LinearLayout();

        void Layout(views::View* host) override;
        gfx::Size GetPreferredSize(views::View* host) override;

        void SetInset(const gfx::Insets& inset);

        void AddView(
            views::View* child,
            AlignType horiz = AlignType::kLeading,
            AlignType vert = AlignType::kLeading,
            int left_padding = 0,
            int top_padding = 0,
            int right_padding = 0,
            int bottom_padding = 0);
        
        //按说应该实现ViewRemoved父类方法，但是为了不影响已有逻辑，这里主动调用实现删除
        void RemoveView(views::View* child);

    private:
        struct AlignParam
        {
            views::View* child = nullptr;
            AlignType horiz = AlignType::kLeading;
            AlignType vert = AlignType::kLeading;
            int left_padding = 0;
            int top_padding = 0;
            int right_padding = 0;
            int bottom_padding = 0;
        };

        int Compose(std::vector<AlignParam>& view_params, int start_pos, int length, LayoutType type);


        LayoutType                  layout_type_ = LayoutType::kHoirzontal;
        views::View*                host_ = nullptr;
        gfx::Insets                 inset_;
        std::vector<AlignParam>     align_params_;
    };


};

