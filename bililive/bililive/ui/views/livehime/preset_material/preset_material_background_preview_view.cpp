#include "preset_material_background_preview_view.h"

#include "ui/base/resource/resource_bundle.h"

#include "ui/views/layout/fill_layout.h"

PresetMaterialBackgroundPreviewView::PresetMaterialBackgroundPreviewView(PresetMaterialBackgroundPreviewObserver* observer, bool is_horiz, bool is_vtuber_preset) :
    observer_(observer),
    is_horiz_(is_horiz),
    is_vtuber_preset_(is_vtuber_preset)
{
}

PresetMaterialBackgroundPreviewView::~PresetMaterialBackgroundPreviewView()
{
}

void PresetMaterialBackgroundPreviewView::Layout()
{
    gfx::Size btn_size = GetBtnSize(is_vtuber_preset_);
    float span = GetBtnSpan(is_vtuber_preset_);

    int x{}, y{};
    int i{};
    for (auto btn : vec_bg_checkbox)
    {
        x = (i % 3) * (btn_size.width() + span);
        y = (i / 3) * (btn_size.height() + GetLengthByDPIScale(10));

        btn->SetBounds(x, y, btn_size.width(), btn_size.height());

        i++;
    }
}

gfx::Size PresetMaterialBackgroundPreviewView::GetPreferredSize()
{
    gfx::Size btn_size = GetBtnSize(is_vtuber_preset_);
    int span = GetBtnSpan(is_vtuber_preset_);
    int width = btn_size.width() * 3 + span * 2;

    int line_count = static_cast<int>(vec_bg_checkbox.size()) / 3;
    if (line_count <= 0)
    {
        return { width, btn_size.height() };
    }
    else
    {
        int heigt = line_count * btn_size.height() + (line_count - 1) * GetLengthByDPIScale(10);
        return { width, heigt };
    }
}

//gfx::Size PresetMaterialBackgroundPreviewView::GetMinimumSize()
//{
//    return GetSizeByDPIScale({400, 206});
//}

bool PresetMaterialBackgroundPreviewView::SelectCheckBox(int index)
{
    if (index < 0 || index >= static_cast<int>(vec_bg_checkbox.size()))
    {
        return false;
    }

    auto& btn = vec_bg_checkbox[index];
    btn->SetChecked(true);
    SelectCheckBoxInternal(btn->id());

    return true;
}

BkImageInfo PresetMaterialBackgroundPreviewView::GetSelectedBkImageInfo()
{
    auto it = std::find_if(vec_bg_checkbox.begin(), vec_bg_checkbox.end(),
        [](const CustomStyleRadioButton* btn)
        {
            return btn->checked();
        });

    if (it != vec_bg_checkbox.end())
    {
        auto btn = *it;
        return PresetMaterailUIPresenter::GetInstance()->FetchBgInfo(is_horiz_, btn->id());
    }
    else
    {
        return BkImageInfo();
    }
}

void PresetMaterialBackgroundPreviewView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add == true)
        {
            PresetMaterailUIPresenter::GetInstance()->AddObserver(this);
            InitView();
        }
        else
        {
            PresetMaterailUIPresenter::GetInstance()->RemoveObserver(this);
        }
    }
}

void PresetMaterialBackgroundPreviewView::OnPresetMaterialInitialized()
{
    LoadBkInfo(group_id_);
    Layout();

    if (observer_)
    {
        observer_->OnBkImageInitialized();
    }
}

void PresetMaterialBackgroundPreviewView::OnPresetMaterialBkUpdate(int index, const BkImageInfo& info)
{
    if (index < 0)
    {
        return;
    }

    bool need_sort{ false };
    if (index >= static_cast<int>(vec_bg_checkbox.size()) ||
        vec_bg_checkbox[index]->id() != index)
    {
        auto btn = CreateBkButton(group_id_, GetBackgroundText(index));
        btn->set_id(index);
        btn->SetAllStateImage(info.bk_img);
        AddChildView(btn);
        vec_bg_checkbox.push_back(btn);
        need_sort = true;
    }
    else
    {
        vec_bg_checkbox[index]->SetAllStateImage(info.bk_img);
    }

    if (need_sort)
    {
         std::sort(vec_bg_checkbox.begin(), vec_bg_checkbox.end(),
                [](CustomStyleRadioButton* rhd, CustomStyleRadioButton* lhd)
                {
                    return rhd->id() < lhd->id();
                });

         Layout();
    }
}

void PresetMaterialBackgroundPreviewView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    int id = sender->id();
    if (id != -1)
    {
        SelectCheckBoxInternal(id);
    }
    else
    {
        DCHECK(0);
    }
}

void PresetMaterialBackgroundPreviewView::InitView()
{
    group_id_ = LivehimeFilterRadioButton::new_groupid();
    if (PresetMaterailUIPresenter::GetInstance()->IsInitialized())
    {
        LoadBkInfo(group_id_);
        if (observer_)
        {
            observer_->OnBkImageInitialized();
        }
    }
    else
    {
        int i{};
        for (int i = 0; i < 6; i++)
        {
            auto btn = CreateBkButton(group_id_, GetBackgroundText(i + 1));
            //id -1 代表没有对应的背景资源
            btn->set_id(-1);
            AddChildView(btn);
            vec_bg_checkbox.push_back(btn);
        }
    }

    auto size = GetPreferredSize();
    SetBounds(0, 0, size.width(), size.height());
}

void PresetMaterialBackgroundPreviewView::LoadBkInfo(int group_id)
{
    this->RemoveAllChildViews(true);
    vec_bg_checkbox.clear();

    auto filter_button_groupid = LivehimeFilterRadioButton::new_groupid();
    int bk_count = PresetMaterailUIPresenter::GetInstance()->GetBgInfoCount(is_horiz_);
    for (int i = 0; i < bk_count; i++)
    {
        auto bk_info = PresetMaterailUIPresenter::GetInstance()->FetchBgInfo(is_horiz_, i);
        if (!bk_info.bk_img.isNull())
        {
            auto btn = CreateBkButton(filter_button_groupid, GetBackgroundText(i + 1));
            btn->SetAllStateImage(bk_info.bk_img);
            btn->set_id(i);
            AddChildView(btn);
            vec_bg_checkbox.push_back(btn);
        }
    }

    if (!vec_bg_checkbox.empty())
    {
        vec_bg_checkbox[0]->SetChecked(true);
    }
}

void PresetMaterialBackgroundPreviewView::SelectCheckBoxInternal(int index)
{
    auto bk_info = PresetMaterailUIPresenter::GetInstance()->FetchBgInfo(is_horiz_, index);
    if (!bk_info.bk_img.isNull())
    {
        if (observer_)
        {
            observer_->OnBkImageSelected(bk_info);
        }
    }
    else
    {
        DCHECK(0);
    }
}

base::string16 PresetMaterialBackgroundPreviewView::GetBackgroundText(int index)
{
    std::wstringstream str_stream;

    str_stream << GetLocalizedString(IDS_PRESET_MATERIAL_BACKGROUND_PREVIEW_VIEW_BACKGROUND_NAME);
    str_stream << index;

    return str_stream.str();
}

gfx::Size PresetMaterialBackgroundPreviewView::GetBtnSize(bool is_vtuber_preset)
{
    if (is_vtuber_preset)
    {
        return GetSizeByDPIScale({ 188, 132 });
    }
    else
    {
        return GetSizeByDPIScale({ 128, 98 });
    }
}

gfx::Size PresetMaterialBackgroundPreviewView::GetBtnImageSize(bool is_vtuber_preset)
{
    if (is_vtuber_preset)
    {
        return GetSizeByDPIScale({188, 106});
    }
    else
    {
        return GetSizeByDPIScale({ 128, 72 });
    }
}

float PresetMaterialBackgroundPreviewView::GetBtnSpan(bool is_vtuber_preset)
{
    if(is_vtuber_preset)
    {
        return GetLengthByDPIScale(10);
    }
    else
    {
        return GetLengthByDPIScale(8);
    }
}

CustomStyleRadioButton* PresetMaterialBackgroundPreviewView::CreateBkButton(int grounp_id, const base::string16& text)
{
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();

    auto btn = new CustomStyleRadioButton(this, grounp_id, text, ftFourteen);
    btn->SetGroup(grounp_id);
    btn->set_id(-1);
    btn->SetPreferredSize(GetBtnSize(is_vtuber_preset_));
    btn->SetAllStateImageSize(GetBtnImageSize(is_vtuber_preset_));
    btn->SetAllStateTextColor(GetColor(LabelTitle));

    btn->SetAllStateImage(*rb.GetImageSkiaNamed(IDR_PRESET_MATERIAL_DEFAULT_BK));
    btn->SetAllStateImageAlign(CustomStyleRadioButton::ImageAlign::kTop);
    btn->SetAllStateImagePadding(GetLengthByDPIScale(6));
    btn->SetImageBorder(true);

    btn->SetCheckedBorder(true);
    btn->SetCheckBorderRoundRatio(GetLengthByDPIScale(4.0f));
    btn->SetCheckedBorderWidth(GetLengthByDPIScale(2.0f));
    btn->SetCheckedBorderColor(SkColorSetRGB(14, 190, 255));
    btn->SetCheckedTextColor(SkColorSetRGB(14, 190, 255));

    return btn;
}



