
#include "bililive/bililive/ui/views/controls/combox/bililive_combobox.h"
#include "bililive/bililive/ui/views/controls/bililive_theme_common.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_number_conversions.h"

#include "third_party/skia/include/core/SkPaint.h"

#include "ui/gfx/canvas.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/controls/textfield/native_textfield_win.h"
#include "ui/base/resource/resource_bundle.h"

#include "grit/theme_resources.h"
#include "ui/base/win/dpi.h"

// Size of the combobox arrow margins
const int kDisclosureArrowLeftPadding = 7 * ui::GetDPIScale();
const int kDisclosureArrowRightPadding = 7 * ui::GetDPIScale();


BililiveComboboxModel::_COMBOBOX_ITEM::_COMBOBOX_ITEM(const base::string16 &str, int64 tag, BililiveComboboxDataOperator* op)
    : text(str)
    , dataOp(op)
{
    data = dataOp->Alloc();
    dataOp->Assign(data, tag);
}

BililiveComboboxModel::_COMBOBOX_ITEM::~_COMBOBOX_ITEM()
{
    dataOp->Free(data);
}


BililiveComboboxModel::BililiveComboboxModel(BililiveComboboxDataOperator* dataOp)
    : default_index_(-1)
    , dataOp_(dataOp)
{}

BililiveComboboxModel::~BililiveComboboxModel()
{

}

void BililiveComboboxModel::SetDataOperator(BililiveComboboxDataOperator* dataOp)
{
    CHECK(dataOp_ == 0);
    dataOp_ = dataOp;
}

int BililiveComboboxModel::AddItem(const base::string16 &str, int64 data/* = 0*/)
{
    item_vector_.push_back(new _COMBOBOX_ITEM(str, data, dataOp_));
    return item_vector_.size() - 1;
}

void BililiveComboboxModel::ClearItems()
{
    item_vector_.clear();
}

int BililiveComboboxModel::FindItem(const base::string16 &str, int start/* = 0*/)
{
    for (int i = start; i < static_cast<int>(item_vector_.size()); i++)
    {
        if (item_vector_[i]->text.find(str) != -1)
        {
            return i;
        }
    }
    return -1;
}

int BililiveComboboxModel::FindItemData(int64 data, int start)
{
    for (int i = start; i < static_cast<int>(item_vector_.size()); i++)
    {
        auto dataOp = item_vector_[i]->dataOp;
        if (dataOp->Compare(item_vector_[i]->data, data))
        {
            return i;
        }
    }
    return -1;
}

bool BililiveComboboxModel::SetItemData(int index, int64 data)
{
    if (index >= 0 && index < static_cast<int>(item_vector_.size()))
    {
        dataOp_->Assign(item_vector_[index]->data, data);
        return true;
    }
    return false;
}

int64 BililiveComboboxModel::GetItemData(int index)
{
    if (index >= 0 && index < static_cast<int>(item_vector_.size()))
    {
        return item_vector_[index]->data;
    }
    return 0;
}

base::string16 BililiveComboboxModel::GetItemText(int index)
{
    return GetItemAt(index);
}

bool BililiveComboboxModel::SetItemText(int index, const base::string16& str)
{
    if (index >= 0 && index < static_cast<int>(item_vector_.size()))
    {
        item_vector_[index]->text = str;
        return true;
    }
    return false;
}



BililiveNativeComboboxViews::BililiveNativeComboboxViews(BililiveCombobox* combobox)
    : views::NativeComboboxViews(combobox)
    , border_color_(clrComboboxBorder)
    , border_focus_color_(clrTextfieldFocusBorder)
    , text_color_(clrLabelText)
    , textfield_(NULL)
    , edit_able_(false)
    , is_mouse_enter_(false)
{
    set_focus_border(nullptr);
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();
    down_arrow_image_ = rb.GetImageSkiaNamed(IDR_LIVEMAIN_TABAREA_SCENE_DOWN);
    up_arrow_image_ = rb.GetImageSkiaNamed(IDR_LIVEMAIN_TABAREA_SCENE_UP);
}

void BililiveNativeComboboxViews::OnPaint(gfx::Canvas* canvas)
{
    OnPaintBackground(canvas);
    PaintText(canvas);
    OnPaintBorder(canvas);
}

void BililiveNativeComboboxViews::PaintText(gfx::Canvas* canvas)
{
    gfx::Insets insets = GetInsets();

    canvas->Save();
    canvas->ClipRect(GetContentsBounds());

    int x = insets.left();
    int y = insets.top();
    int text_height = height() - insets.height();
    SkColor text_color = combobox()->enabled() ? text_color_ :
        GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_ButtonDisabledColor);

    string16 text;
    int index = GetSelectedIndex();
    if (index >= 0 && index < combobox()->model()->GetItemCount())
    {
        text = combobox()->model()->GetItemAt(index);
    }

    int disclosure_arrow_offset = width() - down_arrow_image_->width()
        - kDisclosureArrowLeftPadding - kDisclosureArrowRightPadding;

    const gfx::Font& font = views::Combobox::GetFont();
    int text_width = font.GetStringWidth(text);
    if ((text_width + insets.width()) > disclosure_arrow_offset)
        text_width = disclosure_arrow_offset - insets.width();

    gfx::Rect text_bounds(x, y, text_width, text_height);
    AdjustBoundsForRTLUI(&text_bounds);
    canvas->DrawStringInt(text, font, text_color, text_bounds);

    gfx::Rect arrow_bounds(disclosure_arrow_offset + kDisclosureArrowLeftPadding,
        height() / 2 - down_arrow_image_->height() / 2,
        down_arrow_image_->width(),
        down_arrow_image_->height());
    AdjustBoundsForRTLUI(&arrow_bounds);

    SkPaint imgpaint;
    // This makes the arrow subtractive.
    if (combobox()->invalid())
        imgpaint.setXfermodeMode(SkXfermode::kDstOut_Mode);
    if (IsDropdownOpen())
    {
        canvas->DrawImageInt(*up_arrow_image_, arrow_bounds.x(), arrow_bounds.y(), imgpaint);
    }
    else
    {
        canvas->DrawImageInt(*down_arrow_image_, arrow_bounds.x(), arrow_bounds.y(), imgpaint);
    }

    canvas->Restore();
}

void BililiveNativeComboboxViews::OnPaintBorder(gfx::Canvas* canvas)
{
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(is_mouse_enter_ ? border_focus_color_ : border_color_); //Ïß¿òÑÕÉ«
    bililive::DrawRoundRect(canvas, 0, 0, width(), height(), 4 * ui::GetDPIScale(), paint);
}

void BililiveNativeComboboxViews::SetBorderColor(SkColor clrBlur, SkColor clrFocus)
{
    border_color_ = clrBlur;
    border_focus_color_ = clrFocus;
}

bool BililiveNativeComboboxViews::OnMousePressed(const ui::MouseEvent& mouse_event)
{
    if (edit_able_)
    {
        if (mouse_event.IsOnlyLeftMouseButton())
        {
            gfx::Rect rt = GetBoundsInScreen();
            rt.Inset(0, 0, down_arrow_image_->width() + kDisclosureArrowLeftPadding + kDisclosureArrowRightPadding, 0);
            POINT point = { 0 };
            ::GetCursorPos(&point);
            gfx::Point pt(point);
            if (rt.Contains(pt))
            {
                gfx::Rect text_bounds(0, 0, width() - (down_arrow_image_->width() + kDisclosureArrowLeftPadding + kDisclosureArrowRightPadding), height());
                text_bounds.Inset(4 * ui::GetDPIScale(), 1 * ui::GetDPIScale(), 0, 1 * ui::GetDPIScale());
                textfield_->SetBounds(text_bounds.x(), text_bounds.y(), text_bounds.width(), text_bounds.height());

                base::string16 text = ((BililiveCombobox*)combobox())->GetItemText(combobox()->selected_index());

                textfield_->SetVisible(true);
                textfield_->RequestFocus();
                textfield_->SetText(text);
                textfield_->SelectAll(true);

                return true;
            }
        }
    }



    return __super::OnMousePressed(mouse_event);
}

void BililiveNativeComboboxViews::OnMouseEntered(const ui::MouseEvent& event)
{
    is_mouse_enter_ = true;
    SchedulePaint();
}

void BililiveNativeComboboxViews::OnMouseExited(const ui::MouseEvent& event)
{
    is_mouse_enter_ = false;
    SchedulePaint();
}

void BililiveNativeComboboxViews::SetEditAble(bool able)
{
    edit_able_ = able;
    if (able)
    {
        createTextfield();
    }
    else
    {
        if (textfield_)
        {
            RemoveChildView(textfield_);
            delete textfield_;
        }
    }
}

void BililiveNativeComboboxViews::SetTextInputType(ui::TextInputType type)
{
    createTextfield();
    if (textfield_)
    {
        textfield_->SetTextInputType(type);
    }
}

void BililiveNativeComboboxViews::createTextfield()
{
    if (edit_able_ && !textfield_)
    {
        edit_able_ = true;
        textfield_ = new BililiveComboboxTextfield(this);
        textfield_->SetTextColor(text_color_);
        textfield_->RemoveBorder();
        textfield_->SetBackgroundColor(clrClientAreaBk);
        textfield_->SetBililiveComboboxTextfieldDelegate((BililiveCombobox*)combobox());
        AddChildView(textfield_);
    }
}

void BililiveNativeComboboxViews::AdjustBoundsForRTLUI(gfx::Rect* rect) const {
    rect->set_x(GetMirroredXForRect(*rect));
}

void BililiveNativeComboboxViews::BililiveComboboxTextfield::SetBililiveComboboxTextfieldDelegate(BililiveComboboxTextfieldDelegate *delegate)
{
    SetController(delegate);
    delegate_ = delegate;
}

void BililiveNativeComboboxViews::BililiveComboboxTextfield::OnBlur()
{
    if (delegate_)
    {
        delegate_->OnTextfieldBlur();
    }
}

bool BililiveNativeComboboxViews::BililiveComboboxTextfield::OnKeyPressed(const ui::KeyEvent& e)
{
    if (delegate_)
    {
        delegate_->OnTextfieldKeyPressed(e);
    }
    return __super::OnKeyPressed(e);
}



BililiveCombobox::BililiveCombobox(BililiveComboboxDataOperator* dataOp)
    : views::Combobox(new BililiveComboboxModel(dataOp))
    , dataOp_(dataOp)
    , edit_able_(false)
    , text_input_type_(ui::TEXT_INPUT_TYPE_NONE)
    , border_color_(clrComboboxBorder)
    , border_focus_color_(clrTextfieldFocusBorder)
    , text_color_(clrLabelText)
{
    set_focus_border(nullptr);
}

BililiveCombobox::~BililiveCombobox()
{
    delete (BililiveComboboxModel*)model();
    dataOp_->DeleteThis();
}

int BililiveCombobox::AddItem(const base::string16 &str, int64 data/* = 0*/)
{
    int r = ((BililiveComboboxModel*)model())->AddItem(str, data);
    SchedulePaint();
    return r;
}

int BililiveCombobox::FindItem(const base::string16 &str, int start/* = 0*/)
{
    return ((BililiveComboboxModel*)model())->FindItem(str, start);
}

int BililiveCombobox::FindItemData(int64 data, int start)
{
    return ((BililiveComboboxModel*)model())->FindItemData(data, start);
}

int BililiveCombobox::GetItemCount()
{
    return ((BililiveComboboxModel*)model())->GetItemCount();
}

void BililiveCombobox::ClearItems()
{
    ((BililiveComboboxModel*)model())->ClearItems();
    SchedulePaint();
}

bool BililiveCombobox::SetItemData(int index, int64 data)
{
    return ((BililiveComboboxModel*)model())->SetItemData(index, data);
}

int64 BililiveCombobox::GetItemData(int index)
{
    return ((BililiveComboboxModel*)model())->GetItemData(index);
}

bool BililiveCombobox::SetItemText(int index, const base::string16& str)
{
    bool r = ((BililiveComboboxModel*)model())->SetItemText(index, str);
    if (r)
    {
        SchedulePaint();
    }
    return r;
}

base::string16 BililiveCombobox::GetItemText(int index)
{
    return ((BililiveComboboxModel*)model())->GetItemText(index);
}

void BililiveCombobox::SetDefaultIndex(int index)
{
    ((BililiveComboboxModel*)model())->SetDefaultIndex(index);
    if (selected_index() == -1)
    {
        if (index < GetRowCount())
        {
            SetSelectedIndex(index);
        }
    }
}

void BililiveCombobox::SetEditAble(bool able)
{
    edit_able_ = able;
    if (native_wrapper_)
    {
        ((BililiveNativeComboboxViews*)native_wrapper_)->SetEditAble(edit_able_);
    }
}

void BililiveCombobox::SetBorderColor(SkColor clrBlur, SkColor clrFocus)
{
    border_color_ = clrBlur;
    border_focus_color_ = clrFocus;
    if (native_wrapper_)
    {
        ((BililiveNativeComboboxViews*)native_wrapper_)->SetBorderColor(clrBlur, clrFocus);
    }
}

void BililiveCombobox::SetTextColor(SkColor clr)
{
    text_color_ = clr;
    if (native_wrapper_)
    {
        ((BililiveNativeComboboxViews*)native_wrapper_)->SetTextColor(clr);
    }
}

ui::TextInputType BililiveCombobox::GetTextInputType() const
{
    views::Textfield* text = textfield();
    if (text)
    {
        return text->GetTextInputType();
    }
    return text_input_type_;
}

void BililiveCombobox::SetTextInputType(ui::TextInputType type)
{
    text_input_type_ = type;
    if (native_wrapper_)
    {
        ((BililiveNativeComboboxViews*)native_wrapper_)->SetTextInputType(text_input_type_);
    }
}

void BililiveCombobox::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
    if (details.is_add && !native_wrapper_ && GetWidget()) {
        // The native wrapper's lifetime will be managed by the view hierarchy after
        // we call AddChildView.
        native_wrapper_ = new BililiveNativeComboboxViews(this);
        AddChildView(native_wrapper_->GetView());
        // The underlying native widget may not be created until the wrapper is
        // parented. For this reason the wrapper is only updated after adding its
        // view.
        native_wrapper_->UpdateFromModel();
        native_wrapper_->UpdateSelectedIndex();
        native_wrapper_->UpdateEnabled();

        SetEditAble(edit_able_);
        SetTextInputType(text_input_type_);
        SetTextColor(text_color_);
        SetBorderColor(border_color_, border_focus_color_);
    }
}

gfx::Size BililiveCombobox::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.Enlarge(enlarge_size_.width(), enlarge_size_.height());
    size.SetToMax(pref_size_);
    return size;
}

views::Textfield* BililiveCombobox::textfield() const
{
    return ((BililiveNativeComboboxViews*)native_wrapper_)->textfield_;
}

bool BililiveCombobox::HitTestRect(const gfx::Rect& rect) const
{
    if (!enabled())
    {
        return false;
    }
    return __super::HitTestRect(rect);
}



class BililiveComboboxInt64DataOperator : public BililiveComboboxDataOperator
{
public:
    int64 Alloc() override { return 0; }
    void Free(const int64& val) override {}
    void Assign(int64& lhs, const int64& rhs) override { lhs = rhs; }
    bool Compare(const int64& lhs, const int64& rhs) override { return lhs == rhs; }
    void DeleteThis() override { delete this; }
};

BililiveComboboxDataOperator* BililiveCreateInt64Operator()
{
    return new BililiveComboboxInt64DataOperator();
}

template<class T>
class BililiveComboboxStringDataOperator : public BililiveComboboxDataOperator
{
public:
    int64 Alloc() override
    {
        return (int64)(new T());
    }

    void Free(const int64& val) override
    {
        delete (T*)val;
    }

    void Assign(int64& lhs, const int64& rhs) override
    {
        *(T*)lhs = *(const T*)rhs;
    }

    bool Compare(const int64& lhs, const int64& rhs) override
    {
        return *(T*)lhs == *(const T*)rhs;
    }

    void DeleteThis() override { delete this; }
};

BililiveComboboxDataOperator* BililiveCreateStringOperator()
{
    return new BililiveComboboxStringDataOperator<std::string>();
}

BililiveComboboxDataOperator* BililiveCreateWStringOperator()
{
    return new BililiveComboboxStringDataOperator<std::wstring>();
}

