#pragma once

#include "module_common_view.hpp"

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
    std::string button_url;             //允许用户自定义icon等
    std::string background_color;       //暂不支持背景色，Title_label为文字颜色
    base::string16 title_name;
};

class ModuleCommonTitleDelegate {
public:
    ModuleCommonTitleDelegate() {};
    virtual ~ModuleCommonTitleDelegate() {};
    virtual void FollowCloudTitleMove() {};
    virtual void OnButtonPressed(CommonType button_type) {};
    virtual void SendMouseEvent(const ui::MouseEvent& event) {};
};

class ModuleCommonTitleView:
    public ModuleCommonView<ModuleCommonTitleView>,
    public views::ButtonListener
{
public:
    ModuleCommonTitleView();
    ~ModuleCommonTitleView();

    void SetDelegate(ModuleCommonTitleDelegate* delegate,bool opaque = false);

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
    ModuleCommonTitleDelegate* delegate_ = nullptr;

    std::vector<CommonType> v_com_type_;
    bool opaque_ = false;

    BililiveLabel* title_label_ = nullptr;
    LivehimeImageView* icon_btn_ = nullptr;
    BililiveImageButton* help_btn_ = nullptr;
    BililiveImageButton* set_btn_ = nullptr;
    BililiveImageButton* close_btn_ = nullptr;
    LivehimeImageView* custom_close_btn_ = nullptr;         //H5自定义关闭按钮
    bool custom_close_ = false;
    LivehimeImageView* custom_btn_ = nullptr;           //运行业务方自定义button
};

class SecondaryWindowTitleView :
    public ModuleCommonView<SecondaryWindowTitleView>,
    public views::ButtonListener
{
public:
    SecondaryWindowTitleView();
    ~SecondaryWindowTitleView();

    void SetDelegate(ModuleCommonTitleDelegate* delegate, bool opaque = false);

    void InitView();
    void ChangeBackGround(const std::string& color_value);
    void SetTitleContent(const base::string16& title, const std::string& title_color = "", const std::string& icon = "", bool re_layout = true);
    void SetTitleBar(const CefTitleBar& cef_title_bar);

protected:
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    bool OnMouseDragged(const ui::MouseEvent& event) override;
    bool OnMousePressed(const ui::MouseEvent& event) override;
    void OnMouseReleased(const ui::MouseEvent& event) override;
    void Layout() override;

private:
    bool ControlCommonType(CommonType common_type, TitleCtl title_ctl = TitleCtl::Find_Ctl);
    int GetColorValue(const std::string& color_value);
private:
    ModuleCommonTitleDelegate* delegate_ = nullptr;

    std::vector<CommonType> v_com_type_;
    bool opaque_ = false;

    BililiveLabel* title_label_ = nullptr;
    LivehimeImageView* icon_btn_ = nullptr;
    BililiveImageButton* help_btn_ = nullptr;
    BililiveImageButton* set_btn_ = nullptr;
    BililiveImageButton* close_btn_ = nullptr;
    LivehimeImageView* custom_close_btn_ = nullptr;         //H5自定义关闭按钮
    bool custom_close_ = false;
    LivehimeImageView* custom_btn_ = nullptr;           //运行业务方自定义button
};