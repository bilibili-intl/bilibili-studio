#include "beauty_camera_control_preview_view.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"

enum { kContrastBtn = 1 };

class BeautySlideView :
    public views::View
{
public:
    BeautySlideView()
    {

    }

    ~BeautySlideView()
    {

    }

    //void SetProgressData(const BeautyClassTag& tag, const std::vector<BeautyMaterialValue>& vec_vector)
    //{
    //    auto& rb = ResourceBundle::GetSharedInstance();

    //    tag_ = tag;

    //    process_view_data_.resize(vec_vector.size());
    //    for (int i = 0; i < static_cast<int>(vec_vector.size()); i++)
    //    {
    //        process_view_data_[i].value = vec_vector[i];
    //        process_view_data_[i].label = tag_.name;
    //    }

    //    hit_pt_ = gfx::Point();
    //    silder_hit_ = -1;

    //    //一键美妆有2个进度条
    //    if (tag_.meterial_type == kBeautyMaterialStyleMakeUp)
    //    {
    //        process_view_data_[0].label = rb.GetLocalizedString(IDS_SRCPROP_CAMERA_COLOR_FILTER);
    //        process_view_data_[1].label = rb.GetLocalizedString(IDS_CAMERA_BEAUTY_MAKE_UP_2);
    //    }

    //    if (tag_.meterial_type == kBeautyMaterialBase &&
    //        BeautyCameraController::GetInstance()->IsShrinkModel(tag_.id))
    //    {
    //        int m_value = process_view_data_[0].value.cur_value;
    //        BeautyCameraController::GetInstance()->NotifyPreviewSildeChanged(tag_, m_value, 0);
    //    }

    //    if (parent())
    //    {
    //        parent()->Layout();
    //    }
    //}

    void Layout() override
    {
        CalculateRect();
    }

    gfx::Size GetPreferredSize() override
    {
        int width = GetLengthByDPIScale(352);
        int height = GetLengthByDPIScale(24) * process_view_data_.size();
        return { width,height };
    }

protected:
    void OnPaint(gfx::Canvas* canvas) override
    {
        auto rt = GetContentsBounds();

        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(SkColorSetARGB(255, 0, 0, 0));
        paint.setAlpha(102);
        paint.setAntiAlias(true);
        canvas->DrawRoundRect(rt, GetLengthByDPIScale(8), paint);

        int x = rt.x();
        int y = 0;
        for (int i = 0; i < static_cast<int>(process_view_data_.size()); i++)
        {
            //DrawProcessBar(canvas, process_view_data_[i]);
            y += process_height_;
        }
    }

    bool BeautySlideView::OnMousePressed(const ui::MouseEvent& event)
    {
        float pos_precent{};
        silder_hit_ = SliderHitTest(event.location(), &pos_precent);
        if (silder_hit_ != -1)
        {
            hit_pt_ = event.location();
            SetProcessBarPos(silder_hit_, pos_precent);
            SchedulePaint();
        }
        else
        {
            hit_pt_ = gfx::Point();
        }
        return true;
    }

    //bool BeautySlideView::OnMouseDragged(const ui::MouseEvent& event)
    //{
    //    if (silder_hit_ != -1)
    //    {
    //        if (silder_hit_ >= 0 && silder_hit_ < static_cast<int>(process_view_data_.size()))
    //        {
    //            auto& data = process_view_data_[silder_hit_];
    //            int pt_length = event.location().x() - data.slider_rt.x();
    //            float diff_process = static_cast<float>(pt_length) / static_cast<float>(data.slider_rt.width());
    //            data.value.cur_value = (diff_process * (data.value.max_value - data.value.min_value)) + data.value.min_value;

    //            auto& v = data.value.cur_value;
    //            v = std::min(std::max(v, data.value.min_value), data.value.max_value);

    //            switch (process_view_data_.size())
    //            {
    //                case 1:
    //                {
    //                    int m_value = process_view_data_[0].value.cur_value;
    //                    //BeautyCameraController::GetInstance()->NotifyPreviewSildeChanged(tag_, m_value, 0);
    //                    break;
    //                }

    //                case 2:
    //                {
    //                    int f_value = process_view_data_[0].value.cur_value;
    //                    int m_value = process_view_data_[1].value.cur_value;
    //                    //BeautyCameraController::GetInstance()->NotifyPreviewSildeChanged(tag_, m_value, f_value);
    //                    break;
    //                }

    //                default:
    //                    DCHECK(0);
    //            }

    //            SchedulePaint();
    //        }
    //        else
    //        {
    //            DCHECK(0);
    //        }
    //    }

    //    return true;
    //}

    void BeautySlideView::OnMouseReleased(const ui::MouseEvent& event)
    {
        hit_pt_ = gfx::Point();
        silder_hit_ = -1;
    }

private:
    struct ProcessBarViewData
    {
        gfx::Rect               text_rt;
        gfx::Rect               slider_rt;
        gfx::Rect               num_rt;
        base::string16          label;
        //BeautyMaterialValue     value;
    };

    //void DrawProcessBar(gfx::Canvas* canvas, const ProcessBarViewData& process_view_data)
    //{
    //    const auto& process_data = process_view_data.value;

    //    auto content_rt = GetContentsBounds();

    //    SkPaint paint;
    //    paint.setAntiAlias(true);
    //    paint.setStyle(SkPaint::kFill_Style);

    //    //文字
    //    int text_width = ftFourteen.GetStringWidth(process_view_data.label);
    //    int text_x = (process_view_data.text_rt.width() - text_width) / 2.0f;

    //    gfx::Rect text_rt = process_view_data.text_rt;
    //    text_rt.set_width(text_width);
    //    text_rt.set_x(text_x);
    //    canvas->DrawStringInt(process_view_data.label, ftFourteen, SkColorSetRGB(255, 255, 255), text_rt);

    //    //滑动条整体
    //    const gfx::Rect& slider_rt = process_view_data.slider_rt;
    //    paint.setColor(SkColorSetARGB(102, 255, 255, 255));
    //    canvas->DrawRoundRect(slider_rt, GetLengthByDPIScale(4), paint);

    //    //滑动条当前进度
    //    auto cur_slider_rt = slider_rt;
    //    cur_slider_rt.set_width(GetPrecent(process_data.min_value, process_data.max_value, process_data.cur_value) * slider_rt.width());
    //    paint.setColor(SkColorSetARGB(255, 14, 190, 255));
    //    canvas->DrawRoundRect(cur_slider_rt, GetLengthByDPIScale(4), paint);

    //    //滑动条默认值
    //    gfx::Point default_pt
    //    {
    //        slider_rt.x() + int(GetPrecent(process_data.min_value, process_data.max_value, process_data.best_value) * slider_rt.width()),
    //        slider_rt.y() + int(slider_rt.height() / 2.0f)
    //    };
    //    paint.setColor(SkColorSetARGB(255, 255, 255, 255));
    //    canvas->DrawCircle(default_pt, GetLengthByDPIScale(3), paint);

    //    //滑动条当前位置圆点
    //    gfx::Point cur_pt
    //    {
    //        slider_rt.x() + int(GetPrecent(process_data.min_value, process_data.max_value, process_data.cur_value) * slider_rt.width()),
    //        slider_rt.y() + int(slider_rt.height() / 2.0f)
    //    };
    //    paint.setColor(SkColorSetARGB(255, 14, 190, 255));
    //    canvas->DrawCircle(cur_pt, GetLengthByDPIScale(6), paint);
    //    paint.setColor(SkColorSetARGB(255, 255, 255, 255));
    //    canvas->DrawCircle(cur_pt, GetLengthByDPIScale(4), paint);

    //    //数字
    //    gfx::Rect num_rt = process_view_data.num_rt;
    //    num_rt.set_x(process_view_data.num_rt.x() + GetLengthByDPIScale(6));
    //    canvas->DrawStringInt(std::to_wstring(int(process_data.cur_value)), ftFourteen, SkColorSetRGB(255, 255, 255), num_rt);
    //}

    void CalculateRect()
    {
        auto rt = GetContentsBounds();
        int x = rt.x();
        int y = rt.y();
        for (int i = 0; i < static_cast<int>(process_view_data_.size()); i++)
        {
            auto text_rt = gfx::Rect(x, y, rt.width() * 0.2, process_height_);
            auto slider_rt = gfx::Rect(text_rt.right(), y + (process_height_ - slider_height_) / 2.0f, rt.width() * 0.68181f, slider_height_);
            auto num_rt = gfx::Rect(slider_rt.right(), y, rt.width() * 0.11352f, process_height_);

            process_view_data_[i].text_rt = text_rt;
            process_view_data_[i].slider_rt = slider_rt;
            process_view_data_[i].num_rt = num_rt;

            y += process_height_;
        }
    }

    static float GetPrecent(float min, float max, float value)
    {
        return float(value - min) / float(max - min);
    }

    //static gfx::Rect GetSliderCurPt(gfx::Rect rt, const BeautyMaterialValue& data, int radius)
    //{
    //    return gfx::Rect
    //    {
    //        rt.x() + int(GetPrecent(data.min_value, data.max_value, data.cur_value) * rt.width()) - radius,
    //        rt.y() + int(rt.height() / 2.0f) - radius,
    //        radius * 2,
    //        radius * 2
    //    };
    //}

    int SliderHitTest(const gfx::Point& pt, float* pos_precent)
    {
        for (int i = 0; i < static_cast<int>(process_view_data_.size()); i++)
        {
            auto& process_view_data = process_view_data_[i];
            auto rt = process_view_data.slider_rt;
            rt.Inset(GetLengthByDPIScale(-2), GetLengthByDPIScale (-2)); //增加判断范围

            if (rt.Contains(pt))
            {
                gfx::Point pt_align = {
                    pt.x() - process_view_data.slider_rt.x(),
                    pt.y() - process_view_data.slider_rt.y(),
                };

                if (pos_precent)
                {
                    *pos_precent = static_cast<float>(pt_align.x()) / static_cast<float>(process_view_data.slider_rt.width());
                }

                return i;
            }
        }

        return -1;
    }

    void SetProcessBarPos(int index, float precent)
    {
        if (index < 0 || index >= (int)process_view_data_.size())
        {
            return;
        }

        auto& process_data = process_view_data_[index];

        //process_data.value.cur_value = (process_data.value.max_value - process_data.value.min_value) * precent;
        //process_data.value.cur_value += process_data.value.min_value;
        //auto cur_value = process_data.value.cur_value;
        //auto min_value = process_data.value.min_value;
        //auto max_value = process_data.value.max_value;
        //process_data.value.cur_value = std::max(std::min(cur_value, max_value), min_value);
    }

    //BeautyClassTag                      tag_;
    std::vector<ProcessBarViewData>     process_view_data_;
    const int                           slider_height_ = GetLengthByDPIScale(6);
    const int                           process_height_ = GetLengthByDPIScale(24);
    gfx::Point                          hit_pt_;
    int                                 silder_hit_ = -1;
};

class PreviewViewButton :
    public BililiveImageButton
{
public:
    explicit PreviewViewButton(views::ButtonListener* listener) :
        BililiveImageButton(listener)
    {

    }

protected:
    bool OnMousePressed(const ui::MouseEvent& event) override
    {
        bool b = __super::OnMousePressed(event);
        return b;
    }

    void OnMouseReleased(const ui::MouseEvent& event) override
    {
        __super::OnMouseReleased(event);
    }
};

BeautyCameraControlPreviewView::BeautyCameraControlPreviewView()
{
    InitView();
}

BeautyCameraControlPreviewView::~BeautyCameraControlPreviewView()
{

}

void BeautyCameraControlPreviewView::ShowButton(bool show)
{
    contrast_btn_->SetVisible(show);
}

void BeautyCameraControlPreviewView::ShowSlider(bool show)
{
    slider_view_->SetVisible(show);
}

//void BeautyCameraControlPreviewView::BindMaterial(const BeautyClassTag& tag)
//{
//    tag_ = tag;
//
//    std::vector<BeautyMaterialValue> value;
//    if (BeautyCameraController::GetInstance()->GetBeautyMaterialValue(tag_, value))
//    {
//        slider_view_->SetProgressData(tag_, value);
//    }
//    else
//    {
//        DCHECK(0);
//    }
//}

//void BeautyCameraControlPreviewView::SetProcessData(const BeautyClassTag& tag, const std::vector<BeautyMaterialValue>& vec_vector)
//{
//    tag_ = tag;
//    slider_view_->SetProgressData(tag_, vec_vector);
//}

void BeautyCameraControlPreviewView::VisibilityChanged(View* starting_from, bool is_visible)
{
    __super::VisibilityChanged(starting_from, is_visible);

    Layout();
}

void BeautyCameraControlPreviewView::ButtonPressed(views::Button* sender, const ui::Event& event)
{

}

void BeautyCameraControlPreviewView::InitView()
{
    auto& rb = ResourceBundle::GetSharedInstance();

    slider_view_ = new BeautySlideView();

    contrast_btn_ = new PreviewViewButton(this);
    contrast_btn_->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_CONTRAST));
    contrast_btn_->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_CONTRAST_ACTIVE));
    contrast_btn_->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_CONTRAST));

    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    auto col_set = layout->AddColumnSet(0);
    col_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::TRAILING, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    col_set->AddColumn(views::GridLayout::TRAILING, views::GridLayout::TRAILING, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    col_set->AddPaddingColumn(0.0f, GetLengthByDPIScale(8));

    layout->StartRow(1.0f, 0);
    layout->AddView(slider_view_);
    layout->AddView(contrast_btn_);

    layout->AddPaddingRow(0, GetLengthByDPIScale(6));

    //bool b = LivehimeLiveRoomController::GetInstance()->IsLiveReady();
    //ShowButton(b);
}

