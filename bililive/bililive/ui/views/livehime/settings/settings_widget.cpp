#include "bililive/bililive/ui/views/livehime/settings/settings_widget.h"
#include "ui/views/layout/grid_layout.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_tabbed.h"
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/tabarea/tabarea_materials_view.h"
#include "bililive/bililive/livehime/settings/settings_presenter_impl.h"
#include "bililive/secret/public/event_tracking_service.h"

namespace {
    DanmuSettingWidget* g_danmu_setting_instance = nullptr;
	DanmuGiftSettingWidget* g_danmu_gift_setting_instance = nullptr;
	ActivityAndTaskWidget* g_activity_task_setting_instance = nullptr;
	DanmuVoiceBroadcastSettingWidget* g_voice_broadcast_setting_instance = nullptr;
	const SkColor clrbackground = SkColorSetRGB(0xf9, 0xf9, 0xf9);
}

DanmuSettingWidget::DanmuSettingWidget():BililiveWidgetDelegate(gfx::ImageSkia(), L"��Ļ��������")
{
    presenter_ = std::make_unique<SettingsPresenterImpl>(this);
}

DanmuSettingWidget::~DanmuSettingWidget()
{
    g_danmu_setting_instance = nullptr;
}

void DanmuSettingWidget::ShowWindow(views::Widget * parent, const gfx::Rect& relative_rect)
{
    if (!g_danmu_setting_instance)
    {
		views::Widget* widget = new views::Widget();
		BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(widget);
		g_danmu_setting_instance = new DanmuSettingWidget();
		native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_NONE);
		native_widget->set_insure_show_corner(views::InsureShowCorner::ISC_BOTTOMRIGHT);
		views::Widget::InitParams params(views::Widget::InitParams::TYPE_PANEL);
		params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
		params.native_widget = native_widget;
		params.remove_taskbar_icon = true;
		ShowWidget(g_danmu_setting_instance, widget, params, false,false,views::InsureShowCorner::ISC_NONE);
		livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::DmjDanmuSetting, "");
    }

	int re_pos_x = relative_rect.x();
	int re_pos_y = relative_rect.y();
	gfx::Size s_size = g_danmu_setting_instance->GetPreferredSize();
	s_size.set_height(s_size.height() + GetLengthByDPIScale(30));//GetPreferredSize()���ص�height�������������߶�
	int pos_x = re_pos_x - s_size.width() - GetLengthByDPIScale(5);
	int pos_y = re_pos_y + relative_rect.height() - s_size.height();//�ױ߶���

	HMONITOR monitor = MonitorFromWindow(g_danmu_setting_instance->GetWidget()->GetNativeView(), MONITOR_DEFAULTTONEAREST);
	gfx::Rect monitor_rect(0,0);
	if (monitor)
	{
		MONITORINFO mi = { 0 };
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(monitor, &mi);
		monitor_rect = gfx::Rect(mi.rcMonitor);
	}
	if (pos_x < 0)//������Ļ����ʾ�ڵ�Ļ�����ұ�
	{
		pos_x = re_pos_x + relative_rect.width() + GetLengthByDPIScale(5);
		if (pos_x > monitor_rect.width())//������Ļ
		{
			pos_x = 0;
		}
	}
	if (pos_y < 0)
	{
		pos_y = 0;
	}else if (monitor_rect.height() > 0 && (pos_y + s_size.height() > monitor_rect.height()))
	{
		pos_y = monitor_rect.height() - s_size.height();
	}
	::SetWindowPos(g_danmu_setting_instance->GetWidget()->GetNativeView(), NULL, pos_x, pos_y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	g_danmu_setting_instance->GetWidget()->Show();
	g_danmu_setting_instance->GetWidget()->Activate();
}

void DanmuSettingWidget::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails & details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();
        }
    }
}

void DanmuSettingWidget::InitViews()
{
	this->set_background(views::Background::CreateSolidBackground(clrbackground));
}

gfx::Size DanmuSettingWidget::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
	int x = GetLengthByDPIScale(280);
	int y = GetLengthByDPIScale(580 - 30);
	int offset = GetLengthByDPIScale(5);
	//��֪��Ϊʲô����ͬ��ʾ���űȣ�����ճ��ļ�϶��С���ܴ�������������һ��
	int w = x;
	if (size.width() > x && size.width() < (x + offset))
	{
		w = size.width();
	}
	int h = y;
	if ( y > (size.height() + offset) && size.height() > GetLengthByDPIScale(500))
	{
		h = size.height() + offset;
	}
    size.SetSize(w, h);
    return size;
}

void DanmuSettingWidget::WindowClosing()
{
    BililiveWidgetDelegate::WindowClosing();
	presenter_->SettingChanged();
	presenter_->RequestLivehimeEvent();
}

void DanmuSettingWidget::SaveOrCheckStreamingSettingsChange(bool check, BaseSettingsView::ChangeType& result)
{
	if (danmu_setting_view_)
	{
		danmu_setting_view_->SaveOrCheckStreamingSettingsChange(check, result);
	}
}

void DanmuSettingWidget::SaveNormalSettingsChange() {
	if (danmu_setting_view_)
	{
		danmu_setting_view_->SaveNormalSettingsChange();
	}
}



DanmuGiftSettingWidget::DanmuGiftSettingWidget(const base::string16& title) :BililiveWidgetDelegate(gfx::ImageSkia(), title)
{
	presenter_ = std::make_unique<SettingsPresenterImpl>(this);
}

DanmuGiftSettingWidget::~DanmuGiftSettingWidget()
{
	g_danmu_gift_setting_instance = nullptr;
}

void DanmuGiftSettingWidget::ShowWindow(views::Widget* parent, const gfx::Rect& relative_rect)
{
	if (!g_danmu_gift_setting_instance)
	{
		views::Widget* widget = new views::Widget();
		BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(widget);

		base::string16 rank_text = AppFunctionController::GetInstance()->get_high_rank();
		if (rank_text.empty()) {
			rank_text = L"�����û�&��ˮ��¼����";
		}
		else {
			rank_text = rank_text.append(L"&��ˮ��¼����");
		}
		g_danmu_gift_setting_instance = new DanmuGiftSettingWidget(rank_text);
		native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_NONE);
		native_widget->set_insure_show_corner(views::InsureShowCorner::ISC_BOTTOMRIGHT);
		views::Widget::InitParams params(views::Widget::InitParams::TYPE_PANEL);
		params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
		params.native_widget = native_widget;
		params.remove_taskbar_icon = true;
		ShowWidget(g_danmu_gift_setting_instance, widget, params, false, false, views::InsureShowCorner::ISC_NONE);
		livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::DmjRevenueSetting, "");
	}
	int re_pos_x = relative_rect.x();
	int re_pos_y = relative_rect.y();
	gfx::Size s_size = g_danmu_gift_setting_instance->GetPreferredSize();
	s_size.set_height(s_size.height() + GetLengthByDPIScale(30));//GetPreferredSize()���ص�height�������������߶�
	int pos_x = re_pos_x - s_size.width() - GetLengthByDPIScale(5);
	int pos_y = re_pos_y;//�������

	HMONITOR monitor = MonitorFromWindow(g_danmu_gift_setting_instance->GetWidget()->GetNativeView(), MONITOR_DEFAULTTONEAREST);
	gfx::Rect monitor_rect(0, 0);
	if (monitor)
	{
		MONITORINFO mi = { 0 };
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(monitor, &mi);
		monitor_rect = gfx::Rect(mi.rcMonitor);
	}
	if (pos_x < 0)//������Ļ����ʾ�ڵ�Ļ�����ұ�
	{
		pos_x = re_pos_x + relative_rect.width() + GetLengthByDPIScale(5);
		if (pos_x > monitor_rect.width())//������Ļ
		{
			pos_x = 0;
		}
	}
	if (pos_y < 0)
	{
		pos_y = 0;
	}
	else if (monitor_rect.height() > 0 && (pos_y + s_size.height() > monitor_rect.height()))
	{
		pos_y = monitor_rect.height() - s_size.height();
	}
	::SetWindowPos(g_danmu_gift_setting_instance->GetWidget()->GetNativeView(), NULL, pos_x, pos_y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	g_danmu_gift_setting_instance->GetWidget()->Show();
	g_danmu_gift_setting_instance->GetWidget()->Activate();
}

void DanmuGiftSettingWidget::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
	if (details.child == this)
	{
		if (details.is_add)
		{
			InitViews();
		}
	}
}

void DanmuGiftSettingWidget::InitViews()
{
	this->set_background(views::Background::CreateSolidBackground(clrbackground));
	views::GridLayout* layout = new views::GridLayout(this);
	SetLayoutManager(layout);
	views::ColumnSet* column_set = layout->AddColumnSet(0);
	column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
	layout->StartRow(1.0f, 0);
}

gfx::Size DanmuGiftSettingWidget::GetPreferredSize()
{
	gfx::Size size = __super::GetPreferredSize();
	size.SetSize(GetLengthByDPIScale(280), GetLengthByDPIScale(170 - 30));
	return size;
}

void DanmuGiftSettingWidget::WindowClosing()
{
	BililiveWidgetDelegate::WindowClosing();
	presenter_->SettingChanged();
	presenter_->RequestLivehimeEvent();
}

void DanmuGiftSettingWidget::SaveOrCheckStreamingSettingsChange(bool check, BaseSettingsView::ChangeType& result)
{
	if (danmu_setting_view_)
	{
		danmu_setting_view_->SaveOrCheckStreamingSettingsChange(check, result);
	}
}

void DanmuGiftSettingWidget::SaveNormalSettingsChange() {
	if (danmu_setting_view_)
	{
		danmu_setting_view_->SaveNormalSettingsChange();
	}
}


ActivityAndTaskWidget::ActivityAndTaskWidget() :BililiveWidgetDelegate(gfx::ImageSkia(), L"�&����������")
{
	presenter_ = std::make_unique<SettingsPresenterImpl>(this);
}

ActivityAndTaskWidget::~ActivityAndTaskWidget()
{
	g_activity_task_setting_instance = nullptr;
}

void ActivityAndTaskWidget::ShowWindow(views::Widget* parent, const gfx::Rect& relative_rect)
{
	if (!g_activity_task_setting_instance)
	{
		views::Widget* widget = new views::Widget();
		BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(widget);
		g_activity_task_setting_instance = new ActivityAndTaskWidget();
		native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_NONE);
		native_widget->set_insure_show_corner(views::InsureShowCorner::ISC_BOTTOMRIGHT);
		views::Widget::InitParams params(views::Widget::InitParams::TYPE_PANEL);
		params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
		params.native_widget = native_widget;
		params.remove_taskbar_icon = true;
		ShowWidget(g_activity_task_setting_instance, widget, params, false, false, views::InsureShowCorner::ISC_NONE);
		livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::DmjDanmuSetting, "");
	}
	int re_pos_x = relative_rect.x();
	int re_pos_y = relative_rect.y();
	gfx::Size s_size = g_activity_task_setting_instance->GetPreferredSize();
	s_size.set_height(s_size.height() + GetLengthByDPIScale(30));//GetPreferredSize()���ص�height�������������߶�
	int pos_x = re_pos_x - s_size.width() - GetLengthByDPIScale(5);
	int pos_y = re_pos_y;//�������

	HMONITOR monitor = MonitorFromWindow(g_activity_task_setting_instance->GetWidget()->GetNativeView(), MONITOR_DEFAULTTONEAREST);
	gfx::Rect monitor_rect(0, 0);
	if (monitor)
	{
		MONITORINFO mi = { 0 };
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(monitor, &mi);
		monitor_rect = gfx::Rect(mi.rcMonitor);
	}
	if (pos_x < 0)//������Ļ����ʾ�ڵ�Ļ�����ұ�
	{
		pos_x = re_pos_x + relative_rect.width() + GetLengthByDPIScale(5);
		if (pos_x > monitor_rect.width())//������Ļ
		{
			pos_x = 0;
		}
	}
	if (pos_y < 0)
	{
		pos_y = 0;
	}
	else if (monitor_rect.height() > 0 && (pos_y + s_size.height() > monitor_rect.height()))
	{
		pos_y = monitor_rect.height() - s_size.height();
	}
	::SetWindowPos(g_activity_task_setting_instance->GetWidget()->GetNativeView(), NULL, pos_x, pos_y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	g_activity_task_setting_instance->GetWidget()->Show();
	g_activity_task_setting_instance->GetWidget()->Activate();
}

void ActivityAndTaskWidget::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
	if (details.child == this)
	{
		if (details.is_add)
		{
			InitViews();
		}
	}
}

void ActivityAndTaskWidget::InitViews()
{
	this->set_background(views::Background::CreateSolidBackground(clrbackground));
	views::GridLayout* layout = new views::GridLayout(this);
	SetLayoutManager(layout);
	views::ColumnSet* column_set = layout->AddColumnSet(0);
	column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
}

gfx::Size ActivityAndTaskWidget::GetPreferredSize()
{
	gfx::Size size = __super::GetPreferredSize();

	int view_height = danmu_setting_view_->height();
	size.SetSize(GetLengthByDPIScale(280), view_height);
	return size;
}

void ActivityAndTaskWidget::WindowClosing()
{
	BililiveWidgetDelegate::WindowClosing();
	presenter_->SettingChanged();
	presenter_->RequestLivehimeEvent();
}

void ActivityAndTaskWidget::SaveOrCheckStreamingSettingsChange(bool check, BaseSettingsView::ChangeType& result)
{
	if (danmu_setting_view_){
		danmu_setting_view_->SaveOrCheckStreamingSettingsChange(check, result);
	}
}

void ActivityAndTaskWidget::SaveNormalSettingsChange() {
	if (danmu_setting_view_){
		danmu_setting_view_->SaveNormalSettingsChange();
	}
}
DanmuVoiceBroadcastSettingWidget::DanmuVoiceBroadcastSettingWidget()
	:BililiveWidgetDelegate(gfx::ImageSkia(), L"��������")
{
	presenter_ = std::make_unique<SettingsPresenterImpl>(this);
}

DanmuVoiceBroadcastSettingWidget::~DanmuVoiceBroadcastSettingWidget()
{
	g_voice_broadcast_setting_instance = nullptr;
}

void DanmuVoiceBroadcastSettingWidget::ShowWindow(views::Widget* parent, const gfx::Rect& relative_rect)
{
	if (!g_voice_broadcast_setting_instance)
	{
		views::Widget* widget = new views::Widget();
		BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(widget);
		g_voice_broadcast_setting_instance = new DanmuVoiceBroadcastSettingWidget();
		native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_NONE);
		native_widget->set_insure_show_corner(views::InsureShowCorner::ISC_BOTTOMRIGHT);
		views::Widget::InitParams params(views::Widget::InitParams::TYPE_PANEL);
		params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
		params.native_widget = native_widget;
		params.remove_taskbar_icon = true;
		ShowWidget(g_voice_broadcast_setting_instance, widget, params, false, false, views::InsureShowCorner::ISC_NONE);
		livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::DmjDanmuSetting, "");
	}

	int re_pos_x = relative_rect.x();
	int re_pos_y = relative_rect.y();
	gfx::Size s_size = g_voice_broadcast_setting_instance->GetPreferredSize();
	s_size.set_height(s_size.height() + GetLengthByDPIScale(30));//GetPreferredSize()���ص�height�������������߶�
	int pos_x = re_pos_x - s_size.width() - GetLengthByDPIScale(5);
	int pos_y = re_pos_y + relative_rect.height() - s_size.height();//�ױ߶���

	HMONITOR monitor = MonitorFromWindow(g_voice_broadcast_setting_instance->GetWidget()->GetNativeView(), MONITOR_DEFAULTTONEAREST);
	gfx::Rect monitor_rect(0, 0);
	if (monitor)
	{
		MONITORINFO mi = { 0 };
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(monitor, &mi);
		monitor_rect = gfx::Rect(mi.rcMonitor);
	}
	if (pos_x < 0)//������Ļ����ʾ�ڵ�Ļ�����ұ�
	{
		pos_x = re_pos_x + relative_rect.width() + GetLengthByDPIScale(5);
		if (pos_x > monitor_rect.width())//������Ļ
		{
			pos_x = 0;
		}
	}
	if (pos_y < 0)
	{
		pos_y = 0;
	}
	else if (monitor_rect.height() > 0 && (pos_y + s_size.height() > monitor_rect.height()))
	{
		pos_y = monitor_rect.height() - s_size.height();
	}
	::SetWindowPos(g_voice_broadcast_setting_instance->GetWidget()->GetNativeView(), NULL, pos_x, pos_y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	g_voice_broadcast_setting_instance->GetWidget()->Show();
	g_voice_broadcast_setting_instance->GetWidget()->Activate();
}

void DanmuVoiceBroadcastSettingWidget::SaveOrCheckStreamingSettingsChange(bool check, BaseSettingsView::ChangeType& result)
{
	if (danmu_setting_view_) {
		danmu_setting_view_->SaveOrCheckStreamingSettingsChange(check, result);
	}
}

void DanmuVoiceBroadcastSettingWidget::SaveNormalSettingsChange()
{
	if (danmu_setting_view_) {
		danmu_setting_view_->SaveNormalSettingsChange();
	}
}

void DanmuVoiceBroadcastSettingWidget::WindowClosing()
{
	BililiveWidgetDelegate::WindowClosing();
	presenter_->SettingChanged();
	presenter_->RequestLivehimeEvent();
}

void DanmuVoiceBroadcastSettingWidget::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
	if (details.child == this){
		if (details.is_add){
			InitViews();
		}
	}
}

gfx::Size DanmuVoiceBroadcastSettingWidget::GetPreferredSize()
{
	gfx::Size size = __super::GetPreferredSize();
	size.SetSize(GetLengthByDPIScale(280), GetLengthByDPIScale(349 -30));
	return size;
}

void DanmuVoiceBroadcastSettingWidget::InitViews()
{
	this->set_background(views::Background::CreateSolidBackground(clrbackground));
	views::GridLayout* layout = new views::GridLayout(this);
	SetLayoutManager(layout);
	views::ColumnSet* column_set = layout->AddColumnSet(0);
	column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
	//layout->StartRow(1.0f, 0);
	//danmu_setting_view_ = new DanmakuHimeVoiceBroadcastSettingsView();
	//layout->AddView(danmu_setting_view_->Container());
}
