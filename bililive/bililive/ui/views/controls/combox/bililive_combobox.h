
#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_COMBOX_BILILIVE_COMBOBOX_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_COMBOX_BILILIVE_COMBOBOX_H

#include "base/basictypes.h"
#include "base/memory/scoped_vector.h"

#include "ui/views/controls/combobox/combobox.h"
#include "ui/base/models/combobox_model.h"
#include "ui/views/controls/combobox/combobox_listener.h"
#include "ui/views/controls/combobox/native_combobox_views.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/controls/textfield/textfield_controller.h"
#include "ui/views/controls/textfield/native_textfield_views.h"

class BililiveComboboxDataOperator
{
protected:
	virtual ~BililiveComboboxDataOperator() {}
public:
	virtual int64 Alloc() = 0;
	virtual void Free(const int64& val) = 0;
	virtual void Assign(int64& lhs, const int64& rhs) = 0;
	virtual bool Compare(const int64& lhs, const int64& rhs) = 0;
	virtual void DeleteThis() = 0;
};

BililiveComboboxDataOperator* BililiveCreateInt64Operator(); //int64
BililiveComboboxDataOperator* BililiveCreateStringOperator(); //std::string
BililiveComboboxDataOperator* BililiveCreateWStringOperator(); //std::wstring


// 组合框数据源
class BililiveComboboxModel : public ui::ComboboxModel
{
	// 组合框数据项
	struct _COMBOBOX_ITEM
	{
		_COMBOBOX_ITEM(const base::string16& str, int64 tag, BililiveComboboxDataOperator* op);

		~_COMBOBOX_ITEM();

		base::string16 text;
		int64 data;
		BililiveComboboxDataOperator* dataOp;
	};

public:
	BililiveComboboxModel(BililiveComboboxDataOperator* dataOp);
	virtual ~BililiveComboboxModel();

	void SetDataOperator(BililiveComboboxDataOperator* dataOp);

	int AddItem(const base::string16& str, int64 data = 0);
	void ClearItems();
	int FindItem(const base::string16& str, int start = 0);
	int FindItemData(int64 data, int start = 0);
	bool SetItemData(int index, int64 data);
	int64 GetItemData(int index);
	base::string16 GetItemText(int index);
	bool SetItemText(int index, const base::string16& str);
	void SetDefaultIndex(int index) { default_index_ = index; }

	// ComboboxModel
	int GetItemCount() const override { return item_vector_.size(); }
    base::string16 GetItemAt(int index) override
	{
		if (index >= 0 && index < static_cast<int>(item_vector_.size()))
		{
			return item_vector_[index]->text;
		}
		return L"";
	}
	virtual int GetDefaultIndex() const { return default_index_; }

private:
	int default_index_;
	BililiveComboboxDataOperator* dataOp_;
	ScopedVector<_COMBOBOX_ITEM> item_vector_;

	DISALLOW_COPY_AND_ASSIGN(BililiveComboboxModel);
};

class BililiveCombobox;
// 实际组合框
class BililiveNativeComboboxViews : public views::NativeComboboxViews
{
    // 对combobox进行回调的接口
    class BililiveComboboxTextfieldDelegate : public views::TextfieldController
    {
    public:
        virtual void OnTextfieldBlur() = 0;
        virtual void OnTextfieldKeyPressed(const ui::KeyEvent& e) = 0;
    };

    // 组合框专用编辑框控件
    class BililiveComboboxTextfield : public views::Textfield
    {
    public:
        BililiveComboboxTextfield(BililiveNativeComboboxViews* combox)
            : combobox_(combox)
            , delegate_(NULL){}
        ~BililiveComboboxTextfield(){}

        void SetBililiveComboboxTextfieldDelegate(BililiveComboboxTextfieldDelegate* delegate);

    protected:
        // Textfield
        void OnBlur() override;
        bool OnKeyPressed(const ui::KeyEvent& e) override;

    private:
        BililiveNativeComboboxViews* combobox_;
        BililiveComboboxTextfieldDelegate* delegate_;

        DISALLOW_COPY_AND_ASSIGN(BililiveComboboxTextfield);
    };

public:
    explicit BililiveNativeComboboxViews(BililiveCombobox* combobox);
	virtual ~BililiveNativeComboboxViews(){}

    void SetTextColor(SkColor clr) { text_color_ = clr; }
    void SetBorderColor(SkColor clrBlur, SkColor clrFocus);
    void SetEditAble(bool able);
    void SetTextInputType(ui::TextInputType type);

protected:
    // View
    virtual void OnPaint(gfx::Canvas* canvas) override;
    virtual void OnPaintBorder(gfx::Canvas* canvas) override;
    virtual bool OnMousePressed(const ui::MouseEvent& mouse_event) override;
    virtual void OnMouseEntered(const ui::MouseEvent& event) override;
    virtual void OnMouseExited(const ui::MouseEvent& event) override;

	void PaintText(gfx::Canvas* canvas);
	void AdjustBoundsForRTLUI(gfx::Rect* rect) const;

private:
    void createTextfield();

private:
    friend class BililiveCombobox;
	gfx::ImageSkia* down_arrow_image_;
	gfx::ImageSkia* up_arrow_image_;
    bool is_mouse_enter_;
    SkColor border_color_;
    SkColor border_focus_color_;
    SkColor text_color_;
    BililiveComboboxTextfield* textfield_;
    bool edit_able_;

	DISALLOW_COPY_AND_ASSIGN(BililiveNativeComboboxViews);
};


// BililiveCombobox
class BililiveCombobox :
    public views::Combobox,
    public BililiveNativeComboboxViews::BililiveComboboxTextfieldDelegate
{
public:
	explicit BililiveCombobox(BililiveComboboxDataOperator* dataOp = BililiveCreateInt64Operator());
	virtual ~BililiveCombobox();

    void SetPreferredSize(const gfx::Size &size){ pref_size_ = size; }
    void Enlarge(int width, int height){ enlarge_size_ = gfx::Size(width, height); }

	int AddItem(const base::string16& str, int64 data = 0);
	int FindItem(const base::string16& str, int start = 0);
	int FindItemData(int64 data, int start = 0);
	int GetItemCount();
	void ClearItems();
	bool SetItemData(int index, int64 data);
	int64 GetItemData(int index);
	bool SetItemText(int index, const base::string16& str);
	base::string16 GetItemText(int index);
	void SetDefaultIndex(int index);

    void SetBorderColor(SkColor clrBlur, SkColor clrFocus);
    void SetTextColor(SkColor clr);
    void SetEditAble(bool able);
    ui::TextInputType GetTextInputType() const;
    void SetTextInputType(ui::TextInputType type);

protected:
	// View
	void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;
    gfx::Size GetPreferredSize() override;
    virtual bool HitTestRect(const gfx::Rect& rect) const override;

    // BililiveComboboxTextfieldDelegate
    void OnTextfieldBlur() override {}
    void OnTextfieldKeyPressed(const ui::KeyEvent& e) override { }

protected:
    views::Textfield* textfield() const;
	BililiveComboboxDataOperator* dataOp_;

private:
    bool edit_able_;
    ui::TextInputType text_input_type_;
    SkColor border_color_;
    SkColor border_focus_color_;
    SkColor text_color_;
    gfx::Size pref_size_;
    gfx::Size enlarge_size_;

	DISALLOW_COPY_AND_ASSIGN(BililiveCombobox);
};


#endif  // BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_COMBOX_BILILIVE_COMBOBOX_H
