#include "danmaku_interaction_viddup_view.h"

#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/blank_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/controls/dmkhime_toast_view.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/super_chat/markedness_message_view.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/secret/public/event_tracking_service.h"
#include "bililive/bililive/livehime/danmaku_hime/dmkhime_source_presenter_impl.h"
#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/livehime/danmaku_hime/danmakuhime_data_handler.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/secret/bililive_secret.h"

#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/layout_manager.h"
#include "ui/base/resource/resource_bundle.h"

#include <chrono>
#include <bililive/bililive/livehime/danmaku_hime/MessageLoopView.h>

const int kAntiBrushingHeight = GetLengthByDPIScale(36);
const int kDanmakuTextInputViewHeight = GetLengthByDPIScale(30);

//const int DanmakuInteractionViddupView::kMergeMiniHeight = GetLengthByDPIScale(370);

class DanmakuLayout : public views::LayoutManager
{
public:
    DanmakuLayout() {}

    void Layout(views::View* host) override
    {
        if (!host->has_children())
        {
            return;
        }
        if (host->child_count() != 3)
        {
            NOTREACHED();
            return;
        }

        views::View* dmk_view = host->child_at(0);
        views::View* anti_interact_view = host->child_at(1);
        views::View* anti_free_view = host->child_at(2);

        auto bounds = host->GetContentsBounds();
        int antib_height = anti_interact_view->height() + anti_free_view->height();

        dmk_view->SetBoundsRect(
            gfx::Rect(
                bounds.x(), bounds.y(),
                bounds.width(), bounds.height() - antib_height));

        int cur_y = bounds.y() + dmk_view->bounds().bottom();
        anti_interact_view->SetBoundsRect(
            gfx::Rect(
                bounds.x(), cur_y,
                bounds.width(), anti_interact_view->height()));
        cur_y += anti_interact_view->height();

        anti_free_view->SetBoundsRect(
            gfx::Rect(
                bounds.x(), cur_y,
                bounds.width(), anti_free_view->height()));
    }

    gfx::Size GetPreferredSize(views::View* host) override
    {
        return gfx::Size();
    }
};

class DanmakuIntputTextViewLayout : public views::LayoutManager
{
public:
    DanmakuIntputTextViewLayout()
    {

    }

    virtual ~DanmakuIntputTextViewLayout()
    {

    }

    virtual void Layout(views::View* host) override
    {
        DCHECK(host->child_count() == 2);
        if (host->child_count() != 2)
            return;

        static const int edit_len = GetLengthByDPIScale(199);
        static const int edit_height = GetLengthByDPIScale(30);
        static const int button_y = GetLengthByDPIScale(4);
        static const int button_width = GetLengthByDPIScale(61);
        static const int button_height = GetLengthByDPIScale(30);

        auto edit_view = host->child_at(0);
        auto button_view = host->child_at(1);

        auto bounds = host->GetContentsBounds();

        edit_view->SetBounds(
            bounds.x(), bounds.y(), bounds.width() - button_width, edit_height);
        button_view->SetBounds(
            edit_view->bounds().right(), bounds.y(), button_width, edit_height);
    }

    virtual gfx::Size GetPreferredSize(views::View* host) override
    {
        static const int view_len = GetLengthByDPIScale(280);
        static const int edit_height = GetLengthByDPIScale(34);

        return gfx::Size(view_len, edit_height);
    }

private:
    DISALLOW_COPY_AND_ASSIGN(DanmakuIntputTextViewLayout);

};

DanmakuInteractionViddupView::DanmakuInteractionViddupView() :
    send_dmk_presenter_(this),
    source_presenter_(std::make_unique<DmkhimeSourcePresenterImpl>(this)),
    wpf_(this)
{

    InitView();
}

DanmakuInteractionViddupView::~DanmakuInteractionViddupView()
{
}

void DanmakuInteractionViddupView::AddDanmaku(const DanmakuData& data)
{
    danmaku_rv_->AddDanmaku(data);
}

void DanmakuInteractionViddupView::UpdateView()
{
    danmaku_rv_->UpdateView();
}

void DanmakuInteractionViddupView::SetDelegate(DanmakuInteractionViddupViewDelegate* delegate)
{
    delegate_ = delegate;
}

void DanmakuInteractionViddupView::InitView()
{
    InitInputView();

    title_view_ = new DanmakuInteractionTitleViddupView();
    //title_view_->SetDelegate(this);

    danmaku_rv_ = new DanmakuRenderViddupView(DanmakuViewType::DANMAKU, 500);
    danmaku_rv_->StartGifTimer(true);

    antib_free_rv_ = new DanmakuRenderViddupView(DanmakuViewType::ANTI_BRUSHING, 5);
    antib_free_rv_->SetVisible(false);
    antib_interact_rv_ = new DanmakuRenderViddupView(DanmakuViewType::ANTI_BRUSHING, 5);
    antib_interact_rv_->SetVisible(false);
    danmaku_viewport_ = new views::View();
    {
        danmaku_viewport_->SetLayoutManager(new DanmakuLayout());
        danmaku_viewport_->AddChildView(danmaku_rv_);
        danmaku_viewport_->AddChildView(antib_interact_rv_);
        danmaku_viewport_->AddChildView(antib_free_rv_);
    }

    message_view_ = new MarkednessMessageView(this);
    message_view_->set_background(views::Background::CreateSolidBackground(0));
    message_view_->SetVisible(false);

    label_split_line_ = new SplitLineHorizontalLabel();

    this->AddChildView(title_view_);
    this->AddChildView(danmaku_viewport_);
    this->AddChildView(message_view_);
    this->AddChildView(label_split_line_);
    this->AddChildView(text_input_view_);

    free_anim_ = std::make_unique<views::BoundsAnimator>(danmaku_viewport_);
    interact_anim_ = std::make_unique<views::BoundsAnimator>(danmaku_viewport_);

    Layout();

    SwitchThemeImpl();

    source_presenter_->Initialize();

    InitDanmakuTip();
}

void DanmakuInteractionViddupView::InitInputView()
{
    text_input_view_ = new views::View();
    text_input_view_->SetLayoutManager(new DanmakuIntputTextViewLayout());

    auto& rb = ResourceBundle::GetSharedInstance();

    dmk_edit_view_ = new LivehimeNativeEditView();
    dmk_edit_view_->SetFont(ftTwelve);
    dmk_edit_view_->RemoveBorder();
    dmk_edit_view_->set_placeholder_text(rb.GetLocalizedString(IDS_DANMAKU_INTERFACTION_PLACEHOLDER));
    dmk_edit_view_->SetRadius(0);
    dmk_edit_view_->SetBorderColor(0, 0/*SkColorSetARGB(0x33,0x53, 0x67, 0x77) , SkColorSetARGB(0x33, 0x53, 0x67, 0x77)*/);
    //dmk_edit_view_->SetBackgroundColor(SkColorSetRGB(0xf9, 0xf9, 0xf9));
    dmk_edit_view_->SetBackgroundColor(GetDanmakuThemeColor(GetTheme(), DanmakuThemeViewType::kBackground));
    dmk_edit_view_->LimitText(200);
    dmk_edit_view_->SetSkipDefaultKeyEventProcessing(false);

    //dmk_edit_view_->SetController(this);
    text_input_view_->AddChildView(dmk_edit_view_);

    send_dmk_button_ = new LivehimeActionLabelButton(this, rb.GetLocalizedString(IDS_DANMAKU_INTERFACTION_SEND), true);
    send_dmk_button_->SetRadius(0);
    send_dmk_button_->SetPreferredSize(GetSizeByDPIScale(gfx::Size(61, 30)));
    text_input_view_->AddChildView(send_dmk_button_);
}

void DanmakuInteractionViddupView::InitDanmakuTip()
{
    auto& rb = ResourceBundle::GetSharedInstance();
    string16 local = rb.GetLocalizedString(IDS_DANMAKU_INTERACTION_TIPS);        // tips

    DanmakuData data;
    data.limit_max_display = false;
    data.type = DT_SYS_MSG;
    data.plain_text = local;
    AddDanmaku(data);
}

void DanmakuInteractionViddupView::RedrawRenderViews(const DanmakuRenderingParams& params)
{
    render_params_ = params;

    danmaku_rv_->RedrawAllElements(params);
    antib_interact_rv_->RedrawAllElements(params);
    antib_free_rv_->RedrawAllElements(params);
}

void DanmakuInteractionViddupView::SetScrollingFps(int fps)
{
    danmaku_rv_->SetScrollingFps(fps);
    antib_interact_rv_->SetScrollingFps(fps);
    antib_free_rv_->SetScrollingFps(fps);
}

void DanmakuInteractionViddupView::SetSuspendingScrollingEnabled(bool enabled)
{
    danmaku_rv_->SetSuspendingScrollingEnabled(enabled);
}

void DanmakuInteractionViddupView::SetGiftEffectsEnabled(bool enabled)
{
    
}

void DanmakuInteractionViddupView::ProcessAntiBFree(const DanmakuData& data)
{
    if (free_anim_status_ == AnimStatus::None ||
        free_anim_status_ == AnimStatus::Out)
    {
        OnFreeAntibVisibilityChanged(true);
    }

    antib_free_rv_->SetVisible(true);

    if (!free_timer_.IsRunning() || is_idle_free_timer_)
    {
        antib_free_rv_->AddDanmaku(data);
        free_timer_.Stop();
        is_idle_free_timer_ = false;
        free_timer_.Start(
            FROM_HERE, base::TimeDelta::FromMilliseconds(500),
            base::Bind(&DanmakuInteractionViddupView::OnFreeTimer, base::Unretained(this)));
    }
    else
    {
        free_queue_.push(data);
    }
}

void DanmakuInteractionViddupView::OnFreeTimer()
{
    if (is_idle_free_timer_)
    {
        OnFreeAntibVisibilityChanged(false);
        is_idle_free_timer_ = false;
    }
    else
    {
        if (free_queue_.empty())
        {
            is_idle_free_timer_ = true;
        }
        else
        {
            auto& data = free_queue_.front();
            antib_free_rv_->AddDanmaku(data);
            free_queue_.pop();
        }

        int milli_secs = is_idle_free_timer_ ? 1000 : 500;
        free_timer_.Start(
            FROM_HERE, base::TimeDelta::FromMilliseconds(milli_secs),
            base::Bind(&DanmakuInteractionViddupView::OnFreeTimer, base::Unretained(this)));
    }
}

void DanmakuInteractionViddupView::OnFreeAntibVisibilityChanged(bool visible)
{
    auto cur_bounds = antib_free_rv_->bounds();
    cur_bounds.set_height(visible ? kAntiBrushingHeight : 0);

    if (visible)
    {
        free_anim_status_ = AnimStatus::In;
    }
    else
    {
        free_anim_status_ = AnimStatus::Out;
    }

    free_anim_->AnimateViewTo(antib_free_rv_, cur_bounds);
    //free_anim_->SetAnimationDelegate(antib_free_rv_, this, false);
}

void DanmakuInteractionViddupView::ProcessAntiBInteract(const DanmakuData& data)
{
    if (interact_anim_status_ == AnimStatus::None ||
        interact_anim_status_ == AnimStatus::Out)
    {
        OnInteractAntibVisibilityChanged(true);
    }

    antib_interact_rv_->SetVisible(true);

    if (!interact_timer_.IsRunning() || is_idle_interact_timer_)
    {
        antib_interact_rv_->AddDanmaku(data);
        interact_timer_.Stop();
        is_idle_interact_timer_ = false;
        interact_timer_.Start(
            FROM_HERE, base::TimeDelta::FromMilliseconds(500),
            base::Bind(&DanmakuInteractionViddupView::OnInteractTimer, base::Unretained(this)));
    }
    else
    {
        if (interact_queue_.empty())
        {
            interact_queue_.insert({ data.interact_ts, {data} });
        }
        else
        {
            if (interact_queue_.begin()->first <= data.interact_ts)
            {
                auto it = interact_queue_.find(data.interact_ts);
                if (it == interact_queue_.end())
                {
                    interact_queue_.insert({ data.interact_ts, {data} });
                }
                else
                {
                    if (it->second.size() < 2)
                    {
                        it->second.push_back(data);
                    }
                    else
                    {
                        DCHECK(it->second.size() == 2);
                        if (it->second[0].interact_score < it->second[1].interact_score)
                        {
                            if (data.interact_score >= it->second[0].interact_score)
                            {
                                it->second[0] = data;
                            }
                        }
                        else
                        {
                            if (data.interact_score >= it->second[1].interact_score)
                            {
                                it->second[1] = data;
                            }
                        }
                    }
                }
            }
        }
    }
}

void DanmakuInteractionViddupView::AddNewItem(const secret::LiveStreamingService::MarkednessMessageInfo& danmaku)
{
    if (message_view_)
    {
        message_view_->AddNewItem(danmaku);
    }
}

void DanmakuInteractionViddupView::UpdateMMViews()
{
    if (message_view_)
    {
        InvalidateLayout();
        Layout();
        message_view_->RefreshView();
    }
}

void DanmakuInteractionViddupView::RefreshShadow(int64_t sel_id)
{
    if (message_view_)
    {
        message_view_->RefreshShadow(sel_id);
    }
}

void DanmakuInteractionViddupView::CloseDetailsView()
{
    if (message_view_)
    {
        message_view_->CloseDetailsView();
    }
}

DanmakuInteractionTitleViddupView* DanmakuInteractionViddupView::GetTitleView()
{
    return title_view_;
}

gfx::Point DanmakuInteractionViddupView::GetMarkednessMessageViewPopPosition()
{
    if (!GetWidget() || !message_view_)
    {
        return {};
    }

    int x = message_view_->bounds().x();
    int y = message_view_->bounds().y();

    views::View* cur_view = message_view_;
    while (cur_view)
    {
        y += cur_view->bounds().y();
        x += cur_view->bounds().x();
        cur_view = cur_view->parent();
    }

    return { x, y };
}

void DanmakuInteractionViddupView::OnInteractTimer()
{
    if (is_idle_interact_timer_)
    {
        antib_interact_rv_->Clear();
        antib_interact_rv_->SetVisible(false);
        OnInteractAntibVisibilityChanged(false);
        danmaku_viewport_->InvalidateLayout();
        danmaku_viewport_->Layout();
        is_idle_interact_timer_ = false;

        DanmakuData data;
        if (antib_interact_rv_->GetLastDanmakuData(&data) && InteractType(data.interact_type) != InteractType::UserLike)
        {
            danmaku_rv_->AddDanmaku(data);
        }
    }
    else
    {
        if (cur_slice_.empty() && interact_queue_.empty())
        {
            is_idle_interact_timer_ = true;
        }
        else if (!cur_slice_.empty())
        {
            DCHECK(cur_slice_.size() == 1);
            antib_interact_rv_->AddDanmaku(cur_slice_[0]);
            cur_slice_.clear();
        }
        else
        {
            cur_slice_ = std::move(interact_queue_.begin()->second);
            interact_queue_.erase(interact_queue_.begin());

            DCHECK(cur_slice_.size() <= 2 && cur_slice_.size() > 0);
            if (cur_slice_.size() == 1)
            {
                antib_interact_rv_->AddDanmaku(cur_slice_[0]);
                cur_slice_.clear();
            }
            else if (cur_slice_.size() == 2)
            {
                if (cur_slice_[0].interact_score < cur_slice_[1].interact_score)
                {
                    antib_interact_rv_->AddDanmaku(cur_slice_[1]);
                    cur_slice_.erase(cur_slice_.begin() + 1);
                }
                else
                {
                    antib_interact_rv_->AddDanmaku(cur_slice_[0]);
                    cur_slice_.erase(cur_slice_.begin());
                }
            }
        }

        int milli_secs = is_idle_interact_timer_ ? 1000 : 500;
        interact_timer_.Start(
            FROM_HERE, base::TimeDelta::FromMilliseconds(milli_secs),
            base::Bind(&DanmakuInteractionViddupView::OnInteractTimer, base::Unretained(this)));
    }
}

void DanmakuInteractionViddupView::OnInteractAntibVisibilityChanged(bool visible)
{
    auto cur_bounds = antib_interact_rv_->bounds();
    cur_bounds.set_height(visible ? kAntiBrushingHeight : 0);

    if (visible)
    {
        interact_anim_status_ = AnimStatus::In;
    }
    else
    {
        interact_anim_status_ = AnimStatus::Out;
    }

    interact_anim_->AnimateViewTo(antib_interact_rv_, cur_bounds);
    //interact_anim_->SetAnimationDelegate(antib_interact_rv_, this, false);
}

void DanmakuInteractionViddupView::Layout()
{
    auto content_bounds = GetContentsBounds();
    int y = content_bounds.y();

    label_split_line_->SetBounds(content_bounds.x(), content_bounds.bottom() - kDanmakuTextInputViewHeight - 1, content_bounds.width(), 1);

    text_input_view_->SetBounds(
        content_bounds.x(),
        content_bounds.bottom() - kDanmakuTextInputViewHeight,
        content_bounds.width(),
        kDanmakuTextInputViewHeight);

    title_view_->SetBounds(
        content_bounds.x(),
        y,
        content_bounds.width(),
        DanmakuInteractionTitleViddupView::kViewHeight);
    y += DanmakuInteractionTitleViddupView::kViewHeight;

    int danmaku_viewport_height{};
    if (message_view_->visible())
    {
        message_view_->SetBounds(
            content_bounds.x(), y,
            content_bounds.width(), kMessageViewHeight);
        y += kMessageViewHeight;

        danmaku_viewport_height = content_bounds.height() - y - kDanmakuTextInputViewHeight;
        danmaku_viewport_->SetBounds(
            content_bounds.x(), y,
            content_bounds.width(), danmaku_viewport_height);
        y += danmaku_viewport_height;
    }
    else
    {
        danmaku_viewport_height = content_bounds.height() - y - kDanmakuTextInputViewHeight;
        message_view_->SetBounds(0, 0, 0, 0);
        danmaku_viewport_->SetBounds(content_bounds.x(), y, content_bounds.width(), danmaku_viewport_height);
    }
    if (guide_view_)
    {
        guide_view_->SetBounds(0, 0, content_bounds.width(), content_bounds.height());
        guide_view_->SetBounds(0, 0, content_bounds.width(), content_bounds.height());
    }

}

void DanmakuInteractionViddupView::ReloadInputView()
{
    if (GetShowType() == DanmakuShowType::kMerged)
    {
        RemoveChildView(text_input_view_);
        InitInputView();
        AddChildView(text_input_view_);
        Layout();
    }
    else if (GetShowType() == DanmakuShowType::kSplited)
    {
        dmk_edit_view_->SetExLayeredEnable(true, view_bk_opacity_);
        dmk_edit_view_->SetDrawBackgroundEnable(true);
    }
}

void DanmakuInteractionViddupView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (send_dmk_button_ == sender)
    {
        SendLiveDanmaku();
    }
}

void DanmakuInteractionViddupView::OnDanmakuSent(bool valid_response, bool sending_succeeded, const string16& msg)
{
    send_dmk_button_->SetEnabled(true);
    dmk_edit_view_->SetEnabled(true);

    if (valid_response)
    {
        if (!sending_succeeded && !msg.empty())
        {
            dmkhime::ShowDmkhimeToast(
                GetWidget(), GetBoundsInScreen(), dmkhime::DmkToastType::Default, msg);
        }
        else
        {
            dmk_edit_view_->Clear();
        }
    }
    else
    {
        auto& rb = ResourceBundle::GetSharedInstance();
        dmkhime::ShowDmkhimeToast(
            GetWidget(), GetBoundsInScreen(), dmkhime::DmkToastType::Default, rb.GetLocalizedString(IDS_DANMAKU_INTERFACTION_SEND_FAIL));
    }
}

void DanmakuInteractionViddupView::AddAnchorSendLocalMessage(const string16& msg)
{
    auto& rb = ResourceBundle::GetSharedInstance();
    string16 local = rb.GetLocalizedString(IDS_DANMAKUHIME_FLAG_LOCAL);   

    auto uid = GetBililiveProcess()->secret_core()->account_info().mid();
    auto room_id = GetBililiveProcess()->secret_core()->user_info().room_id();
    DanmakuData data;
    data.user_name = base::UTF8ToUTF16(GetBililiveProcess()->secret_core()->user_info().nickname());
    data.plain_text = msg;
    data.type = DT_DANMU_MSG;
    data.user_id = uid;
    //data.flags = std::move(flags);
    AddDanmaku(data);
}

void DanmakuInteractionViddupView::SwitchThemeImpl()
{
    view_bk_color_ = GetDanmakuThemeColor(GetTheme(), DanmakuThemeViewType::kBackground);
    title_view_->SwitchTheme(GetTheme());
    if (GetShowType() == DanmakuShowType::kSplited)
    {
        dmk_edit_view_->SetBackgroundColor(view_bk_color_);
        auto text_color = GetDanmakuThemeColor(GetTheme(), DanmakuThemeViewType::kText);
        dmk_edit_view_->SetTextColor(text_color);
        dmk_edit_view_->SetFocus();
    }

    OnOpacityChanged();
}

void DanmakuInteractionViddupView::SwitchShowTypeImpl()
{
}

void DanmakuInteractionViddupView::SendLiveDanmaku()
{
    if (!dmk_edit_view_->IsEmptyText())
    {
        auto text = dmk_edit_view_->GetText();
        send_dmk_presenter_.SendDanmaku(text);
        send_dmk_button_->SetEnabled(false);
        dmk_edit_view_->SetEnabled(false);
    }
}

void DanmakuInteractionViddupView::RelocationMessageLoopView()
{
    auto rt = GetContentsBounds();
    if (rt.width() == 0 && rt.height() == 0)
    {
        return;
    }
}

void DanmakuInteractionViddupView::OnOpacityChanged()
{
    if (GetShowType() == DanmakuShowType::kSplited)
    {
        dmk_edit_view_->SetExLayeredEnable(true, view_bk_opacity_);
    }
    send_dmk_button_->SetOpacity(view_bk_opacity_);

    // ClearType Problems arise when drawing on translucent surfaces. So turn off subpixel rendering at translucency
    bool no_sp = (view_bk_opacity_ < 255);
    title_view_->SetNoSubpixelRendering(no_sp);
    send_dmk_button_->set_no_subpixel_rendering(no_sp);

    SchedulePaint();
}

void DanmakuInteractionViddupView::OnLockMouseDect()
{
   
}

views::View* DanmakuInteractionViddupView::GetEventHandlerForPoint(const gfx::Point& point)
{
    auto handle_view = __super::GetEventHandlerForPoint(point);
    return handle_view;
}

void DanmakuInteractionViddupView::OnWidgetVisibilityChanged(views::Widget* widget, bool visible)
{

}

void DanmakuInteractionViddupView::OnWidgetBoundsChanged(views::Widget* widget, const gfx::Rect& new_bounds)
{
    __super::OnWidgetBoundsChanged(widget, new_bounds);

    RelocationMessageLoopView();
}

void DanmakuInteractionViddupView::Paint(gfx::Canvas* canvas)
{
    __super::Paint(canvas);
}

void DanmakuInteractionViddupView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
    __super::ViewHierarchyChanged(details);

    if (details.child == this)
    {
        RelocationMessageLoopView();
    }
}

void DanmakuInteractionViddupView::OnOutgoing(bool first)
{
    SchedulePaint();
}

void DanmakuInteractionViddupView::OnAutism()
{
    
}

bool DanmakuInteractionViddupView::OnMouseDragged(const ui::MouseEvent& event)
{
    bool temp = __super::OnMouseDragged(event);

    return temp;
}