#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_SETTINGS_WIDGET_COMMON_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_SETTINGS_WIDGET_COMMON_VIEW_H_
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/controls/bililive_checkbox.h"
#include "bililive/bililive/ui/views/controls/combox/bililive_combobox_ex.h"
#include "bililive/bililive/ui/views/controls/bililive_radio_button.h"
#include "bililive/bililive/ui\views/livehime/controls/livehime_slider.h"

extern const int kLeftPaddingColWidth;
extern const int kLineSpaceHeightBig;
extern const int kLineSpaceHeightSmall;
extern const int kSecondLevelIndentation;
extern const int kSettingBtWidth;
extern const int kSettingBtHeight;
extern const int kSettingSliderWidth;

class DanmuSettingCategoryLabel : public BililiveLabel
{
public:
	explicit DanmuSettingCategoryLabel(const string16& text);
};

class DanmuSettingTitleLabel : public BililiveLabel
{
public:
	explicit DanmuSettingTitleLabel(const string16& text);
};

class DanmuSettingContentLabel : public BililiveLabel
{
public:
	explicit DanmuSettingContentLabel(const string16& text);
};

class DanmuSettingNotesLabel : public BililiveLabel
{
public:
	explicit DanmuSettingNotesLabel(const string16& text);
};

class DanmuSettingCheckbox : public BililiveCheckbox
{
public:
	explicit DanmuSettingCheckbox(const base::string16& label,bool disable = true);
};

class DanmuVocieBroadcastCheckbox : public BililiveCheckbox
{
public:
	explicit DanmuVocieBroadcastCheckbox(const base::string16& label);
};

class DanmuSettingCustomCheckbox : public BililiveCheckbox
{
public:
	explicit DanmuSettingCustomCheckbox(const base::string16& label,
		const gfx::Font& font,
		gfx::ImageSkia* unsel,
		gfx::ImageSkia* unsel_dis,
		gfx::ImageSkia* selected,
		gfx::ImageSkia* selected_dis);
};

class DanmuSettingCombobox : public BililiveComboboxEx
{
public:
	explicit DanmuSettingCombobox(bool editable = false, bool underline_style = false);
protected:
	// View
	gfx::Size GetPreferredSize() override;
	int GetHeightForWidth(int w) override;
	void OnPaintBackground(gfx::Canvas* canvas) override;
	gfx::NativeCursor GetCursor(const ui::MouseEvent& event)override;
private:
	bool underline_style_ = false;
};

// 单选框
class DanmuSettingRadioButton : public BililiveRadioButton
{
public:
	DanmuSettingRadioButton(const string16& label, int group_id, views::ButtonListener* listener = nullptr);
};

class SplitLineLabel : public views::Label
{
public:
	SplitLineLabel();
protected:
	// view
	void OnPaintBackground(gfx::Canvas* canvas) override;
	gfx::Size GetPreferredSize() override;
};

//原13~20的字号缩到6档：10、12、14、16、18、20，分别对应程度：较小、小、中等、大、较大、大；
class FixedFontSizeSlider : public LivehimeSlider
{
public:
	explicit FixedFontSizeSlider(views::SliderListener* listener);
	~FixedFontSizeSlider();

	void SetNumber(int number);
	int GetNumberAndUpdateLabelText();

	virtual void OnPaint(gfx::Canvas* canvas) override;

private:
	struct ConvertInfo
	{
		ConvertInfo(int index, int number, const base::string16 &text)
		{
			index_ = index;
			number_ = number;
			text_ = text;
		}
		int index_;
		int number_;
		base::string16 text_;
	};
	class FixedFontSizeSliderListener : public views::SliderListener
	{
	public:
		explicit FixedFontSizeSliderListener(views::SliderListener* listener)
			: listener_(listener),
			interval_count_(5),
			prev_interval_index_(0),
			freeze_notify_(false) {
			DCHECK(interval_count_ >= 1);
		}
		void SetIntervalCount(int interval_count) {
			interval_count_ = interval_count;
		};
	protected:
		// SliderListener
		void SliderValueChanged(views::Slider* sender,
			float value,
			float old_value,
			views::SliderChangeReason reason) override;

	private:
		int interval_count_;
		int prev_interval_index_;
		bool freeze_notify_;
		views::SliderListener* listener_;
	};
private:
	int GetConvertIndexAndTextByNumber(int number, base::string16& text);
	int GetConvertNumberAndTextByIndex(int &index, base::string16 &text);
private:
	int min_ = 1;
	int max_ = 7;
	std::vector<ConvertInfo> conv_infos_;
	DISALLOW_COPY_AND_ASSIGN(FixedFontSizeSlider);
};

#endif

