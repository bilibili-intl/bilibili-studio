#ifndef BILILIVE_BILILIVE_UI_VIEWS_TABAREA_LIVE_FUNCTION_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_TABAREA_LIVE_FUNCTION_VIEW_H_

#include "ui/views/controls/button/button.h"
#include "ui/views/controls/menu/menu_delegate.h"
#include "ui/views/widget/widget_observer.h"

#include "bililive/bililive/livehime/live_model/live_model_controller.h"
//#include "bililive/bililive/livehime/vtuber/vtuber_ui_presenter.h"
#include "bililive/bililive/ui/views/controls/bililive_floating_scroll_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_monitor_capture_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"

namespace TabAreaMaterials
{
	const int kMaterialButtonWidth = GetLengthByDPIScale(200);
	const int kMaterialButtonHeight = GetLengthByDPIScale(130);

	enum class MaterialType
	{
		Begin,

		Camera = Begin,
		Window,
		Screen,
		Image,
		Text,
		Media,
		Browser,
		End,
		// end of there
	};

	void HideMaterialButtonNoteView();

	class MaterialButton : public LivehimeVerticalLabelButton
	{
	public:
		MaterialButton(views::ButtonListener* listener, MaterialType type);
		virtual ~MaterialButton();
		// View
		gfx::Size GetPreferredSize() override;
		void SetChecked(bool checked);
		void SetBackgroundColor(SkColor color) { bg_color_ = color; }
	protected:
		// View
		void OnPaint(gfx::Canvas* canvas) override;
		// Button
		void NotifyClick(const ui::Event& event) override;

		void Layout()override;
		void OnPaintBackground(gfx::Canvas* canvas) override;
		void OnMouseEntered(const ui::MouseEvent& event)override;
		void OnMouseExited(const ui::MouseEvent& event)override;
	private:
		bool new_tag_ = false;
		bool checked_ = false;
		MaterialType material_type_;
		SkColor bg_color_;
		base::string16 text_title_;
		base::string16 text_notes_;
		gfx::ImageSkia* img_notes_ = nullptr;
	};
}


class TabAreaMaterialsView
    : public BililiveViewWithFloatingScrollbar
    , public views::ButtonListener
    , views::MenuDelegate
    , LiveModelControllerObserver
    //, public livehime::vtb::VtuberUIObserver
{
public:
    TabAreaMaterialsView();
    ~TabAreaMaterialsView();

    void OnWindowClosing();

    void OnWindowDeactive();

    static int GetMinimumHeight();

protected:
    // View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) override;
    gfx::Size GetPreferredSize() override;

    // MenuDelegate
    void ExecuteCommand(int command_id) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // LiveModelControllerObserver
    void OnLiveLayoutModelChanged(bool user_invoke) override;

    // vtb::VtuberUIObserver
    //void OnVtuberEntranceInfo(bool is_open) override;

private:
    enum class DataType {
        None,
        Menu,
        Button,
    };

    void InitViews();
    void SetSetValidMaterials(bool landscape);

    static void DoPendingWork(DataType data_type, int menu_id, int button_id, bool is_install_ios_drive = false);

    bool current_model_is_landscape_ = true;
    gfx::Size landscape_mode_prefersize_;

    int menu_id_ = 0;
    int button_id_ = 0;
    DataType pending_data_type_ = DataType::None;
	bool is_install_ios_drive_ = false;

    base::WeakPtrFactory<TabAreaMaterialsView> weakptr_factory_;
    DISALLOW_COPY_AND_ASSIGN(TabAreaMaterialsView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_TABAREA_TAB_PANE_DANMAKU_VIEW_H_