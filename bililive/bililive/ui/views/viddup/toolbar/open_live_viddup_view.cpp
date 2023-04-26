#include "open_live_viddup_view.h"
#include <shellapi.h>

#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"

#include "ui/base/resource/resource_bundle.h"

#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"

#include "bililive/bililive/utils/fast_forward_url_convert.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"
#include "base/strings/utf_string_conversions.h"


namespace livehime {
    struct LanguageArea
    {
        LANGID langid;
        int showLocalStr;
        std::string kOpenLiveRoomWebUrl;
    };

    const LanguageArea kOpenLiveRoom[] =
    {
        {0x0421, IDS_OPEN_LIVE_FAILED_APPLY_INDONESIA, "https://forms.gle/HKLpBUxAncvxc1Eb6"},// 若为印尼，即跳转至 Web 在线报名表格；(URL: https://forms.gle/HKLpBUxAncvxc1Eb6)
        {0x041e, IDS_OPEN_LIVE_FAILED_APPLY_THAI, "https://forms.gle/i7UDFktKSR8ovRz97"},// 若为泰国，即跳转至 Web 在线报名表格；(URL: https://forms.gle/i7UDFktKSR8ovRz97)
    };

    void ShowOpenLiveDialog(gfx::NativeView parent_widget)
    {
        OpenLiveViddupView* msgbox = new OpenLiveViddupView();
        views::Widget* widget_ = new views::Widget();
        BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(widget_);
        native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_CLOSE);
        views::Widget::InitParams params;
        params.native_widget = native_widget;
        params.parent = parent_widget;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        BililiveWidgetDelegate::DoModalWidget(msgbox, widget_, params);
    }

    OpenLiveViddupView::OpenLiveViddupView()
        : BililiveWidgetDelegate(
            gfx::ImageSkia(),
            ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_OPEN_LIVE_FAILED))
        , combobox_(nullptr)
    {
    }

    void OpenLiveViddupView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) {
        if (details.child == this) {
            if (details.is_add) {
                InitViews();
                InitViewsData();
            }
        }
    }

    void OpenLiveViddupView::InitViews() {
        ResourceBundle& rb = ResourceBundle::GetSharedInstance();

        views::GridLayout* layout = new views::GridLayout(this);
        SetLayoutManager(layout);

        views::ColumnSet* column_set = layout->AddColumnSet(0);
        column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 1.0f, views::GridLayout::FIXED, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
        column_set = layout->AddColumnSet(1);
        column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 1.0f, views::GridLayout::FIXED, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

        // 最少保持足够显示三个button的宽度
        std::vector<base::string16> btns;
        base::SplitStringDontTrim(GetLocalizedString(IDS_CANCEL) + L"," + GetLocalizedString(IDS_REALNAME_AUTH), ',', &btns);
        int btn_num = std::max(3, static_cast<int>(btns.size()));
        int cx = (LivehimeFunctionLabelButton::GetButtonSize().width() + kPaddingColWidthForActionButton) * btn_num - kPaddingColWidthForActionButton;
        column_set = layout->AddColumnSet(2);
        column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
        column_set->AddPaddingColumn(1.0f, 0);
        column_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, cx, cx);
        column_set->AddPaddingColumn(1.0f, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

        label_ = new LivehimeContentLabel(GetLocalizedString(IDS_OPEN_LIVE_FAILED_MESSAGE));
        label_->SetVerticalAlignment(gfx::ALIGN_VCENTER);
        label_->SetHorizontalAlignment(gfx::ALIGN_CENTER);
        label_->SetAllowCharacterBreak(true);
        label_->SetMultiLine(true);

        layout->AddPaddingRow(0, kPaddingRowHeightEnds);

        layout->StartRow(1.0f, 0);
        layout->AddView(label_);
        layout->AddPaddingRow(0, kPaddingRowHeightEnds);

        combobox_ = new LivehimeCombobox(false);
        combobox_->set_listener(this);
        layout->StartRow(0, 1);
        layout->AddView(combobox_);
        layout->AddPaddingRow(0, kPaddingRowHeightEnds);

        layout->StartRow(0, 2);
        buttons_bar_ = new views::View();
        {
            if (button_padding_ == 0) {
                button_padding_ = kPaddingColWidthForActionButton;
            }
            views::BoxLayout* box_layout = new views::BoxLayout(views::BoxLayout::Orientation::kHorizontal, 0, 0, button_padding_);
            buttons_bar_->SetLayoutManager(box_layout);

            // 有指定默认ok按钮的就设置指定按钮，没有的就将第一个设为默认ok按钮
            bool is_set_def = false;
            for each (const base::string16 & var in btns)
            {
                bool is_def = var == GetLocalizedString(IDS_CANCEL);
                LivehimeActionLabelButton* button = new LivehimeActionLabelButton(this, var, is_def);
                button->SetIsDefault(is_def);
                if (is_def) {
                    button->SetStyle(BililiveLabelButton::STYLE_INTL_WHITETEXT_GRAYBK);
                }
                else {
                    button->SetStyle(BililiveLabelButton::STYLE_INTL_WHITETEXT_BLUEBK);

                }
                buttons_bar_->AddChildView(button);
            }
        }

      
        layout->AddView(buttons_bar_);
        layout->AddPaddingRow(0, kPaddingRowHeightEnds);
       
    }

    void OpenLiveViddupView::InitViewsData() {
        LANGID langid = GetUserDefaultUILanguage();
        int index = 0;
        for (size_t i = 0; i < arraysize(kOpenLiveRoom); i++)
        {
            if (langid == kOpenLiveRoom[i].langid) index = i;
            combobox_->AddItem(GetLocalizedString(kOpenLiveRoom[i].showLocalStr), std::string());
        }
        combobox_->SetSelectedIndex(index);
        url_ = kOpenLiveRoom[index].kOpenLiveRoomWebUrl;
    }

    void OpenLiveViddupView::ButtonPressed(views::Button* sender, const ui::Event& event) {
        if (((LivehimeActionLabelButton*)sender)->GetText() == GetLocalizedString(IDS_CANCEL) || ((LivehimeActionLabelButton*)sender)->GetText().empty()) 
        {
            GetWidget()->Close();
            return;
        }
        ShellExecuteW(nullptr, L"open", bililive::FastForwardChangeEnv(base::UTF8ToUTF16(url_)).c_str(), nullptr, nullptr, SW_SHOW);
        GetWidget()->Close();
    }

    void OpenLiveViddupView::OnSelectedIndexChanged(BililiveComboboxEx* combobox) {
        if (combobox_ == combobox) {
            auto index = combobox_->selected_index();
            if (index >= 0 && index < combobox_->GetItemCount()) {
                url_ = kOpenLiveRoom[index].kOpenLiveRoomWebUrl;
            }
        }
    }
}