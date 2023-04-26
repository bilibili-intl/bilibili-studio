#include "bililive_user_info_modify.h"

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/base/resource/resource_bundle.h"
#include "grit/generated_resources.h"
#include "grit/theme_resources.h"

#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/controls/bililive_labelbutton.h"
#include "bililive/bililive/ui/views/controls/bililive_nativewidget.h"
#include "bililive/bililive/ui/views/controls/bililive_textfield.h"
#include "bililive/bililive/ui/views/controls/combox/bililive_combobox.h"

#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret_object.h"

namespace {

const wchar_t kMsgEmptyTitle[] = L"请输入房间名称";
const wchar_t kMsgEmptyArea[] = L"请选择分区";

}   // namespace

UserInfoModifyView::UserInfoModifyView() 
    : BililiveWidgetDelegate(*ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEMAIN_USER_INFO_MODIFY_ICON),
                                ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_USERINFO_MODIFY_DLG_TITLE))
    , cancel_btn_(nullptr)
    , title_label_(nullptr)
    , partition_label_(nullptr)
    , title_text_field_(nullptr)
    , partition_combox_(nullptr)
    , info_label_(nullptr)
{
    combox_server_index_ = 1;
    combox_local_server_index_.reserve(20);
}

UserInfoModifyView::~UserInfoModifyView()
{

}

void UserInfoModifyView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
    if (details.is_add && details.child == this) {
        InitViews();
        g_bililive_process->secret_object()->RequestRoomInfo(base::Bind(&UserInfoModifyView::OnRequestRoomInfo, AsWeakPtr()));
        g_bililive_process->secret_object()->RequestArealist(base::Bind(&UserInfoModifyView::OnRequestArealist, AsWeakPtr()));
    }
}

void UserInfoModifyView::OnRequestRoomInfo(RefDictionary value)
{
    scoped_ptr<base::DictionaryValue> local(RefCountedDictionary::RecursiveDeepCopy(value->value()));
    bool code = false;
    if (local->GetBoolean(g_netapi_code_tag, &code)) {
        if (code) {
            base::DictionaryValue* contents = NULL;
            if (local->GetDictionary(g_netapi_contents_tag, &contents)) {
                std::string title;
                contents->GetString("data.title", &title);
                title_text_field_->text_field()->SetText(base::UTF8ToWide(title));
                contents->GetInteger("data.area", &combox_server_index_);
                if (combox_local_server_index_.empty())
                {
                    partition_combox_->SetSelectedIndex(0);

                } else {
                    std::vector<int>::iterator  iter = std::find_if(combox_local_server_index_.begin(),
                        combox_local_server_index_.end(),
                        [this](int& i) { return i == combox_server_index_; });
                    if (iter == combox_local_server_index_.cend())
                    {
                        partition_combox_->SetSelectedIndex(0);
                    }
                    else {
                        partition_combox_->SetSelectedIndex(iter - combox_local_server_index_.begin());
                    }
                }
            }
        }
    }
    partition_combox_->SetSelectedIndex(-1);
}

void UserInfoModifyView::OnRequestArealist(RefDictionary value)
{
    scoped_ptr<base::DictionaryValue> local(RefCountedDictionary::RecursiveDeepCopy(value->value()));
    bool code = false;
    if (local->GetBoolean(g_netapi_code_tag, &code)) {
        if (code) {
            base::DictionaryValue* contents = nullptr;
            if (local->GetDictionary(g_netapi_contents_tag, &contents)) {
                base::ListValue* area = nullptr;
                contents->GetList("data", &area);

                for (base::ListValue::iterator iter = area->begin(); iter != area->end(); ++iter)
                {
                    int index;
                    static_cast<base::DictionaryValue*>(*iter)->GetInteger("id", &index);
                    base::string16 area;
                    static_cast<base::DictionaryValue*>(*iter)->GetString("name", &area);
                    int local_index = partition_combox_->AddItem(area);
                    combox_local_server_index_.push_back(index);
                }
            }
        }
    }
}

void UserInfoModifyView::InitViews()
{
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();

    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);
    layout->set_minimum_size(gfx::Size(420, 200));
    views::ColumnSet* column_set = layout->AddColumnSet(0);

    column_set->AddPaddingColumn(0, 20);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.75f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, 20);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.25f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, 20);

    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(0, 100);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.5, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, 40);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.5, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, 100);

    column_set = layout->AddColumnSet(2);
    column_set->AddPaddingColumn(0, 40);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, 40);

    layout->AddPaddingRow(1.0f, 0);
    layout->StartRow(0, 0);
    title_label_ = new BililiveLabel(rb.GetLocalizedString(IDS_USERINFO_MODIFY_TITLE));
    title_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    layout->AddView(title_label_);
    partition_label_ = new BililiveLabel(rb.GetLocalizedString(IDS_USERINFO_MODIFY_PARTITION));
    partition_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    layout->AddView(partition_label_);

    layout->AddPaddingRow(0, 10);
    layout->StartRow(0, 0);
    title_text_field_ = new BililiveTextfield();
    title_text_field_->set_placeholder_text(rb.GetLocalizedString(IDS_USERINFO_MODIFY_HINT));
    layout->AddView(title_text_field_);
    partition_combox_ = new BililiveCombobox();
    layout->AddView(partition_combox_);

    layout->AddPaddingRow(0, 15);
    layout->StartRow(0, 2);
    info_label_ = new views::Label(L"", gfx::Font("arial", 15).DeriveFont(0, gfx::Font::BOLD));
    layout->AddView(info_label_);
    info_label_->SetEnabledColor(SkColorSetARGBInline(255, 255, 0, 0));
    info_label_->SetAllowCharacterBreak(true);
    info_label_->SetMultiLine(true);
    info_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    layout->AddPaddingRow(1.0f, 0);

    layout->StartRow(0, 1);
    ok_btn_ = new BililiveLabelButton(this, rb.GetLocalizedString(IDS_USERINFO_MODIFY_OK));
    ok_btn_->SetIsDefault(true);
    ok_btn_->set_id(1003);
    layout->AddView(ok_btn_);
    cancel_btn_ = new BililiveLabelButton(this, rb.GetLocalizedString(IDS_USERINFO_MODIFY_CANCEL));
    cancel_btn_->set_id(1004);
    layout->AddView(cancel_btn_);
    layout->AddPaddingRow(0, 20);
}

void UserInfoModifyView::OnRequestUpdateRoominfoBack(RefDictionary value)
{
    scoped_ptr<base::DictionaryValue> local(RefCountedDictionary::RecursiveDeepCopy(value->value()));
    bool code = false;
    if (local->GetBoolean(g_netapi_code_tag, &code)) {
        if (code) {
            GetWidget()->Close();
        } else {
            base::string16 msg;
            if (local->GetString(g_netapi_msg_tag, &msg)) {
                info_label_->SetText(msg);
            }
        }
    }
}

void UserInfoModifyView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    int id = sender->id();
    switch (id) {
    case 1003:// 确认
        {
            base::string16 title = title_text_field_->text_field()->text();
            base::string16 sanitized_title;
            TrimString(title, L" \t", &sanitized_title);
            if (sanitized_title.empty()) {
                info_label_->SetText(kMsgEmptyTitle);
                return;
            }

            unsigned index = -1;
            if (!combox_local_server_index_.empty())
            {
                index = partition_combox_->selected_index();
            }

            if (index == -1)
            {
                info_label_->SetText(kMsgEmptyArea);
                return;
            }

            if (combox_local_server_index_.size() > index)
            {
                g_bililive_process->secret_object()->RequestUpdateRoominfo(
                    base::WideToUTF8(title), combox_local_server_index_[index],
                    base::Bind(&UserInfoModifyView::OnRequestUpdateRoominfoBack, AsWeakPtr()));
            }
        }
        break;
    case 1004:// 取消
        GetWidget()->Close();
        break;
    default:
        break;
    }
}

void UserInfoModifyView::ShowForm(views::Widget* parent)
{
    views::Widget* widget = new views::Widget();
    views::Widget::InitParams params(views::Widget::InitParams::TYPE_BUBBLE);
    params.has_dropshadow = true;
    if (parent) {
        params.parent = parent->GetNativeView();
    }
    params.native_widget = new BililiveNativeWidgetWin(widget);
    UserInfoModifyView::ShowViewModelOnWidget(new UserInfoModifyView, widget, params);
}
