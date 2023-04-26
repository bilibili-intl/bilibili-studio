#include "examples_frame_view.h"

#include "base/message_loop/message_loop.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/fill_layout.h"

#include "bililive/bililive/ui/examples/button_tab_view.h"
#include "bililive/bililive/ui/examples/label_tab_view.h"
#include "bililive/bililive/ui/examples/misc_tab_view.h"
#include "bililive/bililive/ui/examples/mp4_player_view.h"

#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/navigation_bar.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_tabbed.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"


namespace examples {

    ExamplesFrameView* ExamplesFrameView::instance_ = nullptr;

    ExamplesFrameView::ExamplesFrameView()
        : BililiveWidgetDelegate(gfx::ImageSkia(), L"Example Window"),
          tab_pane_(nullptr) {
    }

    ExamplesFrameView::~ExamplesFrameView() {
        base::MessageLoop::current()->Quit();
    }


    void ExamplesFrameView::ShowWindow() {
        if (!instance_) {
            views::Widget* widget = new views::Widget();
            views::Widget::InitParams params;
            params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
            params.native_widget = new BililiveNativeWidgetWin(widget);

            instance_ = new ExamplesFrameView();
            return ShowWidget(instance_, widget, params);
        } else {
            if (instance_->GetWidget()) {
                instance_->GetWidget()->Activate();
            }
        }
    }

    views::NonClientFrameView* ExamplesFrameView::CreateNonClientFrameView(views::Widget* widget) {
        auto frame = static_cast<BililiveFrameView*>(__super::CreateNonClientFrameView(widget));
        frame->SetEnableDragSize(true);
        return frame;
    }

    void ExamplesFrameView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) {
        __super::ViewHierarchyChanged(details);

        if (details.child == this) {
            if (details.is_add) {
                InitViews();
            } else {
                UninitViews();
            }
        }
    }

    gfx::Size ExamplesFrameView::GetPreferredSize() {
        return gfx::Size(GetLengthByDPIScale(700), GetLengthByDPIScale(500));
    }

    void ExamplesFrameView::InitViews() {
        ResourceBundle& rb = ResourceBundle::GetSharedInstance();

        views::FillLayout* layout = new views::FillLayout();
        SetLayoutManager(layout);

        tab_pane_ = new NavigationBar(nullptr, NAVIGATIONBAR_TABSTRIP_LEFT, false);
        AddChildView(tab_pane_);

        tab_ids_.push_back(tab_pane_->AddTab(L"",
            new LivehimeLeftStripPosStripView(L"Labels"), new LabelTabView()));
        tab_ids_.push_back(tab_pane_->AddTab(L"",
            new LivehimeLeftStripPosStripView(L"Buttons"), new ButtonTabView()));
        tab_ids_.push_back(tab_pane_->AddTab(L"",
            new LivehimeLeftStripPosStripView(L"Misc"), new MiscTabView()));
        tab_ids_.push_back(tab_pane_->AddTab(L"",
            new LivehimeLeftStripPosStripView(L"MP4"), new Mp4PlayerView()));


        tab_pane_->SelectTabAt(tab_ids_.back());
    }

    void ExamplesFrameView::UninitViews() {
    }

}