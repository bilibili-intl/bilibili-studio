
#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_COMBOX_BILI_SEARCH_LIST_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_COMBOX_BILI_SEARCH_LIST_H

#include "bilibili_native_edit_view.h"

#include "base/basictypes.h"
#include "base/memory/scoped_vector.h"
#include "base/timer/timer.h"

#include "ui/views/controls/button/custom_button.h"
#include "ui/views/widget/widget_observer.h"


class BilibiliSearchListView;
class BilibiliSearchListDroplistItemView;

namespace
{
    class BilibiliSearchListDropdownView;
    class BilibiliSearchListDropdownWidget;

    class BilibiliSearchListDroplistItemDelegate
    {
    public:
        virtual void OnSearchDroplistItemSelected(BilibiliSearchListDroplistItemView* item_view, const ui::Event& event) = 0;
    };
}

enum class BiliveSearchListBlurChangeReason
{
    CHANGED_BY_DEFAULT,
    CHANGED_BY_USER_ESC,
    CHANGED_BY_USER_RETURN,
};

// 组合框数据项
struct _SEARCH_ITEM
{
    explicit _SEARCH_ITEM(const base::string16& str, int64 tag)
        : text(str)
        , data(tag)
        , valid(true)
    {
        id = ::InterlockedIncrement(&inc);
    }

    ~_SEARCH_ITEM() = default;

    long id;
    base::string16 text;
    int64 data;
    bool valid;

private:
    static volatile long inc;
};

// 搜索结果集选项view
class BilibiliSearchListDroplistItemView : public views::CustomButton
{
public:
    BilibiliSearchListDroplistItemView(const _SEARCH_ITEM& data, BilibiliSearchListView* search_edit);

    _SEARCH_ITEM& data() { return data_; }

    virtual void SetSelected(bool selected);

protected:
    // Button
    void NotifyClick(const ui::Event& event) override;
    // CustomButton
    //void StateChanged() override;

private:
    void SetDelegateInternal(BilibiliSearchListDroplistItemDelegate* delegate);

protected:
    BilibiliSearchListView *search_edit_ = nullptr;
    _SEARCH_ITEM data_;
    bool is_selected_ = false;

private:
    friend class BilibiliSearchListDropdownView;
    BilibiliSearchListDroplistItemDelegate* delegate_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(BilibiliSearchListDroplistItemView);
};

// 搜索列表控制器，指定创建什么选项view，经典用法是上层派生BilibiliSearchListDroplistItemView，
// 然后在继承Controller在GetDroplistItemView返回派生的ItemView实例
class BilibiliSearchListController
{
public:
    virtual BilibiliSearchListDroplistItemView* GetDroplistItemView(
        const _SEARCH_ITEM& data, BilibiliSearchListView* search_edit);

protected:
    virtual ~BilibiliSearchListController() = default;
};

class BilibiliSearchListListener {
public:
    virtual void OnSearchListResultSelected(BilibiliSearchListView* search_edit,
        const base::string16 &text, int64 data) = 0;
    virtual void OnSearchEditContentsChanged(BilibiliSearchListView* search_edit, const string16& new_contents) {}
    virtual void OnSearchEditFocus(BilibiliSearchListView* search_edit) {}
    virtual void PreSearchEditBlur(BilibiliSearchListView* search_edit, const gfx::Point& pt_in_screen) {}
    virtual void OnSearchEditBlur(BilibiliSearchListView* search_edit, BiliveSearchListBlurChangeReason reason) {}
    virtual void OnSearchListDropDown(BilibiliSearchListView* search_edit) {}
    virtual void OnSearchListDropDownClosed(BilibiliSearchListView* search_edit) {}

protected:
    virtual ~BilibiliSearchListListener() {}
};

class BilibiliSearchListView
    : public views::View
    , public views::WidgetObserver
    , public BilibiliNativeEditController
{
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

public:
    explicit BilibiliSearchListView(BilibiliSearchListListener *listener, views::View *relate_view = nullptr,
        BilibiliNativeEditView::CooperateDirection dir = BilibiliNativeEditView::BD_RIGHT);
    virtual ~BilibiliSearchListView();

    void SetBlurChangeReason(BiliveSearchListBlurChangeReason reason);
    void SetController(BilibiliSearchListController* controller);
    BilibiliSearchListController* controller();

    // 编辑框相关
    void SetText(const base::string16& text, BilibiliNativeEditView::EnChangeReason reason = 
        BilibiliNativeEditView::EnChangeReason::ECR_API);
    base::string16 GetText() const;
    void SetTextColor(SkColor color);
    void SetFont(const gfx::Font& font);
    const gfx::Font& font() const;
    void SetLimitText(unsigned int max);
    void SetBackgroundColor(SkColor color);
    void SetBorderColor(SkColor clrNormal, SkColor clrFocus, SkColor clrDroplist);
    void SetPlaceHolderText(const base::string16& text);
    const string16& placeholder_text() const;
    void SetPlaceHolderTextColor(SkColor clr);
    void SetHorizontalMargins(int left, int right);
    void GetHorizontalMargins(int &left, int &right) const;
    void SelectAll();
    void SetNumberOnly(bool number_only);

    // combobox数据项相关
    void SetItemColor(SkColor clrWordsNormal, SkColor clrBkNormal, SkColor clrWordsHoverd, SkColor clrBkHoverd);
    SkColor item_words_normal_color() const;
    SkColor item_words_hover_color() const;
    int AddItem(const base::string16& str, int64 data = 0);
    int FindItem(const base::string16& str, int start = 0);
    int FindItemData(int64 data, int start = 0);
    int DeleteItem(int index);
    int GetItemCount();
    void ClearItems(bool close_drop = true);
    bool SetItemData(int index, int64 data);
    bool EnabledItem(int index, bool enable);
    int64 GetItemData(int index);
    bool SetItemText(int index, const base::string16& str);
    base::string16 GetItemText(int index);
    // 下拉框相关
    void SetDroplistEqualWidth(bool equal);
    bool droplist_equal_width() const { return equal_width_; }
    void DoDropDown();

    // View
    void RequestFocus() override;

protected:
    // View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;
    void VisibilityChanged(views::View* starting_from, bool is_visible) override;
    void OnBoundsChanged(const gfx::Rect& previous_bounds) override;
    void OnEnabledChanged() override;
    void OnFocus() override;

    // WidgetObserver
    void OnWidgetBoundsChanged(views::Widget* widget, const gfx::Rect& new_bounds) override;

    // BilibiliNativeEditController
    void ContentsChanged(BilibiliNativeEditView* sender, const string16& new_contents) override;
    bool PreHandleMSG(BilibiliNativeEditView* sender, UINT msg, WPARAM wParam, LPARAM lParam) override;

    BilibiliNativeEditView* edit_ctrl() { return search_edit_; }

private:
    void InitView();
    void UninitView();
    void HideDropdown();
    void OnDropdownHide();
    void OnDropdownClose();
    void OnSearchResultItemSelected(const _SEARCH_ITEM &item);
    void OnDetectMousePressed();

private:
    friend class BilibiliSearchListDropdownView;
    friend class BilibiliSearchListDropdownWidget;
    BilibiliSearchListListener* listener_ = nullptr;
    BilibiliSearchListController* controller_ = nullptr;
    BilibiliNativeEditView *search_edit_ = nullptr;
    BilibiliSearchListDropdownView *dropdown_view_ = nullptr;
    AnchorPosition anchor_position_;
    bool is_edit_ldown_now_;
    bool is_edit_rdown_now_;
    bool is_last_lbtn_valid_;
    bool equal_width_ = false;
    BiliveSearchListBlurChangeReason blur_reason_ = BiliveSearchListBlurChangeReason::CHANGED_BY_DEFAULT;

    SkColor item_words_normal_color_;
    SkColor item_bk_normal_color_;
    SkColor item_words_hover_color_;
    SkColor item_bk_hoverd_color_;
    SkColor droplist_border_color_;
    SkColor droplist_bk_color_;
    ScopedVector<_SEARCH_ITEM> items_;

    base::RepeatingTimer<BilibiliSearchListView> detect_timer_;
    base::WeakPtrFactory<BilibiliSearchListView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(BilibiliSearchListView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_COMBOX_BILILIVE_COMBOBOX_H
