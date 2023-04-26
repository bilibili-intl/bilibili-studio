#include "beauty_base_view.h"

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/livehime/sources_properties/source_camera_property_presenter_impl.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/blank_view.h"
#include "bililive/bililive/ui/views/controls/linear_layout.h"
#include "bililive/bililive/ui/views/controls/mesh_layout.h"
//#include "bililive/bililive/livehime/sources_properties/beauty_camera_controller.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/base_setup_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/beauty_base_item_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/beauty_tab_item_view.h"
#include "bililive/secret/public/event_tracking_service.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/box_layout.h"


enum { kMainTabId = -1 };

enum BeautyTabIndex
{
    //基础
    kBeauty_base_index = 0,

    //脸型
    kBeauty_face_shape_index = 1,

    //面部
    kBeauty_face_index = 2,

    //眼睛
    kBeauty_eye_index = 3,

    //鼻子
    kBeauty_nose_index = 4,

    //嘴巴
    kBeauty_mouth_index = 5,
};

BeautyBaseView::BeautyBaseView() :
    wpf_(this)
{
}

BeautyBaseView::~BeautyBaseView()
{
}

//void BeautyBaseView::UpdateIcon(int id, const gfx::ImageSkia& nor_icon, const gfx::ImageSkia& sel_icon)
//{
//    auto item_view = mp_beauty_item_[id];
//    if (item_view)
//    {
//        item_view->SetSelImage(sel_icon);
//        item_view->SetImage(nor_icon);
//    }
//    else
//    {
//        LOG(INFO) << "BeautyBaseView::UpdateIcon Find BeautyBaseItemView Failed. " << "id: " << id;
//        DCHECK(0);
//    }
//
//    SchedulePaint();
//}
//
//void BeautyBaseView::RefreshUi(const BeautyViewData& view_data)
//{
//    DCHECK(view_data.tag.meterial_type == kBeautyMaterialBase);
//
//    is_loading_ = true;
//
//    auto& rb = ResourceBundle::GetSharedInstance();
//
//    for (auto& tag_view_data : view_data.child_tag)
//    {
//        for (auto& md_view_data : tag_view_data.child_tag)
//        {
//            auto& id = md_view_data.tag.id;
//            if (mp_beauty_item_.find(id) !=
//                mp_beauty_item_.end())
//            {
//                auto item_view = mp_beauty_item_[id];
//
//                auto md = BeautyCameraController::GetInstance()->FindBeautyMaterial(item_view->BeautyTag());
//                if (md)
//                {
//                    if (!md->nor_icon.isNull() &&
//                        !md->sel_icon.isNull())
//                    {
//                        item_view->SetImage(md->nor_icon);
//                        item_view->SetSelImage(md->sel_icon);
//                    }
//                    else
//                    {
//                        item_view->SetImage(*rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_DEFAULT));
//                        item_view->SetSelImage(*rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_DEFAULT));
//                    }
//
//                    float percent = float(md->value.cur_value - md->value.min_value) / float(md->value.max_value - md->value.min_value);
//                    item_view->SetPercent(percent);
//                    item_view->SchedulePaint();
//                }
//            }
//        }
//    }
//
//    if (shrink_content_view_)
//    {
//        for (int i = 0; i < shrink_content_view_->child_count(); i++)
//        {
//            auto item_view = static_cast<BeautyBaseItemView*>(shrink_content_view_->child_at(i));
//            item_view->SetPercent(0);
//
//            if (item_view->BeautyTag().id == BeautyCameraController::GetInstance()->GetShrinkModel())
//            {
//                std::vector<BeautyMaterialValue> value;
//                if (BeautyCameraController::GetInstance()->GetBeautyMaterialValue(item_view->BeautyTag(), value) &&
//                    !value.empty())
//                {
//                    float percent = float(value[0].cur_value - value[0].min_value) / float(value[0].max_value - value[0].min_value);
//                    item_view->SetPercent(percent);
//                }
//
//                shrink_content_view_->Select(i);
//                shrink_content_view_->SchedulePaint();
//            }
//        }
//    }
//
//    is_loading_ = false;
//
//    UpdateSelect();
//
//    SchedulePaint();
//}
//
//void BeautyBaseView::ReloadUi(const BeautyViewData& view_data)
//{
//    DCHECK(view_data.tag.meterial_type == kBeautyMaterialBase);
//
//    is_loading_ = true;
//
//    auto& rb = ResourceBundle::GetSharedInstance();
//
//    beauty_tab_select_view_->RemoveAllChildViews(true);
//    tab_view_->RemoveAllChildViews(true);
//    mp_beauty_item_.clear();
//
//    tab_view_->set_id(kMainTabId);
//
//    for (const auto& tag_view_data : view_data.child_tag)
//    {
//        const auto& tag = tag_view_data.tag;
//        auto tag_view = new BeautyTabItemView(tab_view_, tag.name);
//        tag_view->BeautyTag() = tag;
//        tab_view_->AddTab(tag_view);
//
//        TabBarView* base_item_content_view = new TabBarView(false, this);
//        if (BeautyCameraController::GetInstance()->IsShrinkTag(tag.tag_id))
//        {
//            shrink_content_view_ = base_item_content_view;
//        }
//
//        base_item_content_view->EnableNoSelect(false);
//
//        base_item_content_view->set_id(tag.tag_id);
//        MeshLayout* mesh_layout = new MeshLayout();
//        base_item_content_view->SetLayoutManager(mesh_layout);
//        mesh_layout->SetUnitSize(GetSizeByDPIScale({ 64, 72 }));
//        mesh_layout->SetUnitInterval({ GetLengthByDPIScale(15), GetLengthByDPIScale(15), GetLengthByDPIScale(15), GetLengthByDPIScale(15) });
//        beauty_tab_select_view_->AddChildView(base_item_content_view);
//
//        for (auto& material : tag_view_data.child_tag)
//        {
//            auto item = new BeautyBaseItemView(base_item_content_view);
//            item->BeautyTag() = material.tag;
//            item->SetText(material.tag.name);
//            item->set_id(material.tag.id);
//            item->SetNew(BeautyCameraController::GetInstance()->IsNewBeautyItem(material.tag.id));
//
//            auto md = BeautyCameraController::GetInstance()->FindBeautyMaterial(material.tag);
//            if (md)
//            {
//                if (!md->nor_icon.isNull() &&
//                    !md->sel_icon.isNull())
//                {
//                    item->SetImage(md->nor_icon);
//                    item->SetSelImage(md->sel_icon);
//                }
//                else
//                {
//                    item->SetImage(*rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_DEFAULT));
//                    item->SetSelImage(*rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_DEFAULT));
//                }
//
//                item->SetSideRound(md->value.min_value < 0);
//                std::vector<BeautyMaterialValue> value;
//                if (BeautyCameraController::GetInstance()->GetBeautyMaterialValue(material.tag, value) &&
//                    !value.empty())
//                {
//                    float percent = float(value[0].cur_value - value[0].min_value) / float(value[0].max_value - value[0].min_value);
//                    item->SetPercent(percent);
//                }
//                else
//                {
//                    DCHECK(0);
//                }
//
//                base_item_content_view->AddChildView(item);
//                mp_beauty_item_[material.tag.id] = item;
//            }
//        }
//    }
//
//    if (shrink_content_view_)
//    {
//        for (int i = 0; i < shrink_content_view_->child_count(); i++)
//        {
//            auto item_view = static_cast<BeautyBaseItemView*>(shrink_content_view_->child_at(i));
//            item_view->SetPercent(0);
//
//            if (item_view->BeautyTag().id == BeautyCameraController::GetInstance()->GetShrinkModel())
//            {
//                std::vector<BeautyMaterialValue> value;
//                if (BeautyCameraController::GetInstance()->GetBeautyMaterialValue(item_view->BeautyTag(), value) &&
//                    !value.empty())
//                {
//                    float percent = float(value[0].cur_value - value[0].min_value) / float(value[0].max_value - value[0].min_value);
//                    item_view->SetPercent(percent);
//                }
//
//                shrink_content_view_->Select(i);
//            }
//        }
//    }
//
//    beauty_tab_select_view_->Layout();
//    tab_view_->Layout();
//
//    beauty_tab_select_view_->SchedulePaint();
//    tab_view_->SchedulePaint();
//
//    is_loading_ = false;
//}

bool BeautyBaseView::UpdateSelect(bool report)
{
    if (!this->visible())
    {
        return false;
    }

    int select = beauty_tab_select_view_->GetSelect();
    if (select >= 0 && select < beauty_tab_select_view_->child_count())
    {
        auto beauty_tab = static_cast<TabBarView*>(beauty_tab_select_view_->child_at(select));
        int index = beauty_tab->GetSelect();
        if (index >= 0 && index < beauty_tab->child_count())
        {
            auto child_view = beauty_tab->child_at(index);
            //BeautyCameraController::GetInstance()->BeautyBaseItemSelect(static_cast<BeautyBaseItemView*>(child_view), report);
        }
        else
        {
            //BeautyCameraController::GetInstance()->NotifyBeautyItemNoSelct();
        }
    }
    else
    {
        //BeautyCameraController::GetInstance()->NotifyBeautyItemNoSelct();
    }

    ClearShrinkModeValue();
    return true;
}

void BeautyBaseView::InitView()
{
    auto& rb = ResourceBundle::GetSharedInstance();
    mp_beauty_item_.clear();

    views::GridLayout* layout = new views::GridLayout(this);
    tab_view_ = new TabBarView(true, this);
    tab_view_->set_id(kMainTabId);

    beauty_tab_select_view_ = new TabSelectView();
    //auto base_data = BeautyCameraController::GetInstance()->GetBeautyViewData(kBeautyMaterialBase);
    //if (base_data)
    //{
    //    ReloadUi(*base_data);
    //}

    //reset_btn_ = new BeautyCameraHoverButton(this);
    //reset_btn_->SetHoverText(rb.GetLocalizedString(IDS_CAMERA_BEAUTY_RESET));
    //reset_btn_->SetPreferredSize(GetSizeByDPIScale({32, 32}));
    //reset_btn_->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_RESET));
    //reset_btn_->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_RESET_ACTIVE));
    //reset_btn_->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_RESET_ACTIVE));

    //beauty_btn_ = new BeautyCameraHoverButton(this);
    //beauty_btn_->SetHoverText(rb.GetLocalizedString(IDS_SRCPROP_CAMERA_AUTO_BEAUTY_ON));
    //beauty_btn_->SetPreferredSize(GetSizeByDPIScale({ 32, 32 }));
    //beauty_btn_->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_BEAUTY));
    //beauty_btn_->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_BEAUTY_ACTIVE));
    //beauty_btn_->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_BEAUTY_ACTIVE));

    views::View* tab_subview = new views::View();
    bililive::LinearLayout* tab_layout = new  bililive::LinearLayout(bililive::LinearLayout::kHoirzontal, tab_subview);
    tab_subview->SetLayoutManager(tab_layout);
    tab_layout->AddView(tab_view_, bililive::LinearLayout::kLeading, bililive::LinearLayout::kCenter);
    //tab_layout->AddView(beauty_btn_, bililive::LinearLayout::kTailing, bililive::LinearLayout::kCenter);
    //tab_layout->AddView(reset_btn_, bililive::LinearLayout::kTailing, bililive::LinearLayout::kCenter, GetLengthByDPIScale(20));

    SetLayoutManager(layout);
    auto col_set = layout->AddColumnSet(0);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);

    layout->StartRow(0.0, 0);
    layout->AddView(tab_subview);

    layout->StartRow(0.0, 0);
    layout->AddView(beauty_tab_select_view_);

    is_loading_ = true;
    tab_view_->Select(0);
    is_loading_ = false;

    reset_handler_.closure = base::Bind(&BeautyBaseView::MessageBoxCallback, wpf_.GetWeakPtr());

    //static HCURSOR hand_cursor = LoadCursor(nullptr, IDC_HAND);
}

void BeautyBaseView::InitData()
{
}

bool BeautyBaseView::CheckSetupValid()
{
    return true;
}

bool BeautyBaseView::SaveSetupChange() {


    return true;
}

//bool BeautyBaseView::Cancel()
//{
//    return true;
//}

void BeautyBaseView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    //auto& rb = ResourceBundle::GetSharedInstance();

    //if (sender == beauty_btn_)
    //{
    //    //BeautyCameraController::GetInstance()->NotifyAutoBeauty();
    //}
    //else if (sender == reset_btn_)
    //{
    //    //if (BeautyCameraController::GetInstance()->IsBaseClear())
    //    //{
    //    //    return;
    //    //}

    //    livehime::ShowMessageBox(GetWidget()->GetNativeView(), rb.GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_TITLE), rb.GetLocalizedString(IDS_CAMERA_BEAUTY_TIP3), rb.GetLocalizedString(IDS_SURE) + L"," + rb.GetLocalizedString(IDS_CANCEL), &reset_handler_);
    //}
}

void BeautyBaseView::MessageBoxCallback(const base::string16& button_text, void* data)
{
    auto& rb = ResourceBundle::GetSharedInstance();

    if (button_text == rb.GetLocalizedString(IDS_SURE))
    {
        //BeautyCameraController::GetInstance()->NotifyClearBeauty();
    }
}

void BeautyBaseView::ClearShrinkModeValue()
{
    BeautyBaseItemView* item_view{nullptr};

    //auto used_id = BeautyCameraController::GetInstance()->GetShrinkModel();

    //auto it = mp_beauty_item_.find(BeautyCameraController::NONE_FACE_SHAPE);
    //if (it != mp_beauty_item_.end() && used_id != BeautyCameraController::NONE_FACE_SHAPE)
    //{
    //    item_view = it->second;
    //    item_view->SetPercent(0);
    //    item_view->SchedulePaint();
    //}

    //it = mp_beauty_item_.find(BeautyCameraController::NATURE_FACE_SHAPE);
    //if (it != mp_beauty_item_.end() && used_id != BeautyCameraController::NATURE_FACE_SHAPE)
    //{
    //    item_view = it->second;
    //    item_view->SetPercent(0);
    //    item_view->SchedulePaint();
    //}

    //it = mp_beauty_item_.find(BeautyCameraController::GODDESS_FACE_SHAPE);
    //if (it != mp_beauty_item_.end() && used_id != BeautyCameraController::GODDESS_FACE_SHAPE)
    //{
    //    item_view = it->second;
    //    item_view->SetPercent(0);
    //    item_view->SchedulePaint();
    //}

    //it = mp_beauty_item_.find(BeautyCameraController::ROUND_FACE_SHAPE);
    //if (it != mp_beauty_item_.end() && used_id != BeautyCameraController::ROUND_FACE_SHAPE)
    //{
    //    item_view = it->second;
    //    item_view->SetPercent(0);
    //    item_view->SchedulePaint();
    //}

    //it = mp_beauty_item_.find(BeautyCameraController::LONG_FACE_SHAPE);
    //if (it != mp_beauty_item_.end() && used_id != BeautyCameraController::LONG_FACE_SHAPE)
    //{
    //    item_view = it->second;
    //    item_view->SetPercent(0);
    //    item_view->SchedulePaint();
    //}
}

bool BeautyBaseView::TabItemSelect(int index, TabBarView* tab_view)
{
    //由美颜 tab item 点击发送
    if (tab_view->id() == kMainTabId &&
        (index >= kBeauty_base_index && index <= kBeauty_mouth_index))
    {
        beauty_tab_select_view_->SelectView(index);

        int tab_index = tab_view_->GetSelect();
        if (tab_index >= 0 && index < tab_view_->child_count())
        {
            BeautyTabItemView* tab_item = static_cast<BeautyTabItemView*>(tab_view_->child_at(tab_index));
            //BeautyCameraController::GetInstance()->ReportScendaryTabClick(L"基础美颜", tab_item->GetTagText());
        }

        UpdateSelect(false);
        return true;
    }

    if (is_loading_)
    {
        return true;
    }

    UpdateSelect();
    return true;
}

