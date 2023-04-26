#include "bililive/bililive/ui/views/livehime/settings/settings_widget_common_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "ui/base/resource/resource_bundle.cc"

const int kLeftPaddingColWidth = GetLengthByDPIScale(11);
const int kLineSpaceHeightBig = GetLengthByDPIScale(10);
const int kLineSpaceHeightSmall = GetLengthByDPIScale(6);
const int kSecondLevelIndentation = GetLengthByDPIScale(18);
const int kSettingBtWidth = GetLengthByDPIScale(71);
const int kSettingBtHeight = GetLengthByDPIScale(26);
const int kSettingSliderWidth = GetLengthByDPIScale(160);

namespace {
	const SkColor clrCategory = SkColorSetRGB(0x53, 0x67, 0x77);
	const SkColor clrContent = SkColorSetARGB(0xcc, 0x53, 0x67, 0x77);//80%
	const SkColor clrNotes = SkColorSetARGB(0x99, 0x53, 0x67, 0x77);//60%
	const SkColor	 clrSplitLine = SkColorSetARGB(0x66, 0x53, 0x67, 0x77);//40%
	
}

DanmuSettingCategoryLabel::DanmuSettingCategoryLabel(const string16& text) :BililiveLabel(text)
{
	SetFont(ftFourteenBold);
	SetTextColor(clrCategory);
}


DanmuSettingTitleLabel::DanmuSettingTitleLabel(const string16& text) :BililiveLabel(text)
{
	SetFont(ftTwelve);
	SetTextColor(clrCategory);
}

DanmuSettingContentLabel::DanmuSettingContentLabel(const string16& text) :BililiveLabel(text)
{
	SetFont(ftTwelve);
	SetTextColor(clrContent);
}


DanmuSettingNotesLabel::DanmuSettingNotesLabel(const string16& text) :BililiveLabel(text)
{
	SetFont(ftTen);
	SetTextColor(clrNotes);
}


DanmuSettingCheckbox::DanmuSettingCheckbox(const base::string16& label,bool disable)
	: BililiveCheckbox(label)
{
	SetFont(ftTwelve);
	SetTextColor(Button::STATE_NORMAL, clrContent);
	SetTextColor(Button::STATE_HOVERED, clrContent);
	SetTextColor(Button::STATE_PRESSED, clrContent);
	if (disable) {
		SetTextColor(Button::STATE_DISABLED, SkColorSetA(clrContent, disable_alpha));
	}
	else {
		SetTextColor(Button::STATE_DISABLED, clrContent);
	}
	

	static gfx::ImageSkia unsel, unsel_hv, selected, selected_dis;
	static bool init = false;
	if (!init)
	{
		init = true;
		unsel = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_CHECKBOX);
		unsel_hv = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_CHECKBOX_HV);
		selected = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_CHECKBOX_SELECT);
		selected_dis = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_CHECKBOX_SELECT_DIS);
	}

	SetCustomImage(false, false, views::Button::STATE_NORMAL, unsel);
	SetCustomImage(false, true, views::Button::STATE_NORMAL, unsel);
	SetCustomImage(false, false, views::Button::STATE_HOVERED, unsel_hv);
	SetCustomImage(false, true, views::Button::STATE_HOVERED, unsel_hv);
	SetCustomImage(false, false, views::Button::STATE_PRESSED, unsel_hv);
	SetCustomImage(false, true, views::Button::STATE_PRESSED, unsel_hv);
	SetCustomImage(false, false, views::Button::STATE_DISABLED, unsel);
	SetCustomImage(false, true, views::Button::STATE_DISABLED, unsel);

	SetCustomImage(true, false, views::Button::STATE_NORMAL, selected);
	SetCustomImage(true, true, views::Button::STATE_NORMAL, selected);
	SetCustomImage(true, false, views::Button::STATE_HOVERED, selected);
	SetCustomImage(true, true, views::Button::STATE_HOVERED, selected);
	SetCustomImage(true, false, views::Button::STATE_PRESSED, selected);
	SetCustomImage(true, true, views::Button::STATE_PRESSED, selected);
	SetCustomImage(true, false, views::Button::STATE_DISABLED, selected_dis);
	SetCustomImage(true, true, views::Button::STATE_DISABLED, selected_dis);
}

DanmuVocieBroadcastCheckbox::DanmuVocieBroadcastCheckbox(const base::string16& label)
	: BililiveCheckbox(label)
{
	SetFont(ftTwelve);
	SetTextColor(Button::STATE_NORMAL, clrContent);
	SetTextColor(Button::STATE_HOVERED, clrContent);
	SetTextColor(Button::STATE_PRESSED, clrContent);
	SetTextColor(Button::STATE_DISABLED, SkColorSetA(clrContent, disable_alpha));

	static gfx::ImageSkia unsel, unsel_hv, selected, selected_dis;
	static bool init = false;
	if (!init)
	{
		init = true;
		unsel = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_DANMAKU_CHECK_NORMAL);
		unsel_hv = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_DANMAKU_CHECK_HOVER);
		selected = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_DANMAKU_CHECK_SELECT);
		selected_dis = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_DANMAKU_CHECK_DISABLE);
	}

	SetCustomImage(false, false, views::Button::STATE_NORMAL, unsel);
	SetCustomImage(false, true, views::Button::STATE_NORMAL, unsel);
	SetCustomImage(false, false, views::Button::STATE_HOVERED, unsel_hv);
	SetCustomImage(false, true, views::Button::STATE_HOVERED, unsel_hv);
	SetCustomImage(false, false, views::Button::STATE_PRESSED, unsel_hv);
	SetCustomImage(false, true, views::Button::STATE_PRESSED, unsel_hv);
	SetCustomImage(false, false, views::Button::STATE_DISABLED, unsel);
	SetCustomImage(false, true, views::Button::STATE_DISABLED, unsel);

	SetCustomImage(true, false, views::Button::STATE_NORMAL, selected);
	SetCustomImage(true, true, views::Button::STATE_NORMAL, selected);
	SetCustomImage(true, false, views::Button::STATE_HOVERED, selected);
	SetCustomImage(true, true, views::Button::STATE_HOVERED, selected);
	SetCustomImage(true, false, views::Button::STATE_PRESSED, selected);
	SetCustomImage(true, true, views::Button::STATE_PRESSED, selected);
	SetCustomImage(true, false, views::Button::STATE_DISABLED, selected_dis);
	SetCustomImage(true, true, views::Button::STATE_DISABLED, selected_dis);
}

DanmuSettingCustomCheckbox::DanmuSettingCustomCheckbox(
	const base::string16& label,
	const gfx::Font& font,
	gfx::ImageSkia* unsel,
	gfx::ImageSkia* unsel_dis,
	gfx::ImageSkia* selected,
	gfx::ImageSkia* selected_dis)
	: BililiveCheckbox(label)
{
	SetFont(font);
	SetTextColor(Button::STATE_NORMAL, clrContent);
	SetTextColor(Button::STATE_HOVERED, clrContent);
	SetTextColor(Button::STATE_PRESSED, clrContent);
	SetTextColor(Button::STATE_DISABLED, SkColorSetA(clrContent, disable_alpha));

	SetCustomImage(false, false, views::Button::STATE_NORMAL, *unsel);
	SetCustomImage(false, true, views::Button::STATE_NORMAL, *unsel);
	SetCustomImage(false, false, views::Button::STATE_HOVERED, *unsel);
	SetCustomImage(false, true, views::Button::STATE_HOVERED, *unsel);
	SetCustomImage(false, false, views::Button::STATE_PRESSED, *unsel);
	SetCustomImage(false, true, views::Button::STATE_PRESSED, *unsel);
	SetCustomImage(false, false, views::Button::STATE_DISABLED, *unsel_dis);
	SetCustomImage(false, true, views::Button::STATE_DISABLED, *unsel_dis);

	SetCustomImage(true, false, views::Button::STATE_NORMAL, *selected);
	SetCustomImage(true, true, views::Button::STATE_NORMAL, *selected);
	SetCustomImage(true, false, views::Button::STATE_HOVERED, *selected);
	SetCustomImage(true, true, views::Button::STATE_HOVERED, *selected);
	SetCustomImage(true, false, views::Button::STATE_PRESSED, *selected);
	SetCustomImage(true, true, views::Button::STATE_PRESSED, *selected);
	SetCustomImage(true, false, views::Button::STATE_DISABLED, *selected_dis);
	SetCustomImage(true, true, views::Button::STATE_DISABLED, *selected_dis);
}

DanmuSettingCombobox::DanmuSettingCombobox(bool editable/* = false*/, bool underline_style/* = false*/)
	: BililiveComboboxEx(editable, underline_style), underline_style_(underline_style)
{
	SetBackgroundColor(clrWindowsContent);
	SetBorderColor(clrContent, clrCtrlBorderHov, clrDroplistBorder);
	SetFont(ftTwelve);
	if (underline_style_)
	{
		ResourceBundle& rb = ResourceBundle::GetSharedInstance();
		gfx::ImageSkia* arrow_img_ = rb.GetImageSkiaNamed(IDR_LIVEHIME_TABAREA_STRIP_DROP_DOWN_TRIANGLE);
		SetArrowImage(arrow_img_, arrow_img_, arrow_img_);
		SetTextColor(GetColor(ThemeLight), GetColor(ThemeLight));
		SetItemTextColor(clrContent, GetColor(ThemeLight));
		StateChanged();
	}
	else
	{
		SetTextColor(clrContent, clrLivehime);
	}
}

gfx::Size DanmuSettingCombobox::GetPreferredSize()
{
	gfx::Size size(underline_style_ ? GetLengthByDPIScale(50) : GetLengthByDPIScale(80), GetLengthByDPIScale(20));// = __super::GetPreferredSize();
	//size.set_height(GetLengthByDPIScale(15));
	return size;
}

int DanmuSettingCombobox::GetHeightForWidth(int w)
{
	return GetPreferredSize().height();
}

void DanmuSettingCombobox::OnPaintBackground(gfx::Canvas* canvas)
{
	if (underline_style_)
	{
		canvas->DrawLine(gfx::Point(0, height() - 1), gfx::Point(width(), height() - 1), SkColorSetRGB(0x8D, 0x9A, 0xA4));
	} 
	else
	{
		__super::OnPaintBackground(canvas);
	}
}

gfx::NativeCursor DanmuSettingCombobox::GetCursor(const ui::MouseEvent& event)
{
	if (underline_style_)
	{
		return ::LoadCursor(nullptr, IDC_HAND);
	}
	return __super::GetCursor(event);
}


DanmuSettingRadioButton::DanmuSettingRadioButton(const string16& label, int group_id, views::ButtonListener* listener/* = nullptr*/)
	: BililiveRadioButton(label, group_id)
{
	set_listener(listener);
	set_focus_border(nullptr);
	SetFont(ftTwelve);
	SetTextColor(views::Button::STATE_NORMAL, clrContent);
	SetTextColor(views::Button::STATE_HOVERED, clrContent);
	SetTextColor(views::Button::STATE_PRESSED, clrContent);
	SetTextColor(views::Button::STATE_DISABLED, SkColorSetA(clrContent, disable_alpha));
	this->label()->SetHasFocusBorder(false);

	static gfx::ImageSkia unsel, unsel_hv, selected;
	static bool init = false;
	if (!init)
	{
		init = true;
		unsel = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_RADIO);
		unsel_hv = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_RADIO_HV);
		selected = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_RADIO_SELECT);
	}

	SetCustomImage(false, false, views::Button::STATE_NORMAL, unsel);
	SetCustomImage(false, true, views::Button::STATE_NORMAL, unsel);
	SetCustomImage(false, false, views::Button::STATE_HOVERED, unsel_hv);
	SetCustomImage(false, true, views::Button::STATE_HOVERED, unsel_hv);
	SetCustomImage(false, false, views::Button::STATE_PRESSED, unsel_hv);
	SetCustomImage(false, true, views::Button::STATE_PRESSED, unsel_hv);
	SetCustomImage(false, false, views::Button::STATE_DISABLED, unsel);
	SetCustomImage(false, true, views::Button::STATE_DISABLED, unsel);

	SetCustomImage(true, false, views::Button::STATE_NORMAL, selected);
	SetCustomImage(true, true, views::Button::STATE_NORMAL, selected);
	SetCustomImage(true, false, views::Button::STATE_HOVERED, selected);
	SetCustomImage(true, true, views::Button::STATE_HOVERED, selected);
	SetCustomImage(true, false, views::Button::STATE_PRESSED, selected);
	SetCustomImage(true, true, views::Button::STATE_PRESSED, selected);
	SetCustomImage(true, false, views::Button::STATE_DISABLED, selected);
	SetCustomImage(true, true, views::Button::STATE_DISABLED, selected);
}


SplitLineLabel::SplitLineLabel()
{

}

void SplitLineLabel::OnPaintBackground(gfx::Canvas* canvas)
{
	auto bound = GetContentsBounds();
	SkPaint paint;
	paint.setAntiAlias(true);
	paint.setColor(clrSplitLine);
	paint.setStyle(SkPaint::kFill_Style);
	canvas->DrawRoundRect(bound, 0, paint);
}

gfx::Size SplitLineLabel::GetPreferredSize()
{
	gfx::Size size = __super::GetPreferredSize();
	size.set_height(1);
	return size;
}




FixedFontSizeSlider::FixedFontSizeSlider(views::SliderListener* listener)
	:LivehimeSlider(new FixedFontSizeSliderListener(listener), views::Slider::HORIZONTAL)
{
	conv_infos_.push_back(ConvertInfo(1,10,L"较小"));
	conv_infos_.push_back(ConvertInfo(2, 12, L"推荐"));
	conv_infos_.push_back(ConvertInfo(3, 14, L"中等"));
	conv_infos_.push_back(ConvertInfo(4, 16, L"大"));
	conv_infos_.push_back(ConvertInfo(5, 18, L"较大"));
	conv_infos_.push_back(ConvertInfo(6, 20, L"特大"));
	min_ = conv_infos_.at(0).index_;
	max_ = conv_infos_.at(conv_infos_.size() - 1).index_;
	if (listener_)
	{
		FixedFontSizeSliderListener*pL = static_cast<FixedFontSizeSliderListener*>(listener_);
		if (pL)
		{
			pL->SetIntervalCount(max_ - min_);
		}
	}
	SetKeyboardIncrement(1.0f / (max_ - min_));
	SetMouseWheelIncrement(1.0f / (max_ - min_));
	SetBarDetails(2, clrSliderEmptyArea, clrSliderEmptyArea);
};

FixedFontSizeSlider::~FixedFontSizeSlider() {
	if (listener_) {
		delete static_cast<FixedFontSizeSliderListener*>(listener_);
	}
}

int FixedFontSizeSlider::GetConvertIndexAndTextByNumber(int number, base::string16& text)
{
	if (conv_infos_.size() <= 0)
	{
		return number;
	}
	//查找等于number的数和index,没有等于的，则返回最接近number比他小的数
	int index = conv_infos_.at(0).index_;
	int target_number = conv_infos_.at(0).number_;
	text = conv_infos_.at(0).text_;
	for (int i = 0; i < (int)conv_infos_.size(); i++)
	{
		if (number < conv_infos_.at(i).number_)
		{
			break;
		}
		index = conv_infos_.at(i).index_;
		target_number = conv_infos_.at(i).number_;
		text = conv_infos_.at(i).text_;
	}
	return index;
}

int FixedFontSizeSlider::GetConvertNumberAndTextByIndex(int& index, base::string16& text)
{
	if (conv_infos_.size() <= 0)
	{
		return index;
	}
	int target_number = conv_infos_.at(0).number_;
	text = conv_infos_.at(0).text_;
	for (int i = 0; i < (int)conv_infos_.size(); i++)
	{
		if (index == conv_infos_.at(i).index_)
		{
			target_number = conv_infos_.at(i).number_;
			text = conv_infos_.at(i).text_;
			break;
		}	
	}
	return target_number;
}

void FixedFontSizeSlider::SetNumber(int number) 
{
	base::string16 text;
	int index = GetConvertIndexAndTextByNumber(number, text);
	index = std::max(index, min_);
	index = std::min(index, max_);

	float val = 0;
	int interval_count = max_ - min_;
	val = static_cast<float>(index - min_) / interval_count;

	SetValue(val);

	if (GetAssociationLabel())
	{
		GetAssociationLabel()->SetText(text);
	}
}

int FixedFontSizeSlider::GetNumberAndUpdateLabelText()
{
	float pos = 0.f;
	float val = value();
	int interval_count = max_ - min_;

	int valid_point_count = interval_count * 2;
	int interval_index = 0;

	for (int i = 1; i < valid_point_count + 2; i += 2) {
		if (val < i / static_cast<float>(valid_point_count)) {
			pos = static_cast<float>(interval_index) / interval_count;
			break;
		}
		++interval_index;
	}
	int index = static_cast<int>(pos * interval_count) + min_;
	base::string16 text;
	int number = GetConvertNumberAndTextByIndex(index, text);
	if (GetAssociationLabel())
	{
		GetAssociationLabel()->SetText(text);
	}
	return number;
}

void FixedFontSizeSlider::OnPaint(gfx::Canvas* canvas)
{
	if (conv_infos_.size() > 1)
	{
		gfx::Rect content = GetContentsBoundsImp();
		int thumb_y = content.y() + std::round((content.height() - thumb_->height()) * 1.0f / 2);
		int w = content.width() - thumb_->width();
		int interval_count = max_ - min_;
		int h_w = GetLengthByDPIScale(2);
		int h_h = GetLengthByDPIScale(6);
		for (size_t i = 0;i < conv_infos_.size();i++)
		{
			float value = static_cast<float>(conv_infos_.at(i).index_ - min_) / interval_count;
			int full = value * w;	
			canvas->FillRect(gfx::Rect(
				content.x() + full + thumb_->width() / 2 - h_w / 2,
				thumb_y + thumb_y / 2 - h_h / 2,
				h_w,
				h_h), empty_color_);
		}
	}
	__super::OnPaint(canvas);
}


void FixedFontSizeSlider::FixedFontSizeSliderListener::SliderValueChanged(
	views::Slider* sender, float value, float old_value, views::SliderChangeReason reason)
{
	if (freeze_notify_) {
		return;
	}

	int interval_index = 0;
	int detect_section_count = interval_count_ * 2;

	float tmp = value;
	for (int i = 1; i < detect_section_count + 2; i += 2) {
		if (value < i / static_cast<float>(detect_section_count)) {
			tmp = static_cast<float>(interval_index) / interval_count_;
			break;
		}
		++interval_index;
	}

	if (reason == views::VALUE_CHANGED_BY_USER) {
		if (tmp != value) {
			freeze_notify_ = true;
			sender->SetValue(tmp);
			freeze_notify_ = false;
		}

		old_value = value;
		value = tmp;
	}

	if (prev_interval_index_ != interval_index) {
		prev_interval_index_ = interval_index;

		if (listener_) {
			listener_->SliderValueChanged(sender, value, old_value, reason);
		}
	}
}
