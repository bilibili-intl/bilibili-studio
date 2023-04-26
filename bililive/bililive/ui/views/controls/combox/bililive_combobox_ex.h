
#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_COMBOX_BILILIVE_COMBOBOX_EX_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_COMBOX_BILILIVE_COMBOBOX_EX_H

#include "base/basictypes.h"
#include "base/memory/scoped_vector.h"

#include "bililive/bililive/livehime/help_center/help_center.h"
#include "bililive/bililive/ui/views/controls/bililive_floating_scroll_view.h"
#include "bililive/bililive/ui/views/controls/bilibili_native_edit_view.h"

#include "ui/base/models/combobox_model.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/controls/combobox/combobox.h"
#include "ui/views/controls/combobox/combobox_listener.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/controls/textfield/textfield_controller.h"
#include "ui/views/controls/textfield/native_textfield_views.h"
#include "ui/views/controls/button/custom_button.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/image_view.h"


namespace
{
    class BililiveComboboxDropdownView;
    class BililiveComboboxDroplistItemView;
    class BililiveComboboxDropdownWidget;
    class EditLabelArea;
}

class BililiveComboboxEx;
class BililiveComboboxExListener
{
public:
    virtual ~BililiveComboboxExListener() = default;

    virtual void OnSelectedIndexChanged(BililiveComboboxEx* combobox) = 0;
    virtual void OnBililiveComboboxPressed(BililiveComboboxEx* combobox) {}
    virtual void OnBililiveComboboxExDropDown(BililiveComboboxEx* combobox) {}
    virtual void OnBililiveComboboxExDropDownClosed(BililiveComboboxEx* combobox) {}
    virtual void OnBililiveComboboxExEditContentsChanged(BililiveComboboxEx* combobox, const base::string16& new_contents){}
    // reason = BililiveComboboxEx::EditBlurReason
    virtual void OnBililiveComboboxExEditBlur(BililiveComboboxEx* combobox, const base::string16 &text, int reason) {}
    virtual base::string16 OnBililiveComboboxExEditShow(BililiveComboboxEx* combobox, const base::string16 &label_text) { return label_text; }
};


class BililiveComboboxEx
    : public views::CustomButton
    , public BilibiliNativeEditController
    , public livehime::HelpSupport
{
    // 组合框数据项
    struct _COMBOBOX_ITEM
    {
        explicit _COMBOBOX_ITEM(const base::string16& str)
            : text(str)
            , valid(true)
        {
            id = ::InterlockedIncrement(&inc);
        }

        virtual ~_COMBOBOX_ITEM() = default;

        template<typename T>
        void set_tag(const T &tag)
        {
            DLOG(ERROR) << "invalid data type of BililiveComboboxEx::_COMBOBOX_ITEM!";
            DCHECK(false);
        };
        template<>
        void set_tag(const char &tag) { data_struct.numeric_union.char_ = tag; }
        template<>
        void set_tag(const unsigned char &tag) { data_struct.numeric_union.char_ = tag; }
        template<>
        void set_tag(const bool &tag) { data_struct.numeric_union.bool_ = tag; }
        template<>
        void set_tag(const short &tag) { data_struct.numeric_union.short_ = tag; }
        template<>
        void set_tag(const unsigned short &tag) { data_struct.numeric_union.short_ = tag; }
        template<>
        void set_tag(const int &tag) { data_struct.numeric_union.int_ = tag; }
        template<>
        void set_tag(const unsigned int &tag) { data_struct.numeric_union.int_ = tag; }
        template<>
        void set_tag(const long &tag) { data_struct.numeric_union.long_ = tag; }
        template<>
        void set_tag(const unsigned long &tag) { data_struct.numeric_union.long_ = tag; }
        template<>
        void set_tag(const long long &tag) { data_struct.numeric_union.int64_ = tag; }
        template<>
        void set_tag(const unsigned long long &tag) { data_struct.numeric_union.int64_ = tag; }
        template<>
        void set_tag(const float &tag) { data_struct.numeric_union.float_ = tag; }
        template<>
        void set_tag(const double &tag) { data_struct.numeric_union.double_ = tag; }
        template<>
        void set_tag(const std::string &tag) { data_struct.str_ = tag; }
        template<>
        void set_tag(const std::wstring &tag) { data_struct.wstr_ = tag; }

        template<typename T>
        bool get_tag(T &tag)
        {
            DLOG(ERROR) << "invalid data type of BililiveComboboxEx::_COMBOBOX_ITEM!";
            DCHECK(false);
            return false;
        };
        template<>
        bool get_tag(char &tag) { return tag = data_struct.numeric_union.char_, true; }
        template<>
        bool get_tag(unsigned char &tag) { return tag = data_struct.numeric_union.char_, true; }
        template<>
        bool get_tag(bool &tag) { return tag = data_struct.numeric_union.bool_, true; }
        template<>
        bool get_tag(short &tag) { return tag = data_struct.numeric_union.short_, true; }
        template<>
        bool get_tag(unsigned short &tag) { return tag = data_struct.numeric_union.short_, true; }
        template<>
        bool get_tag(int &tag) { return tag = data_struct.numeric_union.int_, true; }
        template<>
        bool get_tag(unsigned int &tag) { return tag = data_struct.numeric_union.int_, true; }
        template<>
        bool get_tag(long &tag) { return tag = data_struct.numeric_union.long_, true; }
        template<>
        bool get_tag(unsigned long &tag) { return tag = data_struct.numeric_union.long_, true; }
        template<>
        bool get_tag(long long &tag) { return tag = data_struct.numeric_union.int64_, true; }
        template<>
        bool get_tag(unsigned long long &tag) { return tag = data_struct.numeric_union.int64_, true; }
        template<>
        bool get_tag(float &tag) { return tag = data_struct.numeric_union.float_, true; }
        template<>
        bool get_tag(double &tag) { return tag = data_struct.numeric_union.double_, true; }
        template<>
        bool get_tag(std::string &tag) { return tag = data_struct.str_, true; }
        template<>
        bool get_tag(std::wstring &tag) { return tag = data_struct.wstr_, true; }

        long id;
        base::string16 text;
        bool valid;
        struct _DS
        {
            _DS()
            {
                numeric_union.int64_ = 0;
            };

            union
            {
                char char_;
                bool bool_;
                short short_;
                int int_;
                long long_;
                long long int64_;
                float float_;
                double double_;
            }numeric_union;
            std::string str_;
            std::wstring wstr_;
        }data_struct;

    private:
        static volatile long inc;
    };

public:
    enum AnchorPosition
    {
        TOPLEFT,
        TOPRIGHT,
        TOPCENTER,
        BOTTOMLEFT,
        BOTTOMRIGHT,
        BOTTOMCENTER,
    };

    enum EditBlurReason
    {
        EDIT_BLUR_REASON_DEFAULT,
        EDIT_BLUR_REASON_RETURN,
        EDIT_BLUR_REASON_SPECIA_KEY,
        EDIT_BLUR_REASON_SELECT_ITEM,
    };

    enum EditInputType
    {
        EDIT_INPUT_TYPE_DEFAULT,
        EDIT_INPUT_TYPE_NUMBER = 1,
        EDIT_INPUT_TYPE_OBSCURED = 1 << 1,
    };

public:
    explicit BililiveComboboxEx(bool editable = false, int underline_style = 0);
    virtual ~BililiveComboboxEx();

    base::string16 GetText() const;
    bool IsEditFocus();

    void DoDropDown();

    void SetAnchorPosition(AnchorPosition anchor_position){ anchor_position_ = anchor_position; }

    void set_listener(BililiveComboboxExListener* listener){ listener_ = listener; }
    int FindItem(const base::string16& str, int start = 0);
    int DeleteItem(int index);
    int GetItemCount();
    void ClearItems();
    bool EnabledItem(int index, bool enable);
    bool SetItemText(int index, const base::string16& str);
    base::string16 GetItemText(int index);
    int selected_index() const { return current_index_; }
    void SetSelectedIndex(int index);

    void SetArrowImage(gfx::ImageSkia *down_image, gfx::ImageSkia *down_hv_image, gfx::ImageSkia *up_image);
    void SetBackgroundColor(SkColor color);
    void SetBorderColor(SkColor clrBlur, SkColor clrFocus, SkColor clrDroplist);
    void SetTextColor(SkColor clrNor, SkColor clrHov);
    void SetItemTextColor(SkColor clrNor, SkColor clrHov);
    void SetFont(const gfx::Font &font);
    gfx::Font font() const { return font_; }
    void set_default_width_in_chars(int default_width, const base::string16 &padding_char);
    void SetDroplistEqualWidth(bool equal);
    bool droplist_equal_width() const { return equal_width_; }
    void SetFixedWidth(int width) { fixed_width_ = width; };

    bool IsDropDown() { return is_dropdown_; }

    // edit
    void SetEditInputType(EditInputType eit);
    void SetEditLimitText(unsigned int nMax);

    // template
    template<typename T>
    void SetDefaultItem(const base::string16& str, const T &data)
    {
        default_item_.text = str;
        default_item_.set_tag(data);
        if (current_index_ == -1)
        {
            label_->SetText(str);
            Relayout();
        }
    };

    template<typename T>
    int AddItem(const base::string16& str, const T &data)
    {
        _COMBOBOX_ITEM *item = new _COMBOBOX_ITEM(str);
        item->set_tag(data);
        items_.push_back(item);
        return items_.size() - 1;
    };
    int AddItem(const base::string16& str) { return AddItem<int64>(str, 0); }

    template<typename T>
    int FindItemData(const T &data, int start = 0)
    {
        T t_tag;
        for (size_t i = start; i < items_.size(); i++)
        {
            if (items_[i]->get_tag(t_tag))
            {
                if (data == t_tag)
                {
                    return i;
                }
            }
        }
        return -1;
    };

    template<typename T>
    bool SetItemData(int index, const T &data)
    {
        DCHECK(index >= 0 && index < (int)items_.size());
        if (index >= 0 && index < (int)items_.size())
        {
            items_[index]->set_tag(data);
            return true;
        }
        return false;
    };

    template<typename T>
    T GetItemData(int index)
    {
        DCHECK(index >= -1);
        T data;
        if (index >= 0 && index < (int)items_.size())
        {
            items_[index]->get_tag<T>(data);
        }
        else if (index == -1)
        {
            default_item_.get_tag<T>(data);
        }
        return data;
    };

protected:
    // View
    virtual void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;
    virtual bool HitTestRect(const gfx::Rect& rect) const override;
    virtual gfx::Size GetPreferredSize() override;
    virtual int GetHeightForWidth(int w) override;
    virtual void OnPaintBackground(gfx::Canvas* canvas) override;
    virtual void VisibilityChanged(views::View* starting_from, bool is_visible) override;
    virtual void OnBoundsChanged(const gfx::Rect& previous_bounds) override;
    virtual bool NeedsNotificationWhenVisibleBoundsChange() const override{ return true; }
    virtual void OnVisibleBoundsChanged() override;
    virtual bool OnKeyPressed(const ui::KeyEvent& event) override;
    virtual void OnEnabledChanged() override;
    virtual bool OnMousePressed(const ui::MouseEvent& event) override;
    virtual void OnMouseReleased(const ui::MouseEvent& event) override;
    virtual gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override;
    virtual void OnBlur() override;
    virtual bool SkipDefaultKeyEventProcessing(const ui::KeyEvent& event) override;

    // CustomButton
    virtual void StateChanged() override;

    // BilibiliNativeEditController
    virtual void ContentsChanged(BilibiliNativeEditView* sender, const string16& new_contents) override;
    virtual bool PreHandleMSG(BilibiliNativeEditView* sender, UINT msg, WPARAM wParam, LPARAM lParam) override;

private:
    void Relayout();
    void CloseDropDown();
    void OnDropdownClose(BililiveComboboxDropdownView *dropdown_view);
    void OnSelectedIndexChanged(const _COMBOBOX_ITEM &item);
    void OnDetectMousePressed();

private:
    friend class BililiveComboboxDropdownView;
    friend class BililiveComboboxDroplistItemView;
    friend class BililiveComboboxDropdownWidget;
    bool editable_;
    bool equal_width_ = false;
    int fixed_width_ = -1;
    EditBlurReason edit_blur_reason_;
    BililiveComboboxDropdownView *dropdown_view_;
    BililiveComboboxExListener* listener_;
    EditLabelArea *main_area_;
    views::Label *label_;
    BilibiliNativeEditView *edit_view_;
    views::ImageView *arrow_;
    _COMBOBOX_ITEM default_item_;
    AnchorPosition anchor_position_;
    bool is_dropdown_;
    SkColor droplist_border_color_;
    SkColor droplist_bk_color_;
    SkColor border_color_;
    SkColor border_focus_color_;
    SkColor item_color_;
    SkColor item_focus_color_;
    gfx::Font font_;

    gfx::ImageSkia *arrow_down_img_;
    gfx::ImageSkia *arrow_down_hv_img_;
    gfx::ImageSkia *arrow_up_hv_img_;

    int default_width_in_chars_;
    base::string16 default_width_padding_char_;

    ScopedVector<_COMBOBOX_ITEM> items_;
    int current_index_;

    base::RepeatingTimer<BililiveComboboxEx> detect_timer_;
    base::WeakPtrFactory<BililiveComboboxEx> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(BililiveComboboxEx);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_COMBOX_BILILIVE_COMBOBOX_H
