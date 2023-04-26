#include "bililive/bililive/ui/views/tabarea/tabarea_materials_view.h"

#include "bilibase/basic_types.h"

#include "base/notification/notification_service.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_number_conversions.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/controls/menu/menu_runner.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/command_updater_delegate.h"
#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/livehime/obs/source_creator.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_menu.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/tabarea/tabarea_materials_widget.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/secret/public/event_tracking_service.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_toast_notify_view.h"
#include "ui/views/layout/box_layout.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_property_base_view.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"

using namespace TabAreaMaterials;
namespace
{
    const int kMaterialLineCount = 4;

    class ButtonNotesView;
    ButtonNotesView* g_notes_view = nullptr;

    class ButtonNotesView : public views::WidgetDelegateView
    {
    public:
        ButtonNotesView()
        {
            image_ = new views::ImageView();
            image_->set_interactive(false);
            image_->SetVerticalAlignment(views::ImageView::Alignment::CENTER);
            image_->SetHorizontalAlignment(views::ImageView::Alignment::CENTER);
            gfx::ImageSkia* img = GetImageSkiaNamed(IDR_LIVEHIME_V3_SOURCE_SCREEN_NOTES);;
            image_->SetImage(img);//需要初始化一个图片，才能拿到size

            label_title_ = new BililiveLabel(L"");
            label_title_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
            label_title_->SetVerticalAlignment(gfx::ALIGN_VCENTER);
            label_title_->SetFont(ftFourteenBold /*ftNormalBold2 ftPrimary*/);
            label_title_->SetTextColor(SkColorSetRGB(0x2c, 0x44, 0x57));

            label_text_ = new BililiveLabel(L"");
            label_text_->SetMultiLine(true);
            label_text_->SetAllowCharacterBreak(true);
            label_text_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
            label_text_->SetVerticalAlignment(gfx::ALIGN_TOP);
            label_text_->SetFont(ftThirteen);
            label_text_->SetTextColor(SkColorSetRGB(0x2c, 0x44, 0x57));

            AddChildView(image_);
            AddChildView(label_title_);
            AddChildView(label_text_);

            views::Widget* widget = new views::Widget();
            widget->set_frame_type(views::Widget::FRAME_TYPE_FORCE_CUSTOM);
            views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
            params.remove_standard_frame = true;
            params.delegate = this;
            params.remove_taskbar_icon = true;
            params.double_buffer = true;
            params.keep_on_top = true;
            params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;
            params.accept_events = false;
            widget->Init(params);
        }

        ~ButtonNotesView()
        {

        }

        void Hide()
        {
            GetWidget()->Hide();
        }

		void Close()
		{
			GetWidget()->Close();
		}

        void Show(const gfx::Rect& show_area, const base::string16& text_title, const base::string16& text_notes,
            gfx::ImageSkia* img_notes)
        {
            label_title_->SetText(text_title);
            label_text_->SetText(text_notes);
            image_->SetImage(img_notes);
            //gfx::Rect hnv_s = GetWidget()->GetWindowBoundsInScreen();
            GetWidget()->SetBounds(gfx::Rect(
                show_area.x() + (show_area.width() - width()) / 2,
                show_area.y() + (show_area.height() - height()) / 2,
                width(), height()));
            GetWidget()->ShowInactive();
        }

    protected:
        views::View* GetContentsView() override { return this; }

        gfx::Size GetPreferredSize()override
        {
            gfx::Size pref_size;
            pref_size.set_width(kMaterialButtonWidth);
            pref_size.set_height(kMaterialButtonHeight);
            return pref_size;
        }

        void Layout()override
        {
            int space = GetLengthByDPIScale(8);
            gfx::Size img_size = image_->GetPreferredSize();
            gfx::Size lab_title_size = label_title_->GetPreferredSize();
            lab_title_size.set_height(ftNormalBold2.GetHeight());
            image_->SetBounds(width()- img_size.width() - space, height() - img_size.height(), img_size.width(), img_size.height());//右下角
            int top = GetLengthByDPIScale(5);
            label_title_->SetBounds(space, top, width() - space * 2, lab_title_size.height());
            top += lab_title_size.height();
            label_text_->SetBounds(space, top, width() - space * 2, height() - top);
        }

        void OnPaintBackground(gfx::Canvas* canvas)override
        {
            __super::OnPaintBackground(canvas);

            const gfx::Rect& rect = GetContentsBounds();
            // 圆角背景
            int radius = GetLengthByDPIScale(4);
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setColor(SkColorSetRGB(0xDB, 0xF5, 0xFF));
            paint.setStyle(SkPaint::kFill_Style);
            canvas->DrawRoundRect(GetContentsBounds(), radius, paint);
        }

    private:
        views::ImageView* image_;
        BililiveLabel* label_title_;
        BililiveLabel* label_text_;
    };

    void ShowNoteView(const gfx::Rect& show_area, const base::string16& text_title, const base::string16& text_notes,
        gfx::ImageSkia* img_notes)
    {
        if (g_notes_view == nullptr)
        {
            g_notes_view = new ButtonNotesView();
        }
        g_notes_view->Show(show_area, text_title, text_notes, img_notes);
    }

    void HideNoteView()
    {
        if (g_notes_view)
        {
            g_notes_view->Hide();
        }
    }

	void CloseNoteView()
	{
		if (g_notes_view)
		{
			g_notes_view->Close();
            g_notes_view = nullptr;
		}
	}

    // 横竖屏展示的素材是不一样的
    static std::vector<MaterialType> g_portrait_valid_materials{

        MaterialType::Camera,
        MaterialType::Window,
        MaterialType::Screen,
        MaterialType::Image,
        MaterialType::Text,
        MaterialType::Browser,
        MaterialType::Media,
    };

    std::map<MaterialType, MaterialButton*> material_table;
}

void TabAreaMaterials::HideMaterialButtonNoteView()
{
    HideNoteView();
}

MaterialButton::MaterialButton(views::ButtonListener* listener, MaterialType type)
	: LivehimeVerticalLabelButton(listener, L"")
	, material_type_(type)
{
    bg_color_ = GetColor(TabbedBkHov);
	SetColor(views::Button::STATE_NORMAL, SkColorSetRGB(0x2C, 0x44, 0x57));
	SetColor(views::Button::STATE_HOVERED, SkColorSetRGB(0x2C, 0x44, 0x57));
	SetColor(views::Button::STATE_PRESSED, SkColorSetRGB(0x2C, 0x44, 0x57));

	PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();

	gfx::ImageSkia* normal_img = nullptr;
	switch (type)
	{
	case MaterialType::Screen:
		normal_img = GetImageSkiaNamed(IDR_LIVEHIME_V3_SOURCE_FULL_SCREEN);
		img_notes_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_SOURCE_FULL_SCREEN_NOTES);
		text_title_ = GetLocalizedString(IDS_TOOLBAR_SCREEN_FULL);
		text_notes_ = GetLocalizedString(IDS_TOOLBAR_SCREEN_FULL_NOTES);
		break;
	case MaterialType::Window:
		normal_img = GetImageSkiaNamed(IDR_LIVEHIME_V3_SOURCE_WINDOW);
		img_notes_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_SOURCE_WINDOW_NOTES);
		text_title_ = GetLocalizedString(IDS_TOOLBAR_SCREEN_WINDOW);
		text_notes_ = GetLocalizedString(IDS_TOOLBAR_SCREEN_WINDOW_NOTES);
		break;
	case MaterialType::Text:
		normal_img = GetImageSkiaNamed(IDR_LIVEHIME_V3_SOURCE_TEXT);
		img_notes_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_SOURCE_TEXT_NOTES);
		text_title_ = GetLocalizedString(IDS_TOOLBAR_TEXT);
		text_notes_ = GetLocalizedString(IDS_TOOLBAR_TEXT_NOTES);
		break;
	case MaterialType::Image:
		normal_img = GetImageSkiaNamed(IDR_LIVEHIME_V3_SOURCE_IMAGE);
		img_notes_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_SOURCE_IMAGE_NOTES);
		text_title_ = GetLocalizedString(IDS_TOOLBAR_PICTURE);
		text_notes_ = GetLocalizedString(IDS_TOOLBAR_PICTURE_NOTES);
		break;
	case MaterialType::Camera:
		normal_img = GetImageSkiaNamed(IDR_LIVEHIME_V3_SOURCE_CAMERA);
		img_notes_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_SOURCE_CAMERA_NOTES);
		text_title_ = GetLocalizedString(IDS_TOOLBAR_CAMERA);
		text_notes_ = GetLocalizedString(IDS_TOOLBAR_CAMERA_NOTES);
		break;
	case MaterialType::Media:
		normal_img = GetImageSkiaNamed(IDR_LIVEHIME_V3_SOURCE_MEDIA);
		img_notes_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_SOURCE_MEDIA_NOTES);
		text_title_ = GetLocalizedString(IDS_TOOLBAR_VIDEO);
		text_notes_ = GetLocalizedString(IDS_TOOLBAR_VIDEO_NOTES);
		break;
	case MaterialType::Browser:
		normal_img = GetImageSkiaNamed(IDR_LIVEHIME_V3_SOURCE_WEBBROWSER);
		img_notes_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_SOURCE_WEBBROWSER_NOTES);
		text_title_ = GetLocalizedString(IDS_TOOLBAR_BROWSER);
		text_notes_ = GetLocalizedString(IDS_TOOLBAR_BROWSER_NOTES);
		break;
	default:
		NOTREACHED() << "no impl type";
		break;
	}

	SetText(text_title_);
	SetImage(views::Button::STATE_NORMAL, normal_img);
	SetImage(views::Button::STATE_HOVERED, normal_img);
	set_id(bilibase::enum_cast(type));
}

MaterialButton::~MaterialButton()
{

}

gfx::Size MaterialButton::GetPreferredSize()
{
	//static int max_cx = LivehimePaddingCharWidth(ftPrimary) * 5;
	gfx::Size pref_size = __super::GetPreferredSize();
	//pref_size.set_width(std::max(pref_size.width(), max_cx));
	//pref_size.Enlarge(0, GetLengthByDPIScale(6));
	pref_size.set_width(kMaterialButtonWidth);
	pref_size.set_height(kMaterialButtonHeight);
	return pref_size;
};

void MaterialButton::OnPaint(gfx::Canvas* canvas)
{
	__super::OnPaint(canvas);

	if (new_tag_)
	{
		static gfx::ImageSkia* img = GetImageSkiaNamed(IDR_LIVEHIME_V3_SOURCE_PROPERTY_SP_NEW);
		int x = GetLocalBounds().x() + (GetLocalBounds().width() - img->width());
		int y = GetLocalBounds().y();

		canvas->DrawImageInt(*img, x, y);
	}
}

// Button
void MaterialButton::NotifyClick(const ui::Event& event)
{
	__super::NotifyClick(event);
}

void MaterialButton::Layout()
{
	views::ImageView* image_ = image_view();
	BililiveLabel* label_ = label_view();
	if (image_ && label_)
	{
		gfx::Size img_size = image_->GetPreferredSize();
		gfx::Size lab_size = label_->GetPreferredSize();
		int content_height = img_size.height() + GetLengthByDPIScale(5) + lab_size.height();
		int top = (height() - content_height) / 2;
		image_->SetBounds((width() - img_size.width()) / 2, top,
			img_size.width(), img_size.height());
		label_->SetBounds((width() - lab_size.width()) / 2, top + img_size.height() + GetLengthByDPIScale(5),
			lab_size.width(), lab_size.height());
	}
}

void MaterialButton::OnPaintBackground(gfx::Canvas* canvas)
{
	__super::OnPaintBackground(canvas);

	auto bound = GetContentsBounds();
	SkPaint paint;
	paint.setAntiAlias(true);
	paint.setColor(bg_color_);
	paint.setStyle(SkPaint::kFill_Style);
	canvas->DrawRoundRect(bound, GetLengthByDPIScale(4), paint);
    if (checked_)
    {
		paint.setColor(SkColorSetRGB(0x0E, 0xBE, 0xFF));
        bililive::DrawRoundRect(canvas, 0, 0, bound.width(), bound.height(), GetLengthByDPIScale(5), paint);
        bililive::DrawRoundRect(canvas, 1, 1, bound.width()-2, bound.height()-2, GetLengthByDPIScale(3), paint);
    }
}

void MaterialButton::OnMouseEntered(const ui::MouseEvent& event)
{
	__super::OnMouseEntered(event);
	gfx::Rect hnv_s = GetBoundsInScreen();// GetWidget()->GetWindowBoundsInScreen();
	ShowNoteView(hnv_s, text_title_, text_notes_, img_notes_);
}

void MaterialButton::OnMouseExited(const ui::MouseEvent& event)
{
	__super::OnMouseExited(event);
	HideNoteView();
}

void MaterialButton::SetChecked(bool checked)
{
    checked_ = checked;
    this->SchedulePaint();
}


TabAreaMaterialsView::TabAreaMaterialsView()
    : weakptr_factory_(this)

{
    is_install_ios_drive_ = GetBililiveProcess()->bililive_obs()->obs_view()->GetInstalledResult();
}

TabAreaMaterialsView::~TabAreaMaterialsView()
{
    CloseNoteView();
}

// static
int TabAreaMaterialsView::GetMinimumHeight()
{
    static int min_cy = 0;
    if (0 == min_cy)
    {
        scoped_ptr<MaterialButton> button(new MaterialButton(nullptr, MaterialType::Begin));
        min_cy = button->GetPreferredSize().height();
        min_cy += kPaddingRowHeightForGroupCtrls * 2;
    }
    return min_cy;
}

// static
void TabAreaMaterialsView::DoPendingWork(DataType data_type, int menu_id, int button_id, bool is_install_ios_drive)
{
    secret::BehaviorEventMaterialType event_msg_id = secret::BehaviorEventMaterialType::Unknown;//埋点对应id
    if (data_type == DataType::Menu)
    {
        switch (menu_id)
        {
        case IDC_LIVEHIME_ADD_ALBUM_SOURCE_FROM_MENU:
        case IDC_LIVEHIME_ADD_COLOR_SOURCE_FROM_MENU:
        case IDC_LIVEHIME_ADD_IMAGE_SOURCE_FROM_MENU:
        case IDC_LIVEHIME_ADD_AUDIO_INPUT_FROM_MENU:
        case IDC_LIVEHIME_ADD_AUDIO_OUTPUT_FROM_MENU:
            bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), menu_id);
            if (menu_id == IDC_LIVEHIME_ADD_ALBUM_SOURCE_FROM_MENU || menu_id == IDC_LIVEHIME_ADD_COLOR_SOURCE_FROM_MENU || menu_id == IDC_LIVEHIME_ADD_IMAGE_SOURCE_FROM_MENU)
            {
                event_msg_id = secret::BehaviorEventMaterialType::Image;
            }
            break;
        default:
            break;
        }
    }
    else if (data_type == DataType::Button)
    {
        MaterialType type = MaterialType(button_id);
        switch (type)
        {
        case MaterialType::Screen:
        {
            // 抓屏
            event_msg_id = secret::BehaviorEventMaterialType::Screen;
            bililive::CreatingSourceParams params(bililive::SourceType::Monitor);
            bililive::ExecuteCommandWithParams(
                GetBililiveProcess()->bililive_obs(),
                IDC_LIVEHIME_ADD_SOURCE,
                CommandParams<bililive::CreatingSourceParams>(&params));
        }
        break;
        case MaterialType::Window:
        {
            // 抓进程
            event_msg_id = secret::BehaviorEventMaterialType::Window;
            bililive::CreatingSourceParams params(bililive::SourceType::Window);
            bililive::ExecuteCommandWithParams(
                GetBililiveProcess()->bililive_obs(),
                IDC_LIVEHIME_ADD_SOURCE,
                CommandParams<bililive::CreatingSourceParams>(&params));
        }
        break;
        case MaterialType::Text:
        {
            event_msg_id = secret::BehaviorEventMaterialType::Text;
            bililive::CreatingSourceParams params(bililive::SourceType::Text);
            bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_ADD_SOURCE,
                CommandParams<bililive::CreatingSourceParams>(&params));
        }
        break;
        case MaterialType::Camera:
        {
            event_msg_id = secret::BehaviorEventMaterialType::Camera;
            bililive::CreatingSourceParams params(bililive::SourceType::Camera);
            bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_ADD_SOURCE,
                CommandParams<bililive::CreatingSourceParams>(&params));
        }
        break;
        case MaterialType::Media:
        {
            event_msg_id = secret::BehaviorEventMaterialType::Media;
            bililive::CreatingSourceParams params(bililive::SourceType::Media);
            bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_ADD_SOURCE,
                CommandParams<bililive::CreatingSourceParams>(&params));
        }
        break;
        case MaterialType::Browser:
        {
            event_msg_id = secret::BehaviorEventMaterialType::Browser;
            bililive::CreatingSourceParams params(bililive::SourceType::Browser);
            bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_ADD_SOURCE,
                CommandParams<bililive::CreatingSourceParams>(&params));
        }
        break;
        default:
            NOTREACHED() << "no impl type";
            break;
        }
    }

    if (event_msg_id != secret::BehaviorEventMaterialType::Unknown)
    {
        livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::MaterialItemClick,
            "button_type:" + std::to_string((size_t)event_msg_id));
    }
}

void TabAreaMaterialsView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();

            LiveModelController::GetInstance()->AddObserver(this);
        }
        else
        {
            if (LiveModelController::GetInstance())
            {
                LiveModelController::GetInstance()->RemoveObserver(this);
            }
        }
    }
}

gfx::Size TabAreaMaterialsView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();

    if (current_model_is_landscape_)
    {
        landscape_mode_prefersize_ = size;
    }
    else
    {
        size = landscape_mode_prefersize_;
    }

    return size;
}

void TabAreaMaterialsView::OnWindowClosing() {
    if (pending_data_type_ != DataType::None) {
        BililiveThread::PostTask(
            BililiveThread::UI, FROM_HERE,
            base::Bind(&TabAreaMaterialsView::DoPendingWork, pending_data_type_, menu_id_, button_id_,is_install_ios_drive_));
    }
}

void TabAreaMaterialsView::OnWindowDeactive()
{
	//BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,base::Bind(&TabAreaMaterialsView::ExecuteCommand, base::Unretained(this),IDC_LIVEHIME_ADD_ALBUM_SOURCE_FROM_MENU));
    HideNoteView();
}

void TabAreaMaterialsView::ExecuteCommand(int command_id)
{
    switch (command_id)
    {
    case IDC_LIVEHIME_ADD_ALBUM_SOURCE_FROM_MENU:
    case IDC_LIVEHIME_ADD_COLOR_SOURCE_FROM_MENU:
    case IDC_LIVEHIME_ADD_IMAGE_SOURCE_FROM_MENU:
    case IDC_LIVEHIME_ADD_AUDIO_INPUT_FROM_MENU:
    case IDC_LIVEHIME_ADD_AUDIO_OUTPUT_FROM_MENU:
        pending_data_type_ = DataType::Menu;
        menu_id_ = command_id;
        GetWidget()->Close();
        break;
    default:
        break;
    }
}

void TabAreaMaterialsView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    bool close_window = true;
    MaterialType type = MaterialType(sender->id());

    switch (type)
    {
    case MaterialType::Image:
        {
            close_window = false;
            LivehimeMenuItemView *menu = new LivehimeMenuItemView(this);
            menu->AppendMenuItem(IDC_LIVEHIME_ADD_ALBUM_SOURCE_FROM_MENU, GetLocalizedString(IDS_TOOLBAR_ALBUM));
            menu->AppendMenuItem(IDC_LIVEHIME_ADD_IMAGE_SOURCE_FROM_MENU, GetLocalizedString(IDS_TOOLBAR_PICTURE));

            views::MenuRunner menuRunner(menu);
            gfx::Point mouseLocation;
            gfx::Rect rt = sender->GetBoundsInScreen();
            mouseLocation.SetPoint(rt.x() + rt.width() / 2, rt.y());
            menuRunner.RunMenuAt(sender->GetWidget(),
                nullptr,
                gfx::Rect(mouseLocation, gfx::Size(0, 0)),
                views::MenuItemView::BOTTOMCENTER,
                ui::MENU_SOURCE_TOUCH,
                views::MenuRunner::CONTEXT_MENU);
        }
        break;
    case MaterialType::Screen:  // 抓屏
    case MaterialType::Window:  // 抓进程
    case MaterialType::Text:
    case MaterialType::Camera:
    case MaterialType::Media:
    case MaterialType::Browser:
        pending_data_type_ = DataType::Button;
        button_id_ = sender->id();
        break;

    default:
        NOTREACHED() << "no impl type";
        break;
    }

    if (close_window) {
        GetWidget()->Close();
    }
}

void TabAreaMaterialsView::InitViews()
{
    SetSetValidMaterials(LiveModelController::GetInstance()->IsLandscapeModel());
}

void TabAreaMaterialsView::OnLiveLayoutModelChanged(bool user_invoke)
{
    SetSetValidMaterials(LiveModelController::GetInstance()->IsLandscapeModel());
}

void TabAreaMaterialsView::SetSetValidMaterials(bool landscape)
{
    current_model_is_landscape_ = landscape;
    RemoveAllChildViews(true);

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet *column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(14));
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(14));

    material_table.clear();

    if (landscape)
    {
        int valid_index = bilibase::enum_cast(MaterialType::Begin);
        for (int i = bilibase::enum_cast(MaterialType::Begin);
            i < bilibase::enum_cast(MaterialType::End); i++)
        {
            if (valid_index % kMaterialLineCount == 0)
            {
                layout->AddPaddingRow(0, (valid_index == 0) ? GetLengthByDPIScale(14) : GetLengthByDPIScale(8));
                layout->StartRow(0, 0);
            }
            auto material_button = new MaterialButton(this, MaterialType(i));
            layout->AddView(material_button);
            material_table.insert(std::make_pair(MaterialType(i), material_button));
            valid_index++;
        }
    }
    else
    {
        for (size_t i = 0; i < g_portrait_valid_materials.size(); i++)
        {
            if (i % kMaterialLineCount == 0)
            {
                layout->AddPaddingRow(0,(i == 0) ? GetLengthByDPIScale(14) : GetLengthByDPIScale(8));
                layout->StartRow(0, 0);
            }
            auto material_button = new MaterialButton(this, g_portrait_valid_materials[i]);
            layout->AddView(material_button);
            material_table.insert(std::make_pair(g_portrait_valid_materials[i], material_button));
        }
    }

    layout->AddPaddingRow(0, GetLengthByDPIScale(14));

    PreferredSizeChanged();
}
