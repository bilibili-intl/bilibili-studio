#ifndef LIVEHIME_BUTTONS_H
#define LIVEHIME_BUTTONS_H


#include "bililive/bililive/ui/views/controls/bililive_labelbutton.h"
#include "bililive/bililive/ui/views/controls/bililive_radio_button.h"
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/controls/bililive_checkbox.h"

class LivehimeLabelButtonStyleRegister
{
public:
    LivehimeLabelButtonStyleRegister();
    ~LivehimeLabelButtonStyleRegister(){}
};
extern LivehimeLabelButtonStyleRegister livehime_button_reg_obj;

enum LivehimeButtonStyle
{
    // ������ť>ȷ�����ر�
    LivehimeButtonStyle_ActionButtonPositive = BililiveLabelButton::STYLE_LIVEHIME,
    LivehimeButtonStyle_ActionButtonNegative,
    // ���ܰ�ť>�򿪡����
    LivehimeButtonStyle_FunctionButton,

    // ����ȡ��ƥ�䰴ť
    LivehimeButtonStyle_ColiveButtonCancelMatching,
    // �������밴ť
    LivehimeButtonStyle_ColiveButtonInvite,

    // 3.0
    LivehimeButtonStyle_V3_RecordButton, // ¼�ư�ť
    LivehimeButtonStyle_V3_TitleButton, // ������ʹ�õİ�ť

    //�°�UI->��������������
    LivehimeButtonStyle_CoverButton,

    //�°�UIֱ����ť
    LivehimeButtonStyle_V4_CoverButton,
    LivehimeButtonStyle_V4_LiveButton,
    LivehimeButtonStyle_V4_RecordButton, // ¼�ư�ť

    //ͨ�öԻ���ť
    LivehimeGeneralMsgBox_Button_Positive,
    LivehimeGeneralMsgBox_Button_Negative,

};

class LivehimeLabelButton : public BililiveLabelButton
{
protected:
    LivehimeLabelButton(views::ButtonListener* listener, const string16& text, /*LivehimeButtonStyle*/int lbs);
};

class LivehimeActionLabelButton : public LivehimeLabelButton
{
public:
    static const char kViewClassName[];
    LivehimeActionLabelButton(views::ButtonListener* listener, const string16& text, bool positive)
        : LivehimeLabelButton(listener, text, positive ? LivehimeButtonStyle_ActionButtonPositive : LivehimeButtonStyle_ActionButtonNegative)
    {}

    static gfx::Size GetButtonSize();

protected:
    virtual const char* GetClassName() const OVERRIDE{ return kViewClassName; }
};

class LivehimeGeneralMsgBoxButton : public LivehimeLabelButton
{
public:
    static const char kViewClassName[];
    LivehimeGeneralMsgBoxButton(views::ButtonListener* listener, const string16& text, bool positive)
        : LivehimeLabelButton(listener, text, positive ? LivehimeGeneralMsgBox_Button_Positive : LivehimeGeneralMsgBox_Button_Negative)
    {
        SetRadius(2);
    }

    static gfx::Size GetButtonSize();

protected:
    virtual const char* GetClassName() const OVERRIDE { return kViewClassName; }
};

class LivehimeFunctionLabelButton : public LivehimeLabelButton
{
public:
    static const char kViewClassName[];
    LivehimeFunctionLabelButton(views::ButtonListener* listener, const string16& text)
        : LivehimeLabelButton(listener, text, LivehimeButtonStyle_FunctionButton)
    {}

    static gfx::Size GetButtonSize();

protected:
    virtual const char* GetClassName() const OVERRIDE{ return kViewClassName; }
};

// ���Ұ�ť��ѡ�еķ���֮���
class LivehimeCapsuleButton : public views::CustomButton
{
public:
    LivehimeCapsuleButton(views::ButtonListener* listener, const string16& text);

    // �������������ȡ���ĸ߶Ⱥ�ʵ�ʸ߶Ȳ�һ�£�������أ������һ�µ�
    static gfx::Size GetButtonSize();

    virtual void SetChecked(bool checked);
    bool checked() const{ return is_checked_; }
    void SetText(const string16& text);
    const string16& GetText() const{ return label_->text(); }

protected:
    // view
    void StateChanged() override;
    void OnPaintBackground(gfx::Canvas* canvas) override;
    gfx::Size GetPreferredSize() override;
    int GetHeightForWidth(int w) override;
    void Layout() override;
    const char*  GetClassName() const override { return "LivehimeCapsuleButton"; }

    void SetColorDetails(const BililiveLabelButton::ColorStruct& clrs);

    BililiveLabel *label() { return label_; }

private:
    bool is_checked_;
    BililiveLabel* label_;
    gfx::Font font_;

    BililiveLabelButton::ColorStruct clrs_details_;

    DISALLOW_COPY_AND_ASSIGN(LivehimeCapsuleButton);
};


// ֻ����û�б���û�б߿�İ�ť��Ч�����Ƴ����Ӱ�ť
class LivehimeLinkButton : public BililiveLabelButton
{
public:
    LivehimeLinkButton(views::ButtonListener* listener, const string16& text);
};

class LivehimeSmallTitleLinkButton : public BililiveLabelButton
{
public:
    LivehimeSmallTitleLinkButton(views::ButtonListener* listener, const string16& text);

protected:
    // View
    virtual gfx::NativeCursor GetCursor(const ui::MouseEvent& event) OVERRIDE;
};

// ��ѡ��
class LivehimeCheckbox : public BililiveCheckbox
{
public:
    explicit LivehimeCheckbox(const base::string16& label);
    explicit LivehimeCheckbox(UIViewsStyle style,const base::string16& label);
private:
    void InitStyle();
};

// ��ѡ��
class LivehimeRadioButton : public BililiveRadioButton
{
public:
    LivehimeRadioButton(const string16& label, int group_id, views::ButtonListener* listener = nullptr);

    void SetAllStateTextColor(SkColor clr);
};


// ͼƬ���ִ�ֱ����
// ��չ��������Ƿ��ڰ�ť�ھ�����ʾ;
class LivehimeVerticalLabelButton :
    public views::CustomButton,
    public livehime::SupportsEventReportV2
{
public:
    LivehimeVerticalLabelButton(views::ButtonListener* listener, const string16& text);
    virtual ~LivehimeVerticalLabelButton() = default;

    // Set the image the button should use for the provided state.
    virtual void SetImage(views::Button::ButtonState state, const gfx::ImageSkia* image);
    virtual void SetColor(views::Button::ButtonState state, const SkColor clr);

    // Returns the image for a given |state|.
    virtual const gfx::ImageSkia& GetImage(views::Button::ButtonState state) const;

    views::ImageView* image_view() { return image_;  }
    BililiveLabel* label_view() { return label_; }

    void SetFont(const gfx::Font &font);
    void SetText(const base::string16 &text);

    void SetIsCenter(bool is_center) { is_center_ = is_center; }
	// View
	gfx::Size GetPreferredSize() override;
protected:

    void Layout() override;

    // Button
    void NotifyClick(const ui::Event& event) override;

    // CustomButton
    void StateChanged() override;

private:
    // The image and label shown in the button.
    views::ImageView* image_;
    BililiveLabel* label_;
    gfx::ImageSkia images_[STATE_COUNT];
    SkColor colors_[STATE_COUNT];
    bool is_center_;
};

// ����ͼƬ�Ұ�ť
class LivehimeImageRightButton :
	public views::CustomButton
{
public:
    LivehimeImageRightButton(views::ButtonListener* listener, const string16& text);
	virtual ~LivehimeImageRightButton() = default;

	// Set the image the button should use for the provided state.
	virtual void SetImage(views::Button::ButtonState state, const gfx::ImageSkia* image);
	virtual void SetColor(views::Button::ButtonState state, const SkColor clr);

	// Returns the image for a given |state|.
	virtual const gfx::ImageSkia& GetImage(views::Button::ButtonState state) const;

	views::ImageView* image_view() { return image_; }
	BililiveLabel* label_view() { return label_; }

	void SetFont(const gfx::Font& font);
	void SetText(const base::string16& text);
    void SetCursor(gfx::NativeCursor cursor) { cursor_ = cursor; }

public:
	// View
	gfx::Size GetPreferredSize() override;
protected:
    // View
	void Layout() override;
    gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override;
	// CustomButton
	void StateChanged() override;

private:
	// The image and label shown in the button.
    int space_ = 1;
	views::ImageView* image_;
	BililiveLabel* label_;
    gfx::NativeCursor cursor_ = nullptr;
	gfx::ImageSkia images_[STATE_COUNT];
	SkColor colors_[STATE_COUNT];
};

// ��ɫѡ�С����Ͻ�check
class LivehimeColorRadioButton : public BililiveRadioButton
{
public:
    LivehimeColorRadioButton(int group_id, SkColor color);
    virtual ~LivehimeColorRadioButton();

    void SetCheckedImageSkia(gfx::ImageSkia *image);
    void SetColor(SkColor color);

    // Checkbox
    virtual void SetChecked(bool checked);

protected:
    // View
    virtual gfx::Size GetPreferredSize() OVERRIDE;
    virtual int GetHeightForWidth(int w) OVERRIDE;
    virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;

    // LabelButton
    virtual const gfx::ImageSkia& GetImage(ButtonState for_state) OVERRIDE;

    // Overridden from CustomButton:
    //virtual void StateChanged() OVERRIDE;

private:
    SkColor color_;
    gfx::ImageSkia check_image_;
    gfx::ImageSkia null_image_;
};

// �˾�ѡ�С����Ͻ�check
class LivehimeFilterRadioButton : public BililiveRadioButton
{
public:
    explicit LivehimeFilterRadioButton(int group_id);
    virtual ~LivehimeFilterRadioButton();

    void SetCheckedImageSkia(gfx::ImageSkia *image);
    void SetImageAndText(gfx::ImageSkia *image, const base::string16 &text);
    base::string16 GetText() { return text_; }
    void SetValue(int value) { value_ = value; }
    int GetValue() { return value_; }
    gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override;
    // Checkbox
    virtual void SetChecked(bool checked);

    // Overridden from Button:
    virtual void NotifyClick(const ui::Event& event) OVERRIDE;
    // View
    virtual gfx::Size GetPreferredSize() OVERRIDE;
protected:
    // View
    virtual int GetHeightForWidth(int w) OVERRIDE;
    virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;
    virtual void OnFocus() OVERRIDE;

    // LabelButton
    virtual const gfx::ImageSkia& GetImage(ButtonState for_state) OVERRIDE;

    // Overridden from CustomButton:
    //virtual void StateChanged() OVERRIDE;

private:
    gfx::ImageSkia check_image_;
    gfx::ImageSkia null_image_;
    gfx::ImageSkia main_image_;
    base::string16 text_;
    int value_ = 100;
};

// ������ȡ��ƥ�䰴ť
class LivehimeColiveLabelButton : public LivehimeLabelButton {
public:
    static const char kViewClassName[];
    LivehimeColiveLabelButton(views::ButtonListener* listener, const string16& text)
        : LivehimeLabelButton(listener, text, LivehimeButtonStyle_ColiveButtonCancelMatching) {}

    static gfx::Size GetButtonSize();

protected:
    virtual const char* GetClassName() const override { return kViewClassName; }
};

// ���������밴ť
class LivehimeColiveInviteButton : public LivehimeLabelButton
{
public:
    static const char kViewClassName[];

    LivehimeColiveInviteButton(views::ButtonListener* listener, const string16& text)
        : LivehimeLabelButton(listener, text, LivehimeButtonStyle_ColiveButtonInvite)
    {
    }

    static gfx::Size GetButtonSize();

protected:
    // View
    virtual const char* GetClassName() const override
    {
        return kViewClassName;
    }

private:
};


// 3.0������ʹ�õİ�ť
class LivehimeTitlebarButton : public BililiveLabelButton
{
public:
    LivehimeTitlebarButton(views::ButtonListener* listener, const string16& text);
};

// 3.0����������ʹ�õİ�ť
class LivehimeRoomInfoAreaButton : public BililiveLabelButton
{
public:
    LivehimeRoomInfoAreaButton(views::ButtonListener* listener, const string16& text);
};

//�л�ģʽ��ť
class LivehimeSwitchModeButton : public BililiveLabelButton
{
public:
    LivehimeSwitchModeButton(views::ButtonListener* listener, const string16& text);
};

//ѡ�������ť,ע��:��������������ҳ���õ��ˣ��޸�ʱע��ͬ������
class LivehimeCoverModeButton :public LivehimeLabelButton
{
public:
	enum CoverModeButtonType
	{
		BtType_SelectPartition,
		BtType_TalkSubject
	};
    LivehimeCoverModeButton(views::ButtonListener* listener, CoverModeButtonType bt_type);
};

//����زİ�ť,ע��:������������ҳ���õ��ˣ��޸�ʱע��ͬ������
class MaterialControlButton : public BililiveLabelButton
{
public:
    enum MaterialButtonType
    {
        BtType_Add,
        BtType_Clear,
        BtType_Plugin
    };
    MaterialControlButton(views::ButtonListener* listener, MaterialButtonType bt_type);
private:
	gfx::ImageSkia* material_image_normal_;
	gfx::ImageSkia* material_image_hover_;
	gfx::ImageSkia* material_image_press_;
};


#endif
