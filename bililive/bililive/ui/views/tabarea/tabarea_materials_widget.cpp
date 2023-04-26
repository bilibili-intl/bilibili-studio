#include "bililive/bililive/ui/views/tabarea/tabarea_materials_widget.h"

#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_tabbed.h"
#include "bililive/bililive/ui/views/tabarea/tabarea_materials_view.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
//#include "bililive/bililive/livehime/vtuber/three_vtuber/three_vtuber_ui_presenter.h"

namespace {

    TabAreaMaterialsWidget* g_single_instance = nullptr;

    // Ñ¡Ïî¿¨±êÇ©
    class MaterialAreaTabStripView
        : public LivehimeTopStripPosStripView
    {
    public:
        explicit MaterialAreaTabStripView(const base::string16& text)
            : LivehimeTopStripPosStripView(text)
        {
        }

        virtual ~MaterialAreaTabStripView() = default;

    protected:
        // View
        void OnPaintBackground(gfx::Canvas* canvas) override
        {
            __super::OnPaintBackground(canvas);
        }

        // LivehimeTopStripPosStripView
        gfx::Rect GetLineRegion() override
        {
            static int cx = GetLengthByDPIScale(14);
            static int cy = GetLengthByDPIScale(2);
            gfx::Rect rect = GetLocalBounds();
            return gfx::Rect((width() - cx) / 2, height() - cy - GetLengthByDPIScale(6), cx, cy);
        }

        void OnStateChanged() override
        {
            switch (state())
            {
            case NavigationStripView::SS_INACTIVE:
            case NavigationStripView::SS_HOVER:
                label_->SetTextColor(clrTextSecondary);
                break;
            case NavigationStripView::SS_PRESSED:
            case NavigationStripView::SS_SELECTED:
                label_->SetTextColor(clrTextTitle);
                break;
            default:
                break;
            }
        }

    private:

        DISALLOW_COPY_AND_ASSIGN(MaterialAreaTabStripView);
    };

}

TabAreaMaterialsWidget::TabAreaMaterialsWidget()
    : BililiveWidgetDelegate(gfx::ImageSkia(), GetLocalizedString(IDS_TABAREA_MATERIALS_WIDGET_TITLE)),
      weakptr_factory_(this)

{
    SetDisableFlashFrame(true);
}

TabAreaMaterialsWidget::~TabAreaMaterialsWidget()
{
    LivehimeLiveRoomController::GetInstance()->RemoveObserver(this);
    g_single_instance = nullptr;
}

void TabAreaMaterialsWidget::ShowWindow()
{
    if (!g_single_instance)
    {
        views::Widget* parent = GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget();
        views::Widget *widget = new views::Widget();

        views::Widget::InitParams params;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.native_widget = new BililiveNativeWidgetWin(widget);
        params.parent = parent->GetNativeView();

        g_single_instance = new TabAreaMaterialsWidget();
        DoModalWidget(g_single_instance, widget, params);
    }
    else
    {
        g_single_instance->GetWidget()->Activate();
    }
}

void TabAreaMaterialsWidget::Close()
{
    if (g_single_instance)
    {
        g_single_instance->GetWidget()->Close();
    }
}

void TabAreaMaterialsWidget::Hide()
{
    if (g_single_instance)
    {
        g_single_instance->GetWidget()->Hide();
    }
}

int TabAreaMaterialsWidget::GetMinimumHeight()
{
    static int min_cy = 0;
    if (0 == min_cy)
    {
        min_cy = TabAreaMaterialsView::GetMinimumHeight();
        min_cy += GetLengthByDPIScale(40);
    }
    return min_cy;
}

void TabAreaMaterialsWidget::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails & details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();
            LivehimeLiveRoomController::GetInstance()->AddObserver(this);
        }
    }
}

void TabAreaMaterialsWidget::InitViews()
{
    SetLayoutManager(new views::FillLayout());

    tabbed_pane_ = new NavigationBar(nullptr, NAVIGATIONBAR_TABSTRIP_TOP, false);

    materials_view_ = new TabAreaMaterialsView();

    material_strip_ = new MaterialAreaTabStripView(GetLocalizedString(IDS_TABAREA_SCENE_MATERIALS));
    tabbed_pane_->AddTab(L"", material_strip_, materials_view_->Container());
    material_strip_->SetVisible(false);

    AddChildView(tabbed_pane_);
}

void TabAreaMaterialsWidget::OnEnterIntoThirdPartyStreamingMode()
{
    SetResultCode(IDCANCEL);
    this->GetWidget()->Close();
}

gfx::Size TabAreaMaterialsWidget::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();

    size.SetSize(GetLengthByDPIScale(852), GetLengthByDPIScale(464));
    // ref: kMaterialButtonWidth * 4 + 52

    return size;
}

void TabAreaMaterialsWidget::WindowClosing() {
    materials_view_->OnWindowClosing();
}

void TabAreaMaterialsWidget::OnWidgetActivationChanged(views::Widget* widget, bool active)
{
    //LOG(INFO) << "OnWidgetActivationChanged active :" << widget << "  " << active;
    if (!active)
    {
        if (materials_view_)
        {
            materials_view_->OnWindowDeactive();
        }
    }
}