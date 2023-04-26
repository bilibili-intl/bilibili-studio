#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_PROPERTY_BASE_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_PROPERTY_BASE_VIEW_H_

#include "ui/views/view.h"

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_combobox.h"
#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"

#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"


class LivehimeSrcPropCombobox
    : public LivehimeCombobox
{
public:
    LivehimeSrcPropCombobox(){}

    virtual ~LivehimeSrcPropCombobox(){}

    template<typename T>
    void LoadList(const PropertyList<T>& list)
    {
        ClearItems();
        for (auto& x : list)
            AddItem(std::get<0>(x), std::get<1>(x));
        if (GetItemCount() > 0 && selected_index() < 0)
            SetSelectedIndex(0);
    }

    template<typename T>
    bool SetSelectedData(const T& item_data)
    {
        int index = FindItemData(item_data);
        if (index >= 0)
        {
            SetSelectedIndex(index);
            return true;
        }
        else
            return false;
    }

    template<typename T>
    T GetSelectedData()
    {
        T data;
        if (GetItemCount() > 0)
        {
            int index = selected_index();
            if (index >= 0)
            {
                data = GetItemData<T>(index);
            }
        }

        return data;
    }
};

namespace livehime
{

enum ViewType{
    VT_CREATESOURCE = 0,    // 默认是首次创建源
    VT_SETPROPERTY,         // 设置属性
};

enum ResultCode{
    RC_CANCEL = 0,  // 点x的时候返回的默认值是这个！
    RC_OK,
};

enum class SourceMaterialType : size_t {
    UNKNOW = 0,
    WINDOW_MATERIAL = 3,
    VIDEO_CAPTURE_MATERIAL = 7,
    BROWSER_MATERIAL = 10,
};

enum ProjectionScreenType {
     PT_WIRELESSS = 0, //无线投屏
     PT_WIRED_IOS,     //IOS有线投屏
};
class BasePropertyData
{
public:
    // 布局UI
    virtual void InitView(){}

    virtual void UninitView(){}

    // 初始化UI数据
    virtual void InitData(){}

    // 检测信息数据有效性
    virtual bool CheckSetupValid(){ return false; }

    // 保存设置
    virtual bool SaveSetupChange(){ return false; }

    // 取消设置
    virtual bool Cancel(){ return false; }

    // 确认修改，即将关窗
    virtual void PostSaveSetupChange(obs_proxy::SceneItem* scene_item);

protected:
    BasePropertyData();

    virtual ~BasePropertyData() = default;

    int MaxLabelWidth() const { return max_label_width_; }

private:
    int max_label_width_;
};

class BasePropertyView
    : public BasePropertyData,
      public views::View
{
public:
    BasePropertyView();

    virtual ~BasePropertyView() = default;

    virtual gfx::ImageSkia* GetSkiaIcon(){ return nullptr; }

    virtual std::wstring GetCaption(){ return std::wstring(); }

    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override
    {
        if (details.child == this)
        {
            if (details.is_add)
            {
                InitView();
                InitData();
            }
            else
            {
                UninitView();
            }
        }
    };

protected:
    int MaxControlWidth() const { return max_control_width_; }

    int NonHeaderColumnPadding() const { return non_header_column_padding_; }

private:
    int max_control_width_;
    int non_header_column_padding_;
};

class BaseScrollbarView
    : public BasePropertyData,
    public BililiveViewWithFloatingScrollbar
{
public:
    BaseScrollbarView();

    virtual ~BaseScrollbarView() = default;

    virtual gfx::ImageSkia* GetSkiaIcon() { return nullptr; }

    virtual std::wstring GetCaption() { return std::wstring(); }

    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override
    {
        if (details.child == this)
        {
            if (details.is_add)
            {
                InitView();
                InitData();
            }
            else
            {
                UninitView();
            }
        }
    };

protected:
    int MaxControlWidth() const { return max_control_width_; }

    int NonHeaderColumnPadding() const { return non_header_column_padding_; }

private:
    int max_control_width_;
    int non_header_column_padding_;
};

class LivehimeSrcPropView :
    public BililiveWidgetDelegate,
    public views::ButtonListener,
    BililiveFrameTitleBarDelegate
{
public:
    LivehimeSrcPropView(BasePropertyView* content_view, obs_proxy::SceneItem* scene_item);

    ~LivehimeSrcPropView() override;
protected:
    // WidgetDelegate
    views::View* GetContentsView() override { return this; }

    void WindowClosing() override;

    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }

    // BililiveWidgetDelegate
    void OnCreateNonClientFrameView(views::NonClientFrameView *non_client_frame_view) override;

    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;

    // BililiveFrameTitleBarDelegate
    bool OnTitleBarButtonPressed(TitleBarButton button) override;

private:
    void InitView();
    void InitData();
    void EnableButtons(bool enable);

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    void CheckSourceTextSensitive(bool is_sensitive);

private:
    obs_proxy::SceneItem* scene_item_;

    BasePropertyView* content_view_;
    BililiveLabelButton* ok_button_;
    BililiveLabelButton* cancel_button_;

    DISALLOW_COPY_AND_ASSIGN(LivehimeSrcPropView);
};

BasePropertyView* CreateCameraSourcePropertyDetailView(obs_proxy::SceneItem*,const std::string&);
BasePropertyView* CreateTextSourcePropertyDetailView(obs_proxy::SceneItem*);
BasePropertyView* CreateWindowSourcePropertyDetailView(obs_proxy::SceneItem*);
BasePropertyView* CreateMonitorSourcePropertyDetailView(obs_proxy::SceneItem*);
BasePropertyView* CreateMediaSourcePropertyDetailView(obs_proxy::SceneItem*);
BasePropertyView* CreateAlbumSourcePropertyDetailView(obs_proxy::SceneItem*);
BasePropertyView* CreateImageSourcePropertyDetailView(obs_proxy::SceneItem*);
BasePropertyView* CreateColorSourcePropertyDetailView(obs_proxy::SceneItem*);

BasePropertyView* CreateBrowserSourcePropertyDetailView(obs_proxy::SceneItem*);

// 打开源属性对话框
void OpenSourcePropertyWidget(obs_proxy::SceneItemType type, views::Widget* parent,
                              obs_proxy::SceneItem* scene_item, ViewType view_type = VT_CREATESOURCE,
                              const EndDialogSignalHandler *handler = nullptr, int projection_type = 0,bool from_preset_matrial = false, const std::string& tab_type = "beauty");

}   // namespace livehime

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_PROPERTY_BASE_VIEW_H_
