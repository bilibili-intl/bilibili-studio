#include "danmaku_render_viddup_view.h"

#include "base/notification/notification_service.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "bilibase/string_encoding_conversions.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/base/win/dpi.h"

#include "bililive/bililive/livehime/danmaku_hime/danmakuhime_data_handler.h"
#include "bililive/bililive/livehime/audio_player/audio_player_presenter_imp.h"
#include "bililive/bililive/livehime/gift_image/image_fetcher.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_menu.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/d2d_manager.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_renderer.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_structs.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/danmaku_list_adapter.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_structs.h"

#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_hotkey_notify_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"

#include "grit/generated_resources.h"

#include <fstream>


DanmakuRenderViddupView::DanmakuRenderViddupView(DanmakuViewType type, int max_count)
    : max_count_(max_count),
    type_(type),
    weak_ptr_factory_(this)
{
    dmk_adapter_ = new dmkhime::DanmakuListAdapter();

    bool can_interactive = type_ != DanmakuViewType::ANTI_BRUSHING && type_ != DanmakuViewType::COMBO_EFFECT;
    float speed_factor = type_ == DanmakuViewType::ANTI_BRUSHING ? 16.f : 1.f;

    dmk_list_ = std::make_unique<dmkhime::DanmakuList>(can_interactive, speed_factor);
    dmk_list_->SetRenderCallback(
        std::bind(&DanmakuRenderViddupView::OnRenderCallback, this));
    dmk_list_->SetAdapter(dmk_adapter_);
    dmk_list_->SetRecycleListener(dmk_adapter_);
    dmk_list_->SetStatusChangeListener(this);

    InitViews();
}

DanmakuRenderViddupView::DanmakuRenderViddupView(int max_count, bool can_interactive, float speed_factor, dmkhime::DanmakuListAdapter* adapter) :
    max_count_(max_count),
    weak_ptr_factory_(this)
{
    if (!adapter)
    {
        dmk_adapter_ = new dmkhime::DanmakuListAdapter();
    }
    else
    {
        dmk_adapter_ = adapter;
    }

    dmk_list_ = std::make_unique<dmkhime::DanmakuList>(can_interactive, speed_factor);
    dmk_list_->SetRenderCallback(
        std::bind(&DanmakuRenderViddupView::OnRenderCallback, this));
    dmk_list_->SetAdapter(dmk_adapter_);
    dmk_list_->SetRecycleListener(dmk_adapter_);
    dmk_list_->SetStatusChangeListener(this);

    InitViews();
}

DanmakuRenderViddupView::~DanmakuRenderViddupView()
{
    UninitViews();
}

void DanmakuRenderViddupView::InitViews()
{
    base::Thread::Options options;
    options.message_loop_type = base::MessageLoop::TYPE_IO;
    drawing_thread_.reset(new base::Thread("Danmaku_Draw_Thread"));
    if (!drawing_thread_->StartWithOptions(options))
    {
        drawing_thread_->Stop();
    }
}

void DanmakuRenderViddupView::UninitViews()
{
    if (gif_timer_.IsRunning())
    {
        gif_timer_.Stop();
    }
    if (drawing_thread_)
    {
        drawing_thread_->Stop();
        drawing_thread_.reset(nullptr);
    }
}
void DanmakuRenderViddupView::StartGifTimer(bool start)
{
    if (start)
    {
        if (!gif_timer_.IsRunning())
        {
            gif_timer_.Start(
                FROM_HERE, base::TimeDelta::FromMilliseconds(50),
                this, &DanmakuRenderViddupView::OnGifRenderTimer);
        }
    }
}

void DanmakuRenderViddupView::SetEnterEffectDelegate(EnterEffectViddupDelegate* delegate)
{
    delegate_ = delegate;
}

void DanmakuRenderViddupView::Clear()
{
    dmk_adapter_->Clear();
    dmk_list_->NotifyDataSetChanged();
    SchedulePaint();
}

void DanmakuRenderViddupView::OnPaint(gfx::Canvas* canvas)
{
    __super::OnPaint(canvas);

    if (rendering_params_.hardware_acceleration_)
    {
        auto result = HardwareRender(canvas);
        switch (result)
        {
        case HWRenderResult::NeedRetry: {
            LOG(WARNING) << "An error occurred when using hardware-rendering, retrying...";
            hw_rt_.Release();
            result = HardwareRender(canvas);
            if (result == HWRenderResult::Succeeded)
            {
                LOG(INFO) << "The error have been repaired.";
            }
            else
            {
                LOG(WARNING) << "Failed to hardware-rendering, fall back to software-rendering.";
                // fallback
                FallbackToSoftwareRender();
            }
            break;
        }
        case HWRenderResult::Failed: {
            LOG(WARNING) << "Failed to hardware-rendering, fall back to software-rendering.";
            // fallback
            FallbackToSoftwareRender();
            break;
        }
        default:
            break;
        }
    }
    else
    {
        SoftwareRender(canvas);
    }
}

void DanmakuRenderViddupView::OnBoundsChanged(const gfx::Rect& previous_bounds)
{
    CreateHardwareSurface();

    auto bounds = GetContentsBounds();
    dmk_list_->Measure(bounds.width(), bounds.height());
    dmk_list_->Layout(bounds.x(), bounds.y(), bounds.right(), bounds.bottom());
    dmk_list_->ScrollToBottomIfAtBottom(false);

    if (label_no_data_notes_)
    {
        label_no_data_notes_->SetBounds(10, 0, bounds.width() - GetLengthByDPIScale(20), bounds.height());
    }
}

gfx::NativeCursor DanmakuRenderViddupView::GetCursor(const ui::MouseEvent& event)
{
    static gfx::NativeCursor hand = ::LoadCursorW(nullptr, IDC_HAND);
    if (dmk_list_->IsClickable(event.location()))
    {
        return hand;
    }
    return __super::GetCursor(event);
}

views::View* DanmakuRenderViddupView::GetEventHandlerForPoint(const gfx::Point& point)
{
    if (type_ == DanmakuViewType::COMBO_EFFECT)
    {
        return __super::GetEventHandlerForPoint(point);
    }
    return this;
}

bool DanmakuRenderViddupView::OnMouseWheel(const ui::MouseWheelEvent& event)
{
    if (dmk_list_->OnMouseWheel(event))
    {
        SchedulePaint();
        return true;
    }
    return false;
}

bool DanmakuRenderViddupView::OnMousePressed(const ui::MouseEvent& event)
{
    dmkhime::DmkHitType type = dmkhime::DmkHitType::None;
    const dmkhime::DmkDataObject* data = nullptr;
    bool result = dmk_list_->OnMousePressed(event, &type, &data);
    if (result)
    {
        SchedulePaint();
    }

    if (type == dmkhime::DmkHitType::Danmaku &&
        event.IsLeftMouseButton())
    {

        auto dmk_data = static_cast<const dmkhime::CommonDmkData*>(data);
        if (dmk_data->is_click_voice)
        {
            // Sound is played when the microphone is not connected
            if (!IsCallConnectStatus())
            {
                AudioPlayerPresenterImp::GetInstance()->OpenFile(bilibase::UrlDecode(dmk_data->voice_url), dmk_data->file_format, false);
            }
            else
            {
                // Don't want to make a judgment at the bottom, just click Play gif and close it
                AudioPlayInfo info;
                info.file_name = bilibase::UrlDecode(dmk_data->voice_url);
                info.paly = false;
                UpdateVoiceImgState(info);
            }
        }
    }

    return result;
}

void DanmakuRenderViddupView::OnMouseReleased(const ui::MouseEvent& event)
{
    dmkhime::DmkHitType type = dmkhime::DmkHitType::None;
    const dmkhime::DmkDataObject* data = nullptr;
    dmk_list_->OnMouseReleased(event, &type, &data);

    if (type == dmkhime::DmkHitType::CommonNoticeDanmakuBtn)
    {

        auto common_notice_data = reinterpret_cast<const dmkhime::CommonNoticeDanmakuRenderData*>(data);

        for (auto& segment : common_notice_data->common_notice_danmaku_data.vec_segment)
        {

            if (segment.elem_type == 3)
            {

                auto button_segment = reinterpret_cast<CommonNoticeButtonSegment*>(segment.content_segments.get());
                if (!button_segment->uri.empty())
                {

                    std::string::size_type position;
                    position = button_segment->uri.find("bililive://blink/");
                    if (position != button_segment->uri.npos)
                    {

                    }
                    else
                    {

                        DanmakuhimeDataHandler::GetInstance()->CommonNoticeDanmakuOpenWeb(button_segment->uri);
                    }
                }
                break;
            }
        }
    }

    else if (type == dmkhime::DmkHitType::LiveAsistantDanmakuBtn)
    {
        auto live_assistant_data = reinterpret_cast<const dmkhime::LiveAsistantDanmakuRenderData*>(data);
        bool has_button = live_assistant_data->live_asistant_data.asit_has_button;
        if (has_button)
        {

        }

        base::StringPairs event_ext;
        int assist_type = 4;
        if (live_assistant_data->live_asistant_data.report_type == "play_together")
        {
            assist_type = 5;
            event_ext.push_back(std::pair<std::string, std::string>("audience_id", live_assistant_data->live_asistant_data.together_uid));
        }
        event_ext.push_back(std::pair<std::string, std::string>("msg_area", "1"));
        event_ext.push_back(std::pair<std::string, std::string>("type", std::to_string(assist_type)));
        event_ext.push_back(std::pair<std::string, std::string>("msg_id", live_assistant_data->live_asistant_data.milestone_type));
        event_ext.push_back(std::pair<std::string, std::string>("milestone", std::to_string(live_assistant_data->live_asistant_data.milestone_index)));
        event_ext.push_back(std::pair<std::string, std::string>("milestone_value", std::to_string(live_assistant_data->live_asistant_data.milestone_value)));
    }
}

void DanmakuRenderViddupView::OnMouseCaptureLost()
{
    dmk_list_->OnMouseCaptureLost();
}

void DanmakuRenderViddupView::OnMouseMoved(const ui::MouseEvent& event)
{
    dmk_list_->OnMouseMoved(event);
}

bool DanmakuRenderViddupView::OnMouseDragged(const ui::MouseEvent& event)
{
    bool result = dmk_list_->OnMouseDragged(event);
    return result;
}

void DanmakuRenderViddupView::OnMouseEntered(const ui::MouseEvent& event)
{
    if (is_suspend_scrolling_enabled_)
    {
        SuspendScrolling(true);
    }
}

void DanmakuRenderViddupView::OnMouseExited(const ui::MouseEvent& event)
{
    SuspendScrolling(false);
}

bool DanmakuRenderViddupView::HitTestRect(const gfx::Rect& rect) const
{
    if (!View::HitTestRect(rect))
    {
        return false;
    }

    return type_ != DanmakuViewType::COMBO_EFFECT;
}

void DanmakuRenderViddupView::OnListAnimationStart()
{
    OnAnimationStartCallback();
}

void DanmakuRenderViddupView::OnListAutoScrollStateChanged(bool auto_scroll)
{
    if (auto_scroll)
    {
        new_count_ = 0;
    }
    else
    {
        SetNavDownText(false);
    }
}

void DanmakuRenderViddupView::AddTipDanmaku()
{

}

dmkhime::TipsDmkData DanmakuRenderViddupView::OnGetTipDanmakuInfo()
{
    dmkhime::TipsDmkData dat;
    if (type_ == DanmakuViewType::DANMAKU)
    {
        auto& rb = ResourceBundle::GetSharedInstance();
        dat.tips_text = rb.GetLocalizedString(IDS_DANMAKUHIME_DMK_REGION_TIPS);
        dat.parent_height = DanmakuhimeDataHandler::GetInstance()->GetRenderViewCurHeight(DanmakuWindowsType::Windows_Type_Interaction);
    }
    return dat;
}

void DanmakuRenderViddupView::SetNavDownText(bool has_new)
{
    auto& rb = ResourceBundle::GetSharedInstance();
    if (has_new)
    {
        string16 text;
        if (type_ == DanmakuViewType::DANMAKU)
        {
            text.append(rb.GetLocalizedString(IDS_DANMU_NEW_MSG));
        }
        else if (type_ == DanmakuViewType::GIFT_SUPER_CHAT)
        {
            text.append(L"New item");
        }
        else
        {
            text.append(rb.GetLocalizedString(IDS_DANMU_NEW));
        }
        if (new_count_ > 99)
        {
            text.append(L"99+");
        }
        else
        {
            text.append(std::to_wstring(new_count_));
        }
        dmk_list_->SetNavDownText(text);
    }
    else
    {
        dmk_list_->SetNavDownText(rb.GetLocalizedString(IDS_DANMU_GOTO_BOTTOM));
    }
}

void DanmakuRenderViddupView::SoftwareRender(gfx::Canvas* canvas)
{
    auto render_target = D2DManager::GetInstance()->CreateDCRenderTarget();
    if (render_target)
    {
        HDC hDC = canvas->BeginPlatformPaint();

        auto bounds = GetContentsBounds();
        RECT rect = { 0, 0, bounds.width(), bounds.height() };
        HRESULT hr = render_target->BindDC(hDC, &rect);

        if (SUCCEEDED(hr))
        {
            base::win::ScopedComPtr<ID2D1RenderTarget> rt;
            hr = render_target.QueryInterface(rt.Receive());
            DCHECK(SUCCEEDED(hr));

            rt->BeginDraw();

            Sculptor sp(rt);
            OnDraw(sp);

            hr = rt->EndDraw();
            if (FAILED(hr))
            {
                DCHECK(false);
            }
        }
        else
        {
            DCHECK(false);
        }

        canvas->EndPlatformPaint();
    }
}

DanmakuRenderViddupView::HWRenderResult DanmakuRenderViddupView::HardwareRender(gfx::Canvas* canvas)
{
    if (!hw_rt_)
    {
        CreateHardwareSurface();
        if (!hw_rt_)
        {
            return HWRenderResult::NeedRetry;
        }
    }

    HWRenderResult result_code = HWRenderResult::Succeeded;
    gfx::Canvas offc(GetContentsBounds().size(), ui::SCALE_FACTOR_100P, false);

    HDC hDC = offc.BeginPlatformPaint();

    hw_rt_->BeginDraw();
    hw_rt_->Clear();

    Sculptor sp(hw_rt_);
    OnDraw(sp);

    base::win::ScopedComPtr<ID2D1GdiInteropRenderTarget> gdi_rt;
    HRESULT hr = hw_rt_.QueryInterface(gdi_rt.Receive());
    if (SUCCEEDED(hr))
    {
        HDC hdc = nullptr;
        hr = gdi_rt->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hdc);
        if (SUCCEEDED(hr))
        {
            auto bounds = GetContentsBounds();
            BOOL result = ::BitBlt(
                hDC, 0, 0, bounds.width(), bounds.height(),
                hdc, 0, 0, SRCPAINT);
            DCHECK(result != 0);

            RECT rect = {};
            hr = gdi_rt->ReleaseDC(&rect);
            DCHECK(SUCCEEDED(hr));
        }
        else
        {
            if (hr == D2DERR_WRONG_RESOURCE_DOMAIN)
            {
                result_code = HWRenderResult::NeedRetry;
            }
            else if (D2DManager::GetInstance()->ProcessDeviceError(hr))
            {
                result_code = HWRenderResult::NeedRetry;
            }
            else
            {
                result_code = HWRenderResult::Failed;
                DCHECK(false);
            }
        }
    }
    else
    {
        NOTREACHED();
    }

    if (hw_rt_)
    {
        hr = hw_rt_->EndDraw();
        if (FAILED(hr))
        {
            if (hr == D2DERR_RECREATE_TARGET ||
                hr == D2DERR_WRONG_RESOURCE_DOMAIN)
            {
                if (result_code == HWRenderResult::Succeeded)
                {
                    result_code = HWRenderResult::NeedRetry;
                }
            }
            else
            {
                if (result_code == HWRenderResult::Succeeded)
                {
                    result_code = HWRenderResult::Failed;
                    DCHECK(false);
                }
            }
        }
    }

    offc.EndPlatformPaint();

    canvas->DrawImageInt(gfx::ImageSkia(offc.ExtractImageRep()), 0, 0);

    return result_code;
}

void DanmakuRenderViddupView::CreateHardwareSurface()
{
    if (!rendering_params_.hardware_acceleration_)
    {
        return;
    }

    auto bounds = GetContentsBounds();
    if (!bounds.IsEmpty())
    {
        auto surface = D2DManager::GetInstance()->CreateDXGISurface(bounds.width(), bounds.height());
        if (surface)
        {
            hw_rt_ = D2DManager::GetInstance()->CreateHardwareRenderTarget(surface, false);
        }
        else
        {
            hw_rt_ = nullptr;
        }
    }
}

void DanmakuRenderViddupView::SuspendScrolling(bool suspend)
{
    if (is_suspend_scrolling_ != suspend)
    {
        is_suspend_scrolling_ = suspend;
        if (!suspend)
        {
            OnAnimationStartCallback();
        }
    }
}

void DanmakuRenderViddupView::RedrawAllElements(const DanmakuRenderingParams& params)
{
    rendering_params_ = params;
    bool need_redraw_effects =
        (params.flags & DanmakuRenderingParams::Flag_HW) ||
        (params.flags & DanmakuRenderingParams::Flag_ShowIcon);

    if (params.flags & DanmakuRenderingParams::Flag_HW)
    {
        if (params.hardware_acceleration_)
        {
            CreateHardwareSurface();
        }
        else
        {
            hw_rt_.Release();
        }
    }

    RedrawAllElementsSync(false, need_redraw_effects);
}

const DanmakuRenderingParams& DanmakuRenderViddupView::GetRenderingParams()
{
    return rendering_params_;
}

void DanmakuRenderViddupView::RedrawAllElementsSync(bool force, bool redraw_effects)
{
    auto bounds = GetContentsBounds();
    dmk_adapter_->SetRenderingParams(rendering_params_);
    //if (effect_stub_)
    //{
    //    effect_stub_->SetRenderingParams(rendering_params_);
    //}

    bool has_item = false;
    for (const auto& r : dmk_adapter_->GetItems())
    {
        if (force)
        {
            r->Invalid();
        }
        has_item = true;
    }
    //if (redraw_effects && effect_stub_)
    //{
    //    for (const auto& r : effect_stub_->GetGiftEffect()->GetProducts())
    //    {
    //        if (force)
    //        {
    //            r->Invalid();
    //        }
    //        auto& params = rendering_params_;
    //        r->Refactor(params.show_icon);
    //        r->Measure(bounds.width(), params.base_font_size);
    //        r->Render(params.opacity, params.theme, params.hardware_acceleration_);
    //    }
    //    has_item = true;
    //}

    if (!has_item)
    {
        return;
    }

    dmk_list_->NotifyDataSetChanged();
    dmk_list_->ScrollToBottomIfAtBottom(false);
    SchedulePaint();
}

void DanmakuRenderViddupView::ForceRedrawAllElements()
{
    RedrawAllElementsSync(true, true);
}

void DanmakuRenderViddupView::FallbackToSoftwareRender()
{
    hw_rt_.Release();
    rendering_params_.hardware_acceleration_ = false;
    RedrawAllElementsSync(false, true);
}

void DanmakuRenderViddupView::SetScrollingFps(int fps)
{
    int interval = 1000 / fps;
    if (interval == scrolling_interval_)
    {
        return;
    }

    scrolling_interval_ = interval;

    if (timer_.IsRunning())
    {
        timer_.Stop();
        timer_.Start(FROM_HERE, base::TimeDelta::FromMilliseconds(scrolling_interval_),
            this, &DanmakuRenderViddupView::OnRenderTimer);
    }
}

void DanmakuRenderViddupView::SetSuspendingScrollingEnabled(bool enabled)
{
    if (is_suspend_scrolling_enabled_ != enabled)
    {
        is_suspend_scrolling_enabled_ = enabled;
        if (!enabled)
        {
            SuspendScrolling(false);
        }
    }
}

void DanmakuRenderViddupView::AddDanmaku(const DanmakuData& data)
{
    int width = GetContentsBounds().width();
    //auto renderer = std::make_shared<dmkhime::DanmakuRendererPtr>();
    DmkRendererPtrVectorPtr vec_renderer_ptr = std::make_shared<std::vector<dmkhime::DanmakuRendererPtr>>();

    last_data_ = data;
    has_last_data_ = true;

    if (drawing_thread_ && drawing_thread_->IsRunning())
    {
        // Gives the danmaku to the worker thread to download the resource
        drawing_thread_->message_loop_proxy()->PostTaskAndReply(
            FROM_HERE,
            base::Bind(&DanmakuRenderViddupView::RenderOneDanmakuRawForward, base::Unretained(this), data, vec_renderer_ptr, width),
            base::Bind(&DanmakuRenderViddupView::OnDanmakuAdded, weak_ptr_factory_.GetWeakPtr(), vec_renderer_ptr));
    }
}

void DanmakuRenderViddupView::UpdateVoiceImgState(const AudioPlayInfo& data)
{
    bool need_redraw = false;

    auto& items = dmk_adapter_->GetItems();
    for (auto it = items.begin(); it != items.end(); ++it)
    {
        if (it->get()->GetType() != dmkhime::DRT_COMMON)
        {
            continue;
        }

        auto renderer = static_cast<dmkhime::CommonViddupDmkRenderer*>(it->get());
        auto dmk_data = static_cast<const dmkhime::CommonDmkData*>(it->get()->GetData());

        if (dmk_data->voice_url.empty() || data.file_name.empty())
            return;
    }

    SchedulePaint();
}

void DanmakuRenderViddupView::UpdateView()
{
    SchedulePaint();
}

void DanmakuRenderViddupView::UpdateDanmaku(const DanmakuData& data)
{
    bool need_redraw = false;
    if (type_ == DanmakuViewType::GIFT_SUPER_CHAT)
    {
        return;
    }
    else if (type_ == DanmakuViewType::DANMAKU)
    {
        if (data.type != DT_DANMU_MSG)
        {
            return;
        }

        auto& items = dmk_adapter_->GetItems();
        for (auto it = items.begin(); it != items.end(); ++it)
        {
            if (it->get()->GetType() != dmkhime::DRT_COMMON)
            {
                continue;
            }
        }
    }
    else
    {
        NOTREACHED();
        return;
    }

    if (need_redraw)
    {
        SchedulePaint();
    }
}

bool DanmakuRenderViddupView::GetLastDanmakuData(DanmakuData* out) const
{
    if (has_last_data_)
    {
        *out = last_data_;
        return true;
    }
    return false;
}

void DanmakuRenderViddupView::OnDanmakuAdded(DmkRendererPtrVectorPtr vec_renderer_ptr_ptr)
{

    if (!vec_renderer_ptr_ptr)
    {
        DCHECK(0);
        return;
    }

    auto& vec_renderer_ptr = *vec_renderer_ptr_ptr;

    for (auto& ptr : vec_renderer_ptr)
    {
        if (!ptr)
        {
            DCHECK(0);
            continue;
        }

        if (dmk_adapter_->GetItemCount() == 1)
        {
            auto& items = dmk_adapter_->GetItems();
            if (items.back()->GetType() == dmkhime::DRT_TIPS)
            {
                dmk_adapter_->RemoveDanmakuVision(0);
                dmk_list_->NotifyItemRemoveFromTop(1);
            }
        }

        if (label_no_data_notes_)
        {
            this->RemoveChildView(label_no_data_notes_);
            delete label_no_data_notes_;
            label_no_data_notes_ = nullptr;
        }

        dmk_adapter_->AddDanmakuVision(ptr);

        int remove_from_top_count = 0;
        if (dmk_adapter_->GetItemCount() > max_count_)
        {
            remove_from_top_count = dmk_adapter_->GetItemCount() - max_count_;
            dmk_adapter_->RemoveDanmakuVisions(0, remove_from_top_count);

            if (!com_dmk_list_.empty())
            {
                std::lock_guard<std::mutex> lk(com_dmk_list_mtx_);
                com_dmk_list_.erase(com_dmk_list_.begin());
            }
            if (!gift_dmk_list_.empty())
            {
                std::lock_guard<std::mutex> lk(gift_dmk_list_mtx_);
                gift_dmk_list_.erase(gift_dmk_list_.begin());
            }
        }

        // No auto scroll. We need to count new bullets
        if (!dmk_list_->IsAutoScrollToBottom())
        {
            ++new_count_;
            SetNavDownText(true);
        }

        dmk_list_->NotifyItemAddToBottom();
        dmk_list_->NotifyItemRemoveFromTop(remove_from_top_count);
        // t some point you may want to have a barrage coming in when the render area is not visible. 
        // Smooth scrolling is not necessary at this point
        dmk_list_->ScrollToBottomIfAtBottom(visible());
        SchedulePaint();
    }
}

void DanmakuRenderViddupView::OnDraw(Sculptor& s)
{
    dmk_list_->Draw(&s);
}

void DanmakuRenderViddupView::OnRenderTimer()
{
    if (timer_.IsRunning())
    {
        bool scrolling_stopped = true;
        if (!is_suspend_scrolling_)
        {
            scrolling_stopped = !dmk_list_->OnComputeScroll(scrolling_interval_);
        }
    }

    SchedulePaint();
}
void DanmakuRenderViddupView::OnGifRenderTimer()
{
    if (gif_timer_.IsRunning())
    {
        auto cur_time = base::TimeTicks::Now();
        int interval = (cur_time - start_time_).InMilliseconds();
        bool need_refresh = false;
        {
            std::lock_guard<std::mutex> lk(com_dmk_list_mtx_);
           
            for (auto it = com_dmk_list_.begin(); it != com_dmk_list_.end(); ++it)
            {
                if (it->get())
                {
                    auto renderer = static_cast<dmkhime::CommonViddupDmkRenderer*>(it->get());
                    if (renderer && renderer->SetGifTick(interval))
                    {
                        need_refresh = true;
                    }
                }
            }
        }
        need_refresh = false;
        {
         std::lock_guard<std::mutex> lk(gift_dmk_list_mtx_);
            for (auto it = gift_dmk_list_.begin(); it != gift_dmk_list_.end(); ++it)
            {
                if (it->get())
                {
                    auto renderer = static_cast<dmkhime::GiftComboViddupDmkRenderer*>(it->get());
                    if (renderer && renderer->SetGifTick(interval))
                    {
                        need_refresh = true;
                    }
                }
            } 
        }
        start_time_ = cur_time;
        if (need_refresh)
        {
            SchedulePaint();
        }
    }
}

void DanmakuRenderViddupView::OnRenderCallback()
{
    SchedulePaint();
}

void DanmakuRenderViddupView::OnAnimationStartCallback()
{
    if (!timer_.IsRunning())
    {
        timer_.Start(
            FROM_HERE, base::TimeDelta::FromMilliseconds(scrolling_interval_),
            this, &DanmakuRenderViddupView::OnRenderTimer);

        if (delegate_)
        {
            delegate_->OnAnimationStart();
        }
    }
}

bool DanmakuRenderViddupView::IsCallConnectStatus()
{
    bool is_linked = false;

    return is_linked;
}

DanmakuRenderingParams& DanmakuRenderViddupView::RenderParam()
{
    return rendering_params_;
}

dmkhime::DanmakuListAdapter* DanmakuRenderViddupView::GetDanmakuListAdapter()
{
    return dmk_adapter_;
}

void DanmakuRenderViddupView::SetDanmakuListAdapter(dmkhime::DanmakuListAdapter* adapter)
{
    dmk_list_->SetAdapter(adapter);
    dmk_list_->SetRecycleListener(adapter);
    dmk_adapter_ = adapter;
}

dmkhime::DanmakuList* DanmakuRenderViddupView::GetDanmakuList()
{
    return dmk_list_.get();
}

void DanmakuRenderViddupView::RenderOneDanmakuRaw(
    const DanmakuData& data, DmkRendererPtrVectorPtr vec_renderer_ptr_ptr, int width)
{
    std::vector<dmkhime::DanmakuRendererPtr>& vec_renderer_ptr = *vec_renderer_ptr_ptr;

    dmkhime::DanmakuRendererPtr renderer;

    switch (data.type)
    {
    case DT_DANMU_MSG:
    {
        DCHECK(type_ == DanmakuViewType::DANMAKU);

        dmkhime::CommonDmkData dat;
        dat.plain_text = data.plain_text;
        dat.user_name = data.user_name;

        if (data.user_identity_tag_admin_)
        {
            dat.user_identity_tag_admin_ = ImageFetcher::GetInstance()->FetchAsset(ImageFetcher::DAMAKU_ICON_USER_IDENTITY_TAG_ADMIN);
        }

        switch (data.user_identity_tag_top_)
        {
        case DanmakuViddupInfo::Rank1:
            dat.user_identity_tag_top_ = ImageFetcher::GetInstance()->FetchAsset(ImageFetcher::DAMAKU_ICON_USER_IDENTITY_TAG_TOP1);
            break;
        case DanmakuViddupInfo::Rank2:
            dat.user_identity_tag_top_ = ImageFetcher::GetInstance()->FetchAsset(ImageFetcher::DAMAKU_ICON_USER_IDENTITY_TAG_TOP2);
            break;
        case DanmakuViddupInfo::Rank3:
            dat.user_identity_tag_top_ = ImageFetcher::GetInstance()->FetchAsset(ImageFetcher::DAMAKU_ICON_USER_IDENTITY_TAG_TOP3);
            break;
        }

        renderer = std::make_shared<dmkhime::CommonViddupDmkRenderer>(dat);
        {
            std::lock_guard<std::mutex> lk(com_dmk_list_mtx_);
            com_dmk_list_.push_back(renderer);
        }
        vec_renderer_ptr.push_back(renderer);
        break;
    }
    case DT_SYS_MSG:
    {
        DCHECK(type_ == DanmakuViewType::DANMAKU);

        dmkhime::AnnocDmkData dat;
        dat.limit_max_display = data.limit_max_display;
        dat.user_name = data.user_name;
        dat.plain_text = data.plain_text;

        dat.popup_msg = data.popup_msg;
        dat.block_msg = data.block_msg;

        renderer = std::make_shared<dmkhime::AnnocViddupDmkRenderer>(dat);
        vec_renderer_ptr.push_back(renderer);
        break;
    }    
    case DT_SEND_GIFT:
    {
        if (data.has_combo_send)
        {
            DCHECK(type_ == DanmakuViewType::DANMAKU);

            dmkhime::GiftComboDmkData dat;
            dat.action = data.action;
            dat.user_name = data.user_name;
            dat.gift_combo_string = data.gift_combo_string;
            dat.gift_id = data.gift_id;
            dat.gift_name = data.gift_name;
            dat.gift_icon_data = ImageFetcher::GetInstance()->FetchGiftDynamicImageSyncByUrl(data.gift_id, data.face_url);

            renderer = std::make_shared<dmkhime::GiftComboViddupDmkRenderer>(dat);
            {
                std::lock_guard<std::mutex> lk(gift_dmk_list_mtx_);
                gift_dmk_list_.push_back(renderer);
            }
            vec_renderer_ptr.push_back(renderer);     
        }

        break;
    }

    case DT_ANCHOR_BROADCAST:
    {
        DCHECK(type_ == DanmakuViewType::DANMAKU);

        dmkhime::LiveAsistantDanmakuRenderData dat;
        dat.live_asistant_data = data.asistant_data;
        
        if (data.warning_msg)
        {
            dat.live_asistant_data.warning_icon = ImageFetcher::GetInstance()->FetchAsset(ImageFetcher::DAMAKU_ICON_WARING);
        }

        renderer = std::make_shared<dmkhime::LiveAsistantViddupDmkRenderer>(dat);
        vec_renderer_ptr.push_back(renderer);
        break;
    }

    default:
        break;
    }

    if (!renderer)
    {
        return;
    }

    DCHECK(vec_renderer_ptr.size() >= 1);

    for (auto& ptr : vec_renderer_ptr)
    {
        ptr->Init(width, rendering_params_.show_icon);
    }
}

void DanmakuRenderViddupView::RenderOneDanmakuRawForward(const DanmakuData& data, DmkRendererPtrVectorPtr vec_renderer_ptr_ptr, int width)
{
    RenderOneDanmakuRaw(data, vec_renderer_ptr_ptr, width);
}

void DanmakuRenderViddupView::ShowNoDataNotes(const string16& text)
{
    if (!label_no_data_notes_)
    {
        label_no_data_notes_ = new BililiveLabel(text);
        label_no_data_notes_->SetFont(ftTwelve);
        label_no_data_notes_->SetTextColor(SkColorSetRGB(0x8d, 0x9a, 0xa4));
        label_no_data_notes_->SetMultiLine(true);
        label_no_data_notes_->SetAllowCharacterBreak(true);
        label_no_data_notes_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
        label_no_data_notes_->SetVerticalAlignment(gfx::ALIGN_TOP);
        this->AddChildView(label_no_data_notes_);
    }
    else
    {
        label_no_data_notes_->SetText(text);
    }
    label_no_data_notes_->SetVisible(true);
    auto bounds = GetContentsBounds();
    label_no_data_notes_->SetBounds(10, 0, bounds.width() - GetLengthByDPIScale(20), bounds.height());
}
