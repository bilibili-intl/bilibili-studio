#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_SETTINGS_WIDGET_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_SETTINGS_WIDGET_H_

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/livehime/settings/settings_contract.h"

class DanmuSettingWidget
    : public BililiveWidgetDelegate
     ,public contracts::SettingsFrameView
{
public:
    DanmuSettingWidget();
    virtual ~DanmuSettingWidget();

    static void ShowWindow(views::Widget *parent, const gfx::Rect & relative_rect);

    //contracts::SettingsFrameView
    void SaveOrCheckStreamingSettingsChange(bool check, BaseSettingsView::ChangeType& result) override;
	void SaveNormalSettingsChange() override;
    bool ShowRestreamingDialog() override { return true; };
    bool CheckExclusive() override { return true; };
    void ShowLiveReplaySetting(bool publish, bool archive) override {};

protected:
    // WidgetDelegate
    views::View *GetContentsView() override { return this; }
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }
    void WindowClosing() override;

    // views::View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) override;
    gfx::Size GetPreferredSize() override;

private:
    void InitViews();
    BaseSettingsView* danmu_setting_view_ = nullptr;
    std::unique_ptr<contracts::SettingsPresenter> presenter_;
    DISALLOW_COPY_AND_ASSIGN(DanmuSettingWidget);
};


class DanmuGiftSettingWidget
	: public BililiveWidgetDelegate
	, public contracts::SettingsFrameView
{
public:
	DanmuGiftSettingWidget(const base::string16& title);
	virtual ~DanmuGiftSettingWidget();

	static void ShowWindow(views::Widget* parent, const gfx::Rect& relative_rect);

	//contracts::SettingsFrameView
	void SaveOrCheckStreamingSettingsChange(bool check, BaseSettingsView::ChangeType& result) override;
	void SaveNormalSettingsChange() override;
	bool ShowRestreamingDialog() override { return true; };
	bool CheckExclusive() override { return true; };
	void ShowLiveReplaySetting(bool publish, bool archive) override {};

protected:
	// WidgetDelegate
	views::View* GetContentsView() override { return this; }
	ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }
	void WindowClosing() override;

	// views::View
	void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;
	gfx::Size GetPreferredSize() override;

private:
	void InitViews();
	BaseSettingsView* danmu_setting_view_ = nullptr;
	std::unique_ptr<contracts::SettingsPresenter> presenter_;
	DISALLOW_COPY_AND_ASSIGN(DanmuGiftSettingWidget);
};


class ActivityAndTaskWidget
	: public BililiveWidgetDelegate
	, public contracts::SettingsFrameView
{
public:
	ActivityAndTaskWidget();
	virtual ~ActivityAndTaskWidget();

	static void ShowWindow(views::Widget* parent, const gfx::Rect& relative_rect);

	//contracts::SettingsFrameView
	void SaveOrCheckStreamingSettingsChange(bool check, BaseSettingsView::ChangeType& result) override;
	void SaveNormalSettingsChange() override;
	bool ShowRestreamingDialog() override { return true; };
	bool CheckExclusive() override { return true; };
	void ShowLiveReplaySetting(bool publish, bool archive) override {};

protected:
	// WidgetDelegate
	views::View* GetContentsView() override { return this; }
	ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }
	void WindowClosing() override;

	// views::View
	void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;
	gfx::Size GetPreferredSize() override;

private:
	void InitViews();
	BaseSettingsView* danmu_setting_view_ = nullptr;
	std::unique_ptr<contracts::SettingsPresenter> presenter_;
	DISALLOW_COPY_AND_ASSIGN(ActivityAndTaskWidget);
};

//”Ô“Ù≤•±®
class DanmuVoiceBroadcastSettingWidget
	: public BililiveWidgetDelegate
	, public contracts::SettingsFrameView
{
public:
	DanmuVoiceBroadcastSettingWidget();
	virtual ~DanmuVoiceBroadcastSettingWidget();

	static void ShowWindow(views::Widget* parent, const gfx::Rect& relative_rect);

	//contracts::SettingsFrameView
	void SaveOrCheckStreamingSettingsChange(bool check, BaseSettingsView::ChangeType& result) override;
	void SaveNormalSettingsChange() override;
	bool ShowRestreamingDialog() override { return true; };
	bool CheckExclusive() override { return true; };
	void ShowLiveReplaySetting(bool publish, bool archive) override {};

protected:
	// WidgetDelegate
	views::View* GetContentsView() override { return this; }
	ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }
	void WindowClosing() override;

	// views::View
	void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;
	gfx::Size GetPreferredSize() override;

private:
	void InitViews();
	BaseSettingsView* danmu_setting_view_ = nullptr;
	std::unique_ptr<contracts::SettingsPresenter> presenter_;
	DISALLOW_COPY_AND_ASSIGN(DanmuVoiceBroadcastSettingWidget);
};

#endif

