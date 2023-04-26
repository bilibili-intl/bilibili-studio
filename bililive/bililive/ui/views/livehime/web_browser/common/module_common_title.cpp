#include "module_common_title.h"

#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/livehime/cef/cef_callback_data_presenter.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"


namespace {
    const int kButtonSpace = GetLengthByDPIScale(6);
    const int kBorderSpace = GetLengthByDPIScale(12);
    const int kCustomWidth = 22;

    const std::string kBackGroundColor = "37384b";

    class CommonCustomButton : public LivehimeImageView {
    public:
        CommonCustomButton(views::ButtonListener* listener)
            : LivehimeImageView(listener){
        };

    protected:
            void OnPaintBackground(gfx::Canvas* canvas) override
            {
                __super::OnPaintBackground(canvas);

                // 边框
                if (is_mouse_entered_) {
                    auto bound = GetContentsBounds();
                    SkPaint paint;
                    paint.setAntiAlias(true);
                    paint.setColor(SkColorSetARGB(30,0x00,0x00,0x00));
                    bililive::DrawRoundRect(canvas, 0, 0, width(), height(), GetLengthByDPIScale(2), paint);
                }
            }

            void OnMouseEntered(const ui::MouseEvent& event) override{
                is_mouse_entered_ = true;
                SchedulePaint();
            }

            void OnMouseExited(const ui::MouseEvent& event) override{
                is_mouse_entered_ = false;
                SchedulePaint();
            }

    private:
        bool is_mouse_entered_ = false;
    };
}

ModuleCommonTitleView::ModuleCommonTitleView()
{
}

ModuleCommonTitleView::~ModuleCommonTitleView()
{
}

void ModuleCommonTitleView::SetDelegate(ModuleCommonTitleDelegate* delegate, bool opaque)
{
    delegate_ = delegate;
    opaque_ = opaque;
    if (opaque) {
        ChangeBackGround(kBackGroundColor);
    }
    InitView();
}

void ModuleCommonTitleView::ChangeBackGround(const std::string& color_value)
{
    if (!opaque_) {
        return;
    }
    int color = GetColorValue(color_value);
    if (color != 0) {
        SetBackGroundColor(color);
        SchedulePaint();
    }
}

int ModuleCommonTitleView::GetColorValue(const std::string& color_value)
{
    int color = 0;
    if (base::HexStringToInt(color_value, &color) && color_value!="0")
    {
        color |= 0xFF000000;
    }
    return color;
}

void ModuleCommonTitleView::SetTitleContent(const base::string16& title, const std::string& title_color, const std::string& icon, bool re_layout)
{
    if (!title.empty()) {
        title_label_->SetText(title);
        ControlCommonType(CommonType::Title_label, TitleCtl::Add_Ctl);
    }

    if (!title_color.empty()) {
        int color = GetColorValue(title_color);
        if (color != 0) {
            title_label_->SetTextColor(color);
        }
    }

    if (!icon.empty()) {
        icon_btn_->SetImageUrl(icon);
        ControlCommonType(CommonType::Title_Icon, TitleCtl::Add_Ctl);
    }

    if (re_layout) {
        InvalidateLayout();
        Layout();
    }
}

void ModuleCommonTitleView::SetTitleBar(const CefTitleBar& cef_title_bar)
{
    //设置标题栏
    SetTitleContent(cef_title_bar.title, cef_title_bar.title_color, cef_title_bar.icon,false);
    if (cef_title_bar.clear_title) {
        ControlCommonType(CommonType::Title_label, TitleCtl::Del_Ctl);
    }

    if (cef_title_bar.clear_icon) {
        ControlCommonType(CommonType::Title_Icon, TitleCtl::Del_Ctl);
    }

    ChangeBackGround(cef_title_bar.backgroundColor);

    if (cef_title_bar.re_default_btn) {
        ControlCommonType(CommonType::Button_Set, TitleCtl::Del_Ctl);
        ControlCommonType(CommonType::Button_Help, TitleCtl::Del_Ctl);
        ControlCommonType(CommonType::Button_Custom, TitleCtl::Del_Ctl);
        custom_close_ = false;
    }

    for (auto it = cef_title_bar.title_btns.begin(); it != cef_title_bar.title_btns.end(); it++) {
        if (it->tagname == "set") {
            ControlCommonType(CommonType::Button_Set, TitleCtl::Add_Ctl);
        }
        else if (it->tagname == "help") {
            ControlCommonType(CommonType::Button_Help, TitleCtl::Add_Ctl);
        }
        else if (it->tagname == "custom" && !it->url.empty()) {
            custom_btn_->SetImageUrl(base::UTF16ToUTF8(it->url));
            custom_btn_->SetTooltipText(it->text);
            ControlCommonType(CommonType::Button_Custom, TitleCtl::Add_Ctl);
        }
        else if (it->tagname == "close" && !it->url.empty()) {
            if (custom_close_btn_) {
                custom_close_ = true;
                custom_close_btn_->SetImageUrl(base::UTF16ToUTF8(it->url));
            }
        }
    }

    InvalidateLayout();
    Layout();
}

bool ModuleCommonTitleView::ControlCommonType(CommonType common_type, TitleCtl title_ctl)
{
    auto it = std::find_if(v_com_type_.begin(), v_com_type_.end(), [common_type](CommonType type) {
        return common_type == type;
    });
    if (it == v_com_type_.end()) {
        if (title_ctl == TitleCtl::Add_Ctl) {
            v_com_type_.push_back(common_type);
            return true;
        }
        else {
            return false;
        }
    }
    else {
        if (title_ctl == TitleCtl::Del_Ctl) {
            v_com_type_.erase(it);
        }
        return true;
    }
}

void ModuleCommonTitleView::InitView()
{
    title_label_ = new BililiveLabel(L"", ftFourteen);
    if (opaque_) {
        title_label_->SetTextColor(SkColorSetRGB(0xff, 0xff, 0xff));
    }

    icon_btn_ = new LivehimeImageView();
    icon_btn_->SetPreferredSize(GetSizeByDPIScale({ kCustomWidth,kCustomWidth }));

    help_btn_ = new BililiveImageButton(this);
    help_btn_->SetImage(views::Button::STATE_NORMAL, opaque_? GetImageSkiaNamed(IDR_COMMON_TITLE_HELP) : GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_HELP));
    help_btn_->SetImage(views::Button::STATE_PRESSED, opaque_ ? GetImageSkiaNamed(IDR_COMMON_TITLE_HELP_HV) : GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_HELP_HV));
    help_btn_->SetImage(views::Button::STATE_HOVERED, opaque_ ? GetImageSkiaNamed(IDR_COMMON_TITLE_HELP_HV) : GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_HELP_HV));
    help_btn_->set_id(static_cast<int>(CommonType::Button_Help));
    help_btn_->SetTooltipText(GetLocalizedString(IDS_HELP));

    set_btn_ = new BililiveImageButton(this);
    set_btn_->SetImage(views::Button::STATE_NORMAL, opaque_ ? GetImageSkiaNamed(IDR_COMMON_TITLE_SET) : GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_SET));
    set_btn_->SetImage(views::Button::STATE_PRESSED, opaque_ ? GetImageSkiaNamed(IDR_COMMON_TITLE_SET_HV) : GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_SET_HV));
    set_btn_->SetImage(views::Button::STATE_HOVERED, opaque_ ? GetImageSkiaNamed(IDR_COMMON_TITLE_SET_HV) : GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_SET_HV));
    set_btn_->set_id(static_cast<int>(CommonType::Button_Set));
    set_btn_->SetTooltipText(GetLocalizedString(IDS_SET));

    close_btn_ = new BililiveImageButton(this);
    close_btn_->SetImage(views::Button::STATE_NORMAL, opaque_? GetImageSkiaNamed(IDR_COMMON_TITLE_CLOSE): GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_CLOSE));
    close_btn_->SetImage(views::Button::STATE_PRESSED, opaque_ ? GetImageSkiaNamed(IDR_COMMON_TITLE_CLOSE_HV): GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_CLOSE_HV));
    close_btn_->SetImage(views::Button::STATE_HOVERED, opaque_ ? GetImageSkiaNamed(IDR_COMMON_TITLE_CLOSE_HV) : GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_CLOSE_HV));
    close_btn_->set_id(static_cast<int>(CommonType::Button_Close));
    ControlCommonType(CommonType::Button_Close, TitleCtl::Add_Ctl);

    if (!opaque_) {
        custom_close_btn_ = new CommonCustomButton(this);
        custom_close_btn_->set_id(static_cast<int>(CommonType::Custom_Close));
        custom_close_btn_->SetPreferredSize(GetSizeByDPIScale({ kCustomWidth, kCustomWidth }));
        AddChildView(custom_close_btn_);
    }

    custom_btn_ = new CommonCustomButton(this);
    custom_btn_->SetPreferredSize(GetSizeByDPIScale({ kCustomWidth, kCustomWidth }));
    custom_btn_->set_id(static_cast<int>(CommonType::Button_Custom));

    AddChildView(title_label_);
    AddChildView(icon_btn_);

    AddChildView(help_btn_);
    AddChildView(set_btn_);
    AddChildView(close_btn_);
    AddChildView(custom_btn_);
}

void ModuleCommonTitleView::Layout()
{
    gfx::Rect rect(GetContentsBounds());

    int start_x_pos = kBorderSpace;
    gfx::Size size;
    if (ControlCommonType(CommonType::Title_Icon)) {
        icon_btn_->SetVisible(true);
        size = icon_btn_->GetPreferredSize();
        icon_btn_->SetBounds(
            start_x_pos, (rect.height() - size.height()) / 2, size.width(), size.height());
        start_x_pos += kButtonSpace;
        start_x_pos += size.width();
    }
    else {
        icon_btn_->SetVisible(false);
    }

    if (ControlCommonType(CommonType::Title_label)) {
        title_label_->SetVisible(true);
        size = title_label_->GetPreferredSize();
        title_label_->SetBounds(
            start_x_pos, (rect.height() - size.height()) / 2, size.width(), size.height());
    }
    else {
        title_label_->SetVisible(false);
    }

    if (custom_close_) {
        close_btn_->SetVisible(false);
        size = custom_close_btn_->GetPreferredSize();
        start_x_pos = rect.width() - kBorderSpace - size.width();
        custom_close_btn_->SetBounds(start_x_pos, (rect.height() - size.height()) / 2, size.width(), size.height());
    }
    else {
        size = close_btn_->GetPreferredSize();
        start_x_pos = rect.width() - kBorderSpace - size.width();
        close_btn_->SetBounds(start_x_pos, (rect.height() - size.height()) / 2, size.width(), size.height());
    }

    if (ControlCommonType(CommonType::Button_Set)) {
        set_btn_->SetVisible(true);
        size = set_btn_->GetPreferredSize();
        start_x_pos = start_x_pos - kButtonSpace - size.width();
        set_btn_->SetBounds(start_x_pos, (rect.height() - size.height()) / 2, size.width(), size.height());
    }
    else {
        set_btn_->SetVisible(false);
    }

    if (ControlCommonType(CommonType::Button_Help)) {
        help_btn_->SetVisible(true);
        size = help_btn_->GetPreferredSize();
        start_x_pos = start_x_pos - kButtonSpace - size.width();
        help_btn_->SetBounds(start_x_pos, (rect.height() - size.height()) / 2, size.width(), size.height());
    }
    else {
        help_btn_->SetVisible(false);
    }

    if (ControlCommonType(CommonType::Button_Custom)) {
        custom_btn_->SetVisible(true);
        size = custom_btn_->GetPreferredSize();
        start_x_pos = start_x_pos - kButtonSpace - size.width();
        custom_btn_->SetBounds(start_x_pos, (rect.height() - size.height()) / 2, size.width(), size.height());
    }
    else {
        custom_btn_->SetVisible(false);
    }
}

void ModuleCommonTitleView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (!delegate_) {
        return;
    }

    if (sender->id() == static_cast<int>(CommonType::Button_Help)) {
        delegate_->OnButtonPressed(CommonType::Button_Help);
    }
    else if (sender->id() == static_cast<int>(CommonType::Button_Close) || sender->id() == static_cast<int>(CommonType::Custom_Close)) {
        delegate_->OnButtonPressed(CommonType::Button_Close);
    }
    else if (sender->id() == static_cast<int>(CommonType::Button_Set)) {
        delegate_->OnButtonPressed(CommonType::Button_Set);
    }
    else if (sender->id() == static_cast<int>(CommonType::Button_Custom)) {
        delegate_->OnButtonPressed(CommonType::Button_Custom);
    }
}

bool ModuleCommonTitleView::OnMouseDragged(const ui::MouseEvent& event)
{
    __super::OnMouseDragged(event);

    if (delegate_) {
        delegate_->FollowCloudTitleMove();
    }
    return true;
}

bool ModuleCommonTitleView::OnMousePressed(const ui::MouseEvent& event)
{
    __super::OnMousePressed(event);

    delegate_->SendMouseEvent(event);
    return true;
}

void ModuleCommonTitleView::OnMouseReleased(const ui::MouseEvent& event)
{
    __super::OnMouseReleased(event);

    delegate_->SendMouseEvent(event);
}

void ModuleCommonTitleView::OnOpacityChanged()
{

}

SecondaryWindowTitleView::SecondaryWindowTitleView()
{
}

SecondaryWindowTitleView::~SecondaryWindowTitleView()
{
}

void SecondaryWindowTitleView::SetDelegate(ModuleCommonTitleDelegate* delegate, bool opaque)
{
    delegate_ = delegate;
    opaque_ = opaque;
    if (opaque) {
        ChangeBackGround(kBackGroundColor);
    }
    InitView();
}

void SecondaryWindowTitleView::ChangeBackGround(const std::string& color_value)
{
    if (!opaque_) {
        return;
    }
    int color = GetColorValue(color_value);
    if (color != 0) {
        SetBackGroundColor(color);
        SchedulePaint();
    }
}

int SecondaryWindowTitleView::GetColorValue(const std::string& color_value)
{
    int color = 0;
    if (base::HexStringToInt(color_value, &color) && color_value != "0")
    {
        color |= 0xFF000000;
    }
    return color;
}

void SecondaryWindowTitleView::SetTitleContent(const base::string16& title, const std::string& title_color, const std::string& icon,bool re_layout)
{
    if (!title.empty()) {
        title_label_->SetText(title);
        ControlCommonType(CommonType::Title_label, TitleCtl::Add_Ctl);
    }

    if (!title_color.empty()) {
        int color = GetColorValue(title_color);
        if (color != 0) {
            title_label_->SetTextColor(color);
        }
    }

    if (!icon.empty()) {
        icon_btn_->SetImageUrl(icon);
        ControlCommonType(CommonType::Title_Icon, TitleCtl::Add_Ctl);
    }

    if (re_layout) {
        InvalidateLayout();
        Layout();
    }
}

void SecondaryWindowTitleView::SetTitleBar(const CefTitleBar& cef_title_bar)
{
    //设置标题栏
    SetTitleContent(cef_title_bar.title, cef_title_bar.title_color, cef_title_bar.icon,false);
    if (cef_title_bar.clear_title) {
        ControlCommonType(CommonType::Title_label, TitleCtl::Del_Ctl);
    }

    if (cef_title_bar.clear_icon) {
        ControlCommonType(CommonType::Title_Icon, TitleCtl::Del_Ctl);
    }

    ChangeBackGround(cef_title_bar.backgroundColor);

    if (cef_title_bar.re_default_btn) {
        ControlCommonType(CommonType::Button_Set, TitleCtl::Del_Ctl);
        ControlCommonType(CommonType::Button_Help, TitleCtl::Del_Ctl);
        ControlCommonType(CommonType::Button_Custom, TitleCtl::Del_Ctl);
        custom_close_ = false;
    }

    for (auto it = cef_title_bar.title_btns.begin(); it != cef_title_bar.title_btns.end(); it++) {
        if (it->tagname == "set") {
            ControlCommonType(CommonType::Button_Set, TitleCtl::Add_Ctl);
        }
        else if (it->tagname == "help") {
            ControlCommonType(CommonType::Button_Help, TitleCtl::Add_Ctl);
        }
        else if (it->tagname == "custom" && !it->url.empty()) {
            custom_btn_->SetImageUrl(base::UTF16ToUTF8(it->url));
            custom_btn_->SetTooltipText(it->text);
            ControlCommonType(CommonType::Button_Custom, TitleCtl::Add_Ctl);
        }
        else if (it->tagname == "close" && !it->url.empty()) {
            if (custom_close_btn_) {
                custom_close_ = true;
                custom_close_btn_->SetImageUrl(base::UTF16ToUTF8(it->url));
            }
        }
    }

    InvalidateLayout();
    Layout();
}

bool SecondaryWindowTitleView::ControlCommonType(CommonType common_type, TitleCtl title_ctl)
{
    auto it = std::find_if(v_com_type_.begin(), v_com_type_.end(), [common_type](CommonType type) {
        return common_type == type;
    });
    if (it == v_com_type_.end()) {
        if (title_ctl == TitleCtl::Add_Ctl) {
            v_com_type_.push_back(common_type);
            return true;
        }
        else {
            return false;
        }
    }
    else {
        if (title_ctl == TitleCtl::Del_Ctl) {
            v_com_type_.erase(it);
        }
        return true;
    }
}

void SecondaryWindowTitleView::InitView()
{
    title_label_ = new BililiveLabel(L"", ftFourteen);
    if (opaque_) {
        title_label_->SetTextColor(SkColorSetRGB(0xff, 0xff, 0xff));
    }

    icon_btn_ = new LivehimeImageView();
    icon_btn_->SetPreferredSize(GetSizeByDPIScale({ kCustomWidth,kCustomWidth }));

    help_btn_ = new BililiveImageButton(this);
    help_btn_->SetImage(views::Button::STATE_NORMAL, opaque_ ? GetImageSkiaNamed(IDR_COMMON_TITLE_HELP) : GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_HELP));
    help_btn_->SetImage(views::Button::STATE_PRESSED, opaque_ ? GetImageSkiaNamed(IDR_COMMON_TITLE_HELP_HV) : GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_HELP_HV));
    help_btn_->SetImage(views::Button::STATE_HOVERED, opaque_ ? GetImageSkiaNamed(IDR_COMMON_TITLE_HELP_HV) : GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_HELP_HV));
    help_btn_->set_id(static_cast<int>(CommonType::Button_Help));
    help_btn_->SetTooltipText(GetLocalizedString(IDS_HELP));

    set_btn_ = new BililiveImageButton(this);
    set_btn_->SetImage(views::Button::STATE_NORMAL, opaque_ ? GetImageSkiaNamed(IDR_COMMON_TITLE_SET) : GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_SET));
    set_btn_->SetImage(views::Button::STATE_PRESSED, opaque_ ? GetImageSkiaNamed(IDR_COMMON_TITLE_SET_HV) : GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_SET_HV));
    set_btn_->SetImage(views::Button::STATE_HOVERED, opaque_ ? GetImageSkiaNamed(IDR_COMMON_TITLE_SET_HV) : GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_SET_HV));
    set_btn_->set_id(static_cast<int>(CommonType::Button_Set));
    set_btn_->SetTooltipText(GetLocalizedString(IDS_SET));

    close_btn_ = new BililiveImageButton(this);
    close_btn_->SetImage(views::Button::STATE_NORMAL, opaque_ ? GetImageSkiaNamed(IDR_COMMON_TITLE_CLOSE) : GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_CLOSE));
    close_btn_->SetImage(views::Button::STATE_PRESSED, opaque_ ? GetImageSkiaNamed(IDR_COMMON_TITLE_CLOSE_HV) : GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_CLOSE_HV));
    close_btn_->SetImage(views::Button::STATE_HOVERED, opaque_ ? GetImageSkiaNamed(IDR_COMMON_TITLE_CLOSE_HV) : GetImageSkiaNamed(IDR_COMMON_TITLE_GRAY_CLOSE_HV));
    close_btn_->set_id(static_cast<int>(CommonType::Button_Close));
    ControlCommonType(CommonType::Button_Close, TitleCtl::Add_Ctl);

    if (!opaque_) {
        custom_close_btn_ = new CommonCustomButton(this);
        custom_close_btn_->set_id(static_cast<int>(CommonType::Custom_Close));
        custom_close_btn_->SetPreferredSize(GetSizeByDPIScale({ kCustomWidth, kCustomWidth }));
        AddChildView(custom_close_btn_);
    }

    custom_btn_ = new CommonCustomButton(this);
    custom_btn_->SetPreferredSize(GetSizeByDPIScale({ kCustomWidth, kCustomWidth }));
    custom_btn_->set_id(static_cast<int>(CommonType::Button_Custom));

    AddChildView(title_label_);
    AddChildView(icon_btn_);

    AddChildView(help_btn_);
    AddChildView(set_btn_);
    AddChildView(close_btn_);
    AddChildView(custom_btn_);
}

void SecondaryWindowTitleView::Layout()
{
    gfx::Rect rect(GetContentsBounds());

    int start_x_pos = kBorderSpace;
    gfx::Size size;
    if (ControlCommonType(CommonType::Title_Icon)) {
        icon_btn_->SetVisible(true);
        size = icon_btn_->GetPreferredSize();
        icon_btn_->SetBounds(
            start_x_pos, (rect.height() - size.height()) / 2, size.width(), size.height());
        start_x_pos += kButtonSpace;
        start_x_pos += size.width();
    }
    else {
        icon_btn_->SetVisible(false);
    }

    if (ControlCommonType(CommonType::Title_label)) {
        title_label_->SetVisible(true);
        size = title_label_->GetPreferredSize();
        title_label_->SetBounds(
            start_x_pos, (rect.height() - size.height()) / 2, size.width(), size.height());
    }
    else {
        title_label_->SetVisible(false);
    }

    if (custom_close_) {
        close_btn_->SetVisible(false);
        size = custom_close_btn_->GetPreferredSize();
        start_x_pos = rect.width() - kBorderSpace - size.width();
        custom_close_btn_->SetBounds(start_x_pos, (rect.height() - size.height()) / 2, size.width(), size.height());
    }
    else {
        size = close_btn_->GetPreferredSize();
        start_x_pos = rect.width() - kBorderSpace - size.width();
        close_btn_->SetBounds(start_x_pos, (rect.height() - size.height()) / 2, size.width(), size.height());
    }

    if (ControlCommonType(CommonType::Button_Set)) {
        set_btn_->SetVisible(true);
        size = set_btn_->GetPreferredSize();
        start_x_pos = start_x_pos - kButtonSpace - size.width();
        set_btn_->SetBounds(start_x_pos, (rect.height() - size.height()) / 2, size.width(), size.height());
    }
    else {
        set_btn_->SetVisible(false);
    }

    if (ControlCommonType(CommonType::Button_Help)) {
        help_btn_->SetVisible(true);
        size = help_btn_->GetPreferredSize();
        start_x_pos = start_x_pos - kButtonSpace - size.width();
        help_btn_->SetBounds(start_x_pos, (rect.height() - size.height()) / 2, size.width(), size.height());
    }
    else {
        help_btn_->SetVisible(false);
    }

    if (ControlCommonType(CommonType::Button_Custom)) {
        custom_btn_->SetVisible(true);
        size = custom_btn_->GetPreferredSize();
        start_x_pos = start_x_pos - kButtonSpace - size.width();
        custom_btn_->SetBounds(start_x_pos, (rect.height() - size.height()) / 2, size.width(), size.height());
    }
    else {
        custom_btn_->SetVisible(false);
    }
}

void SecondaryWindowTitleView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (!delegate_) {
        return;
    }

    if (sender->id() == static_cast<int>(CommonType::Button_Help)) {
        delegate_->OnButtonPressed(CommonType::Button_Help);
    }
    else if (sender->id() == static_cast<int>(CommonType::Button_Close) || sender->id() == static_cast<int>(CommonType::Custom_Close)) {
        delegate_->OnButtonPressed(CommonType::Button_Close);
    }
    else if (sender->id() == static_cast<int>(CommonType::Button_Set)) {
        delegate_->OnButtonPressed(CommonType::Button_Set);
    }
    else if (sender->id() == static_cast<int>(CommonType::Button_Custom)) {
        delegate_->OnButtonPressed(CommonType::Button_Custom);
    }
}

bool SecondaryWindowTitleView::OnMouseDragged(const ui::MouseEvent& event)
{
    __super::OnMouseDragged(event);

    if (delegate_) {
        delegate_->FollowCloudTitleMove();
    }
    return true;
}

bool SecondaryWindowTitleView::OnMousePressed(const ui::MouseEvent& event)
{
    __super::OnMousePressed(event);

    delegate_->SendMouseEvent(event);
    return true;
}

void SecondaryWindowTitleView::OnMouseReleased(const ui::MouseEvent& event)
{
    __super::OnMouseReleased(event);

    delegate_->SendMouseEvent(event);
}