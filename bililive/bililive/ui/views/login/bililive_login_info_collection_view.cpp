#include "bililive/bililive/ui/views/login/bililive_login_info_collection_view.h"

#include "third_party/skia/include/core/SkColor.h"

#include "ui/views/controls/button/button.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/widget/widget.h"

#include "bililive/bililive/ui/views/controls/gridview.h"
#include "bililive/bililive/ui/views/controls/bililive_floating_scroll_view.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_user_related_ctrls.h"
#include "bililive/bililive/ui/views/login/bilibili_login_control.h"
#include "bililive/bililive/ui/views/login/bililive_login_main_view.h"
#include "bililive/bililive/ui/views/login/livehime_login_message_box.h"
#include "bililive/bililive/ui/views/login/livehime_login_util.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_bubble.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/bililive_paths.h"
#include "bililive/public/secret/bililive_secret.h"

#include "base/file_util.h"
#include "base/path_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"


namespace
{
    int kAccountItemHeight = GetLengthByDPIScale(54);
    gfx::Size kAccountItemAvatarSize = GetSizeByDPIScale(gfx::Size(36, 36));
    gfx::Size kAvatarSize = GetSizeByDPIScale(gfx::Size(55, 55));

    enum CtrlID
    {
        Button_IDS = 1,
        Button_Obscured,
        Check_Remenber,
        Check_Auto,
        Button_Register,
        Button_Forget,
        Button_Login,

        AccountItem_Self,
        AccountItem_Del,
    };

    void CreateBubble(views::View *center, const string16 &tip)
    {
        TipBubble *bubble = new TipBubble(center, tip);

        views::BubbleDelegateView::CreateBubble(bubble);
        bubble->SetAlignment(views::BubbleBorder::ALIGN_EDGE_TO_ANCHOR_EDGE);
        bubble->GetWidget()->Show();
    }

    class HistoryAccountListView;
    HistoryAccountListView* single_instance = nullptr;

    class HistoryAccountsDropdownButton : public BililiveImageButton
    {
    public:
        explicit HistoryAccountsDropdownButton(views::ButtonListener* listener)
            : BililiveImageButton(listener)
        { }

    protected:
        bool OnMousePressed(const ui::MouseEvent& event) override
        {
            bool result = __super::OnMousePressed(event);
            __super::NotifyClick(event);
            return true;
        }

        void NotifyClick(const ui::Event& event) override
        {
            // do nth
            //__super::NotifyClick(event);
        }

    private:
    };

    class AccountItemDelegate
    {
    public:
        virtual ~AccountItemDelegate() = default;

        virtual void AccountItemPressed(const secret::LoginInfo& account, int ctrl_id) = 0;
    };

    class AccountItem
        : public GridItemView
        , views::ButtonListener
    {
    public:
        static GridItemView* CreateAccountItem(GridView *grid_view, AccountItemDelegate* delegate)
        {
            return new AccountItem(grid_view, delegate);
        }

        AccountItem(GridView *grid_view, AccountItemDelegate* delegate)
            : GridItemView(0, grid_view)
            , delegate_(delegate)
        {
            set_notify_enter_exit_on_child(true);

            views::GridLayout *layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            views::ColumnSet *column_set = layout->AddColumnSet(0);
            column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
            column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

            avatar_view_ = new LivehimeCircleImageView(nullptr);
            avatar_view_->SetImagePlaceholder(*GetImageSkiaNamed(IDR_LIVEMAIN_TITLEBAR_USER_FACE));
            avatar_view_->SetPreferredSize(kAccountItemAvatarSize);

            account_name_label_ = new LivehimeTitleLabel();
            account_name_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);

            mid_label_ = new LivehimeContentLabel();
            mid_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);

            del_button_ = new BililiveImageButton(this);
            del_button_->SetAllStateImage(GetImageSkiaNamed(IDR_LIVEMAIN_JOINMIC_USER_LIST_CANCEL_SEARCH));
            del_button_->SetVisible(false);

            layout->AddPaddingRow(1.0f, 0);
            layout->StartRow(0, 0);
            layout->AddView(avatar_view_, 1, 2, views::GridLayout::FILL, views::GridLayout::CENTER, 0, 0);
            layout->AddView(account_name_label_);
            layout->AddView(del_button_, 1, 2, views::GridLayout::FILL, views::GridLayout::CENTER, 0, 0);

            layout->StartRow(0, 0);
            layout->SkipColumns(1);
            layout->AddView(mid_label_);
            layout->AddPaddingRow(1.0f, 0);
        }

        void SetAccountInfo(const secret::LoginInfo& info)
        {
            account_info_ = info;
            std::string content(std::move(livehime::GetAvatarBinaryDataByMid(account_info_.mid)));
            if (!content.empty())
            {
                avatar_view_->SetImageBinary((const unsigned char*)content.data(), content.length());
            }
            account_name_label_->SetText(base::UTF8ToUTF16(account_info_.name));
            mid_label_->SetText(std::to_wstring(account_info_.mid));
            Layout();
        }

        void SetChecked(bool checked)
        {
            if (checked_ != checked)
            {
                checked_ = checked;
                SchedulePaint();
            }
        }

    protected:
        // View
        gfx::Size GetPreferredSize() override
        {
            gfx::Size size = __super::GetPreferredSize();
            size.set_height(kAccountItemHeight);
            return size;
        }

        int GetHeightForWidth(int w) override
        {
            return kAccountItemHeight;
        }

        void OnPaintBackground(gfx::Canvas* canvas) override
        {
            if (checked_)
            {
                canvas->FillRect(GetContentsBounds(), GetColor(ListItemBkPre));
            }
            else
            {
                if (mouse_enter_)
                {
                    canvas->FillRect(GetContentsBounds(), GetColor(ListItemBkHov));
                }
            }
        }

        void OnMouseEntered(const ui::MouseEvent& event) override
        {
            mouse_enter_ = true;
            StateChanged();
        }

        void OnMouseExited(const ui::MouseEvent& event) override
        {
            mouse_enter_ = false;
            StateChanged();
        }

        bool OnMousePressed(const ui::MouseEvent& event) override
        {
            return true;
        }

        void OnMouseReleased(const ui::MouseEvent& event) override
        {
            if (delegate_)
            {
                delegate_->AccountItemPressed(account_info_, AccountItem_Self);
            }
        }

        void StateChanged()
        {
            if (mouse_enter_ != del_button_->visible())
            {
                del_button_->SetVisible(mouse_enter_);
            }
            SchedulePaint();
        }

        // ButtonListener
        void ButtonPressed(views::Button* sender, const ui::Event& event) override
        {
            if (delegate_)
            {
                delegate_->AccountItemPressed(account_info_, AccountItem_Del);
            }
        }

        // GridItemView
        void BeforePushBackToItemDeque() override
        {
            mouse_enter_ = false;
            checked_ = false;
            del_button_->SetVisible(false);
        }

    private:
        AccountItemDelegate* delegate_ = nullptr;
        secret::LoginInfo account_info_;
        bool checked_ = false;
        bool mouse_enter_ = false;
        LivehimeImageView* avatar_view_ = nullptr;
        BililiveLabel* account_name_label_ = nullptr;
        BililiveLabel* mid_label_ = nullptr;
        BililiveImageButton* del_button_ = nullptr;
    };

    class HistoryAccountListView
        : public views::WidgetDelegateView
        , views::WidgetObserver
        , GridModel
        , public AccountItemDelegate
    {
    public:
        static void DoDropDown(views::View* relate_view, const base::string16& cur_account,
            const std::vector<secret::LoginInfo>& account_list, HistoryAccountListViewDelegate* delegate)
        {
            if (account_list.empty())
            {
                return;
            }

            if (!single_instance)
            {
                if (!account_list.empty())
                {
                    HistoryAccountListView* account_list_view = new HistoryAccountListView(delegate);
                    views::Widget *widget = new views::Widget();
                    views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
                    params.parent = relate_view->GetWidget()->GetNativeView();
                    params.delegate = account_list_view;
                    params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
                    widget->Init(params);
                    widget->Hide();
                }
            }

            if (single_instance)
            {
                if (single_instance->GetWidget()->IsVisible())
                {
                    single_instance->GetWidget()->Hide();
                }
                else
                {
                    // 当前文本框的账号放在下拉列表的第一项
                    std::string a_cur_account(base::UTF16ToUTF8(cur_account));
                    std::vector<secret::LoginInfo> sort_list;
                    for (auto& iter : account_list)
                    {
                        if (iter.name == a_cur_account)
                        {
                            sort_list.insert(sort_list.begin(), iter);
                        }
                        else
                        {
                            sort_list.push_back(iter);
                        }
                    }

                    single_instance->UpdateData(sort_list, a_cur_account);
                    gfx::Size pref_size = single_instance->GetWidget()->GetRootView()->GetPreferredSize();

                    gfx::Rect view_bounds = relate_view->GetBoundsInScreen();
                    single_instance->GetWidget()->SetBounds(gfx::Rect(view_bounds.x(), view_bounds.bottom(),
                        view_bounds.width(), pref_size.height()));
                    single_instance->GetWidget()->Show();
                    single_instance->GetWidget()->Activate();
                }
            }
        }

    protected:
        HistoryAccountListView(HistoryAccountListViewDelegate* delegate)
            : delegate_(delegate)
            , weakptr_factory_(this)
        {
            single_instance = this;
        }

        virtual ~HistoryAccountListView()
        {
            single_instance = nullptr;
        }

        void UpdateData(const std::vector<secret::LoginInfo>& account_list, const std::string& cur_account)
        {
            account_list_ = account_list;
            cur_account_ = cur_account;
            grid_view_->UpdateData(GridView::TOTOP);
        }

        // View
        void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override
        {
            if (details.child == this)
            {
                if (details.is_add)
                {
                    InitViews();

                    GetWidget()->AddObserver(this);
                }
                else
                {
                    UninitViews();
                }
            }
        }

        gfx::Size GetPreferredSize() override
        {
            int cy = account_list_.size() * kAccountItemHeight;
            cy = std::min(kAccountItemHeight * 4, cy);
            if (border())
            {
                cy += border()->GetInsets().height();
            }
            return gfx::Size(GetLengthByDPIScale(250), cy);
        }

        // WidgetDelegate
        View* GetContentsView() override { return this; }
        void WindowClosing() override
        {
            UninitViews();
        }

        // WidgetObserver
        void OnWidgetActivationChanged(views::Widget* widget, bool active) override
        {
            if (widget == GetWidget())
            {
                if (!active)
                {
                    base::MessageLoop::current()->PostTask(FROM_HERE,
                        base::Bind(&HistoryAccountListView::HideWidget, weakptr_factory_.GetWeakPtr()));
                }
            }
        }

        void OnWidgetVisibilityChanged(views::Widget* widget, bool visible) override
        {
            if (widget == GetWidget())
            {
                if (delegate_)
                {
                    delegate_->OnListViewDropdown(visible);
                }
            }
        }

        // GridModel
        int GetNumberOfGroups() override { return 1; }
        int GetGroupHeaderHeight(int group_index) override { return 0; }
        int GetItemCountForGroup(int group_index) override
        {
            return (int)account_list_.size();
        }
        gfx::Size GetItemSizeForGroup(const gfx::Rect& show_bounds, int group_index) override
        {
            return gfx::Size(show_bounds.width(), kAccountItemHeight);
        }
        int GetGroupHeaderTypeId(int group_index) override { return -1; }
        int GetGroupItemTypeId(int group_index, int item_index) override { return 0; }
        void UpdateData(GridItemView *item) override
        {
            if (item->item_index() >= 0 && item->item_index() < (int)account_list_.size())
            {
                ((AccountItem*)item)->SetAccountInfo(account_list_[item->item_index()]);
                ((AccountItem*)item)->SetChecked(account_list_[item->item_index()].name == cur_account_);
            }
        }

        // AccountItemDelegate
        void AccountItemPressed(const secret::LoginInfo& account, int ctrl_id) override
        {
            GetWidget()->Hide();
            if (ctrl_id == AccountItem_Self)
            {
                if (delegate_)
                {
                    delegate_->OnAccountItemSelected(account);
                }
            }
            else if (ctrl_id == AccountItem_Del)
            {
                if (delegate_)
                {
                    delegate_->OnAccountItemDeleted(account);
                }
            }
        }

    private:
        void InitViews()
        {
            grid_view_ = new GridView(this);
            grid_view_->RegisterItemType(0, base::Bind(&AccountItem::CreateAccountItem,
                grid_view_, this));

            SetLayoutManager(new views::FillLayout());
            AddChildView(grid_view_->CreateParentIfNecessary());

            set_background(views::Background::CreateSolidBackground(SK_ColorWHITE));
            set_border(views::Border::CreateSolidBorder(GetLengthByDPIScale(1), GetColor(CtrlBorderNor)));
        }

        void UninitViews()
        {
            if (GetWidget())
            {
                GetWidget()->RemoveObserver(this);
            }
        }

        void HideWidget()
        {
            GetWidget()->Hide();
        }

    private:
        HistoryAccountListViewDelegate* delegate_ = nullptr;
        std::vector<secret::LoginInfo> account_list_;
        std::string cur_account_;

        GridView* grid_view_ = nullptr;

        base::WeakPtrFactory<HistoryAccountListView> weakptr_factory_;
    };
}


LoginInfoCollectionView::LoginInfoCollectionView(LoginInfoCollectionDelegate *delegate)
    : delegate_(delegate)
    , weakptr_factory_(this)
{
    DCHECK(delegate_ != nullptr);
}

LoginInfoCollectionView::~LoginInfoCollectionView() {
}

void LoginInfoCollectionView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) {
    if (details.child == this) {
        if (details.is_add) {
            InitViews();
            InitData();
        }
    }
}

void LoginInfoCollectionView::InitViews() {

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet *column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(30));
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::LEADING, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(12));
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(30));

    user_pic_ = new LivehimeCircleImageView();
    user_pic_->SetPreferredSize(kAvatarSize);
    user_pic_->SetImagePlaceholder(*GetImageSkiaNamed(IDR_LOGIN_DEF_USR_FACE));

    input_area_ = new views::View();
    {
        views::GridLayout *grid_layout = new views::GridLayout(input_area_);
        input_area_->SetLayoutManager(grid_layout);

        views::ColumnSet *columnset = grid_layout->AddColumnSet(0);
        columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
        columnset->AddPaddingColumn(0, GetLengthByDPIScale(14));
        columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

        // id、注册
        ids_button_ = new HistoryAccountsDropdownButton(this);
        ids_button_->set_id(Button_IDS);
        ids_button_->set_focusable(false);

        id_edit_ = new LoginTextfield(ids_button_);
        id_edit_->SetController(this);
        id_edit_->set_placeholder_text(GetLocalizedString(IDS_LOGIN_UID_HINT));

        account_register_link_ = new LivehimeLinkButton(this, GetLocalizedString(IDS_LOGIN_REGISTER_LINK));
        account_register_link_->set_id(Button_Register);

        grid_layout->StartRow(0, 0);
        grid_layout->AddView(id_edit_, 1, 1, views::GridLayout::FILL, views::GridLayout::FILL, 0, ftPrimary.GetHeight() * 1.5f);
        grid_layout->AddView(account_register_link_);

        grid_layout->AddPaddingRow(0, GetLengthByDPIScale(5));   // 留一像素画分隔线

        // 密码、忘记
        obscured_button_ = new BililiveImageButton(this);
        obscured_button_->set_id(Button_Obscured);
        obscured_button_->set_focusable(false);
        BililiveHideAwareView* hide_view = new BililiveHideAwareView(true, true);
        hide_view->AddChildView(obscured_button_);

        pw_edit_ = new LoginTextfield(hide_view, BilibiliNativeEditView::CooperateDirection::BD_RIGHT,
            BilibiliNativeEditView::StyleFlags::STYLE_OBSCURED);
        pw_edit_->SetController(this);
        pw_edit_->set_placeholder_text(GetLocalizedString(IDS_LOGIN_PWD_HINT));

        pw_lost_link_ = new LivehimeLinkButton(this, GetLocalizedString(IDS_LOGIN_LOST_LINK));
        pw_lost_link_->set_id(Button_Forget);

        grid_layout->StartRow(0, 0);
        grid_layout->AddView(pw_edit_, 1, 1, views::GridLayout::FILL, views::GridLayout::FILL, 0, ftPrimary.GetHeight() * 1.5f);
        grid_layout->AddView(pw_lost_link_);

        // 记密，自登
        views::View *checkbox_view = new views::View();
        {
            views::GridLayout *checkbox_grid_layout = new views::GridLayout(checkbox_view);
            checkbox_view->SetLayoutManager(checkbox_grid_layout);

            auto inner_column_set = checkbox_grid_layout->AddColumnSet(0);
            inner_column_set->AddColumn(
                views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            inner_column_set->AddPaddingColumn(0, GetLengthByDPIScale(20));
            inner_column_set->AddColumn(
                views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            inner_column_set->AddPaddingColumn(1.0f, 0);

            pw_rem_check_ = new LivehimeCheckbox(GetLocalizedString(IDS_LOGIN_REMEMBER_PWD));
            pw_rem_check_->set_id(Check_Remenber);
            pw_rem_check_->set_listener(this);

            auto_login_check_ = new LivehimeCheckbox(GetLocalizedString(IDS_LOGIN_AUTO_LOGIN));
            auto_login_check_->set_id(Check_Auto);
            auto_login_check_->set_listener(this);

            checkbox_grid_layout->StartRow(0, 0);
            checkbox_grid_layout->AddView(pw_rem_check_);
            checkbox_grid_layout->AddView(auto_login_check_);
        }

        grid_layout->AddPaddingRow(0, GetLengthByDPIScale(5));
        grid_layout->StartRow(0, 0);
        grid_layout->AddView(checkbox_view);

        // 登录
        login_btn_ = new BilibiliLoginLabelButton(this, GetLocalizedString(IDS_LOGIN));
        login_btn_->set_id(Button_Login);
        login_btn_->SetIsDefault(true);

        grid_layout->AddPaddingRow(0, GetLengthByDPIScale(12));
        grid_layout->StartRow(0, 0);
        grid_layout->AddView(login_btn_);
    }

    layout->AddPaddingRow(0, GetLengthByDPIScale(12));
    layout->StartRow(0, 0);
    layout->AddView(user_pic_);
    layout->AddView(input_area_);
    layout->AddPaddingRow(0, GetLengthByDPIScale(12));

    input_area_->ReorderChildView(id_edit_, 0);
    input_area_->ReorderChildView(pw_edit_, 1);
}

void LoginInfoCollectionView::InitData()
{
    UpdateObscuredButtonImage();
    UpdateDropdownButtonImage(false);

    account_list_ = *GetBililiveProcess()->secret_core()->GetHistoryLoginInfo();
}

void LoginInfoCollectionView::UninitViews()
{
    if (single_instance)
    {
        single_instance->GetWidget()->Close();
    }
}

// ButtonListener
void LoginInfoCollectionView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    switch (sender->id())
    {
    case Button_IDS:
        GetWidget()->Activate();
        HistoryAccountListView::DoDropDown(id_edit_, id_edit_->text(), account_list_, this);
        break;
    case Button_Obscured:
        SetPSWEditObscured(!pw_edit_->IsObscured());
        break;
    case Button_Register:
        delegate_->OnInfoCollectionLinkClicked(static_cast<int>(LinkId_Register));
        break;
    case Button_Forget:
        delegate_->OnInfoCollectionLinkClicked(static_cast<int>(LinkId_Lost));
        break;
    case Check_Remenber:
        {
            if (!pw_rem_check_->checked())
            {
                auto_login_check_->SetChecked(false);
            }
        }
        break;
    case Check_Auto:
        {
            if (auto_login_check_->checked())
            {
                pw_rem_check_->SetChecked(true);
            }
        }
        break;
    case Button_Login:
        {
            if (id_edit_->text().empty())
            {
                CreateBubble(id_edit_, GetLocalizedString(IDS_LOGIN_NO_ID));
            }
            else if (pw_edit_->text().empty())
            {
                CreateBubble(pw_edit_, GetLocalizedString(IDS_LOGIN_NO_PASSWARD));
            }
            else
            {
                delegate_->OnInfoCollectionButtonPressed();
            }
        }
        break;
    default:
        break;
    }
}

// TextfieldController
void LoginInfoCollectionView::ContentsChanged(BilibiliNativeEditView* sender, const string16& new_contents)
{
    if (sender == id_edit_)
    {
        // 与历史列表进行匹配，帮助用户快速完成输入
        if (!del_or_bs_pressing_)
        {
            std::string account = base::WideToUTF8(new_contents);
            if (!account.empty())
            {
                for (auto& iter : account_list_)
                {
                    if (0 == iter.name.find(account))
                    {
                        //del_or_bs_pressing_ = true;
                        SetLoginInfo(iter);
                        sender->SelectRange(MAKELONG(account.length(), iter.name.length()));
                        return;
                    }
                }
            }
        }

        SetUserFace(0);
        ClearPassword();
        UncheckAllOption();
        account_info_.reset();
    }
    else if (sender == pw_edit_)
    {
        SetTokenVerify(false);
    }
}

bool LoginInfoCollectionView::PreHandleMSG(BilibiliNativeEditView* edit, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
    {
        if (edit == id_edit_)
        {
            del_or_bs_pressing_ = (wParam == ui::VKEY_BACK || wParam == ui::VKEY_DELETE);
        }
        else if (edit == pw_edit_)
        {
            //OnCapsLockTip();
        }
    }
    break;
    default:
    break;
    }
    return true;
}

void LoginInfoCollectionView::SetLoginInfo(const secret::LoginInfo& login_info)
{
    if (login_info.name.empty())
    {
        RequestIdInputFocus();
    }
    SetUserFace(login_info.mid);
    SetPSWEditObscured(true);
    id_edit_->SetText(base::UTF8ToUTF16(login_info.name));
    pw_edit_->SetText(base::UTF8ToUTF16(login_info.token));
    pw_rem_check_->SetChecked(!login_info.token.empty());
    auto_login_check_->SetChecked(login_info.auto_login);
    SetTokenVerify(!login_info.token.empty());

    account_info_ = std::make_unique<secret::LoginInfo>(login_info);
}

const std::unique_ptr<secret::LoginInfo>& LoginInfoCollectionView::account_info()
{
    return account_info_;
}

void LoginInfoCollectionView::TokenExpired()
{
    ClearPassword();
}

void LoginInfoCollectionView::ClearPassword()
{
    SetPSWEditObscured(true);
    pw_edit_->SetText(L"");
    SetTokenVerify(false);
}

int64 LoginInfoCollectionView::mid() const
{
    if (account_info_)
    {
        return account_info_->mid;
    }
    return 0;
}

void LoginInfoCollectionView::RequestIdInputFocus() {
    id_edit_->RequestFocus();
}

void LoginInfoCollectionView::OnListViewDropdown(bool show)
{
    UpdateDropdownButtonImage(show);
}

void LoginInfoCollectionView::OnAccountItemSelected(const secret::LoginInfo& account)
{
    GetWidget()->Activate();
    SetLoginInfo(account);
}

void LoginInfoCollectionView::OnAccountItemDeleted(const secret::LoginInfo& account)
{
    GetWidget()->Activate();

    std::unique_ptr<secret::LoginInfo> account_info = std::make_unique<secret::LoginInfo>(account);
    EndDialogSignalHandler handler;
    handler.closure = base::Bind(&LoginInfoCollectionView::DelAccountEndDialogHandler, weakptr_factory_.GetWeakPtr());
    handler.data = account_info.release();
    // 仅从列表删除
    LivehimeLoginDeleteAccountView::DoModal(account, GetWidget()->GetNativeView(), &handler);
}

void LoginInfoCollectionView::DelAccountEndDialogHandler(int ret, void* data)
{
    if (ret != LivehimeLoginDeleteAccountView::OptionType::Cancel && data)
    {
        std::unique_ptr<secret::LoginInfo> account_info;
        account_info.reset((secret::LoginInfo*)data);
        GetBililiveProcess()->secret_core()->RemoveHistoryLoginInfo(account_info->name,
            ret == LivehimeLoginDeleteAccountView::OptionType::DelDirectory);
        account_list_ = *GetBililiveProcess()->secret_core()->GetHistoryLoginInfo();

        // 删的号是当前的号就清空所有输入
        if (id_edit_->text() == base::UTF8ToUTF16(account_info->name))
        {
            SetLoginInfo(secret::LoginInfo());
        }
    }
}

void LoginInfoCollectionView::SetPSWEditObscured(bool obscured)
{
    if (obscured != pw_edit_->IsObscured())
    {
        pw_edit_->SetObscured(obscured);
        UpdateObscuredButtonImage();
    }
}

void LoginInfoCollectionView::SetTokenVerify(bool token_verify)
{
    is_token_verify_ = token_verify;
}

void LoginInfoCollectionView::UpdateDropdownButtonImage(bool dropdown_list_showwing)
{
    ids_button_->SetAllStateImage(
        GetImageSkiaNamed(dropdown_list_showwing ?
            IDR_LIVEMAIN_TABAREA_SCENE_UP : IDR_LIVEMAIN_TABAREA_SCENE_DOWN));
    ids_button_->SetTooltipText(dropdown_list_showwing ? L"收起列表" : L"展开历史登录账号列表");
    ids_button_->SchedulePaint();
}

void LoginInfoCollectionView::UpdateObscuredButtonImage()
{
    obscured_button_->SetImage(views::Button::STATE_NORMAL,
        GetImageSkiaNamed(pw_edit_->IsObscured() ?
            IDR_LIVEHIME_V3_SCENEITEM_SHOW : IDR_LIVEHIME_V3_SCENEITEM_HIDE));
    obscured_button_->SetImage(views::Button::STATE_HOVERED,
        GetImageSkiaNamed(pw_edit_->IsObscured() ?
            IDR_LIVEHIME_V3_SCENEITEM_SHOW_HV : IDR_LIVEHIME_V3_SCENEITEM_HIDE_HV));
    obscured_button_->SetImage(views::Button::STATE_PRESSED,
        GetImageSkiaNamed(pw_edit_->IsObscured() ?
            IDR_LIVEHIME_V3_SCENEITEM_SHOW_HV : IDR_LIVEHIME_V3_SCENEITEM_HIDE_HV));
    obscured_button_->SetTooltipText(pw_edit_->IsObscured() ? L"显示密码" : L"隐藏密码");
    obscured_button_->SchedulePaint();
}

void LoginInfoCollectionView::UncheckAllOption()
{
    pw_rem_check_->SetChecked(false);
    auto_login_check_->SetChecked(false);
}

void LoginInfoCollectionView::SetUserFace(int64 mid)
{
    if (0 != mid)
    {
        std::string content(std::move(livehime::GetAvatarBinaryDataByMid(mid)));
        user_pic_->SetImageBinary((const unsigned char*)content.data(), content.length());
    }
    else
    {
        user_pic_->SetImage(gfx::ImageSkia());
    }
}
