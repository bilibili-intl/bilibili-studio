#pragma once

#include "bililive/bililive/ui/views/livehime/web_browser/common/module_common_view.hpp"

#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/browser/livehime_web_browser_view.h"


class LivehimeImageView;
struct CefTitleBar;

enum class CommonType {
    Title_Icon = 0,
    Title_label,
    Button_Close,
    Button_Set,
    Button_Help,
    Button_Custom,
    Custom_Close
};

enum class TitleCtl {
    Find_Ctl = 0,
    Add_Ctl,
    Del_Ctl
};

struct TitleBar {
    CommonType button_type;
    std::string button_url;             //�����û��Զ���icon��
    std::string background_color;       //�ݲ�֧�ֱ���ɫ��Title_labelΪ������ɫ
    base::string16 title_name;
};

class GiftPanelTitleDelegate {
public:
    GiftPanelTitleDelegate() {};
    virtual ~GiftPanelTitleDelegate() {};
    virtual void FollowCloudTitleMove() {};
    virtual void OnButtonPressed(CommonType button_type) {};
    virtual void SendMouseEvent(const ui::MouseEvent& event) {};
};

class GiftPanelTitleView:
    public ModuleCommonView<GiftPanelTitleView>,
    public views::ButtonListener
{
public:
    GiftPanelTitleView();
    ~GiftPanelTitleView();

    void SetDelegate(GiftPanelTitleDelegate* delegate,bool opaque = false);

    void InitView();
    void ChangeBackGround(const std::string& color_value);
    void SetTitleContent(const base::string16& title,const std::string& title_color = "", const std::string& icon = "",bool re_layout = true);
    void SetTitleBar(const CefTitleBar& cef_title_bar);

protected:
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    bool OnMouseDragged(const ui::MouseEvent& event) override;
    bool OnMousePressed(const ui::MouseEvent& event) override;
    void OnMouseReleased(const ui::MouseEvent& event) override;
    void OnOpacityChanged()override;
    void Layout() override;

private:
    bool ControlCommonType(CommonType common_type, TitleCtl title_ctl = TitleCtl::Find_Ctl);
    int GetColorValue(const std::string& color_value);
private:
    GiftPanelTitleDelegate* delegate_ = nullptr;

    std::vector<CommonType> v_com_type_;
    bool opaque_ = false;

    BililiveLabel* title_label_ = nullptr;
    LivehimeImageView* icon_btn_ = nullptr;
    BililiveImageButton* help_btn_ = nullptr;
    BililiveImageButton* set_btn_ = nullptr;
    BililiveImageButton* close_btn_ = nullptr;
    LivehimeImageView* custom_close_btn_ = nullptr;         //H5�Զ���رհ�ť
    bool custom_close_ = false;
    LivehimeImageView* custom_btn_ = nullptr;           //����ҵ���Զ���button
};