#ifndef BILILIVE_SECRET_UI_VIEWS_LOGIN_BILILIVE_LOGIN_INFO_COLLENCTION_VIEW_H
#define BILILIVE_SECRET_UI_VIEWS_LOGIN_BILILIVE_LOGIN_INFO_COLLENCTION_VIEW_H

#include "ui/views/view.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/controls/textfield/textfield_controller.h"
#include "ui/views/focus/focus_manager.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/login/bilibili_login_control.h"
#include "bililive/secret/public/login_info.h"

class LivehimeCircleImageView;
class BililiveImageButton;

namespace
{
    class HistoryAccountListViewDelegate
    {
    public:
        virtual ~HistoryAccountListViewDelegate() = default;

        virtual void OnListViewDropdown(bool show) = 0;
        virtual void OnAccountItemSelected(const secret::LoginInfo& account) = 0;
        virtual void OnAccountItemDeleted(const secret::LoginInfo& account) = 0;
    };
}

class LoginInfoCollectionDelegate {
public:
    virtual ~LoginInfoCollectionDelegate() = default;

    virtual void OnInfoCollectionButtonPressed() = 0;
    virtual void OnInfoCollectionLinkClicked(int cid) = 0;
};

// 登录框主页，信息收集页
class LoginInfoCollectionView :
    public views::View,
    public views::ButtonListener,
    public BilibiliNativeEditController,
    HistoryAccountListViewDelegate
{
public:
    enum CheckBoxId {
        CheckBoxId_RemainPsw = 1,
        CheckBoxId_AutoLogin
    };
    enum LinkId {
        LinkId_Register = 1,
        LinkId_Lost,
    };
    enum ButtonId {
        ButtonId_Login = 1,
    };

    explicit LoginInfoCollectionView(LoginInfoCollectionDelegate *delegate);
    virtual ~LoginInfoCollectionView();

    void SetLoginInfo(const secret::LoginInfo& login_info);
    const std::unique_ptr<secret::LoginInfo>& account_info();

    void TokenExpired();
    void ClearPassword();
    bool is_token_verify() const { return is_token_verify_; }
    string16 id_text() const { return id_edit_->text(); }
    string16 pw_text() const { return pw_edit_->text(); }
    int64 mid() const;
    bool pw_remain() const { return pw_rem_check_->checked(); }
    bool auto_login() const { return auto_login_check_->checked(); }

protected:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // BilibiliNativeEditController
    void ContentsChanged(BilibiliNativeEditView* sender, const string16& new_contents) override;
    bool PreHandleMSG(BilibiliNativeEditView* sender, UINT msg, WPARAM wParam, LPARAM lParam) override;

    // HistoryAccountListViewDelegate
    void OnListViewDropdown(bool show) override;
    void OnAccountItemSelected(const secret::LoginInfo& account) override;
    void OnAccountItemDeleted(const secret::LoginInfo& account) override;

private:
    void InitViews();
    void InitData();
    void UninitViews();
    void RequestIdInputFocus();
    void SetTokenVerify(bool token_verify);
    void UpdateDropdownButtonImage(bool dropdown_list_showwing);
    void UpdateObscuredButtonImage();
    void UncheckAllOption();
    void SetUserFace(int64 mid);
    void DelAccountEndDialogHandler(int ret, void* data);
    void SetPSWEditObscured(bool obscured);

private:
    LoginInfoCollectionDelegate *delegate_ = nullptr;

    LivehimeCircleImageView* user_pic_ = nullptr;

    views::View *input_area_ = nullptr;
    LoginTextfield* id_edit_ = nullptr;
    BililiveImageButton* ids_button_ = nullptr;
    LivehimeLinkButton* account_register_link_ = nullptr;
    LoginTextfield* pw_edit_ = nullptr;
    BililiveImageButton* obscured_button_ = nullptr;
    LivehimeLinkButton* pw_lost_link_ = nullptr;

    LivehimeCheckbox* pw_rem_check_ = nullptr;
    LivehimeCheckbox* auto_login_check_ = nullptr;

    BilibiliLoginLabelButton* login_btn_ = nullptr;

    bool del_or_bs_pressing_ = false;
    std::vector<secret::LoginInfo> account_list_;
    std::unique_ptr<secret::LoginInfo> account_info_;

    bool is_token_verify_ = false;

    base::WeakPtrFactory<LoginInfoCollectionView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(LoginInfoCollectionView);
};

#endif //BILILIVE_SECRET_UI_VIEWS_LOGIN_BILILIVE_LOGIN_INFO_COLLENCTION_VIEW_H