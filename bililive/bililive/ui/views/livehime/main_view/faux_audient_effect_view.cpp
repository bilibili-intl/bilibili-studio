#include "faux_audient_effect_view.h"

#include "base/strings/utf_string_conversions.h"

#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"
#include "bililive/bililive/ui/views/controls/bililive_floating_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/preview/livehime_obs_preview_control.h"
#include "bililive/bililive/utils/net_util.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/public/anchor_info.h"


namespace
{
    const int kLandscapeAvatarSize = 26;
    const int kPortraitAvatarSize = 20;

    class FauxAudientEffectImageView : public views::View
    {
        class UserInfoView : public views::View
        {
        public:
            UserInfoView()
            {
                avatar_view_ = new LivehimeCircleImageView();
                avatar_view_->SetImageBinary(GetBililiveProcess()->secret_core()->user_info().avatar().data(),
                    GetBililiveProcess()->secret_core()->user_info().avatar().size());
                avatar_view_->SetPreferredSize(GetSizeByImgDPIScale(gfx::Size(kLandscapeAvatarSize, kLandscapeAvatarSize)));

                name_label_ = new BililiveLabel();
                name_label_->SetTextColor(SK_ColorWHITE);
                name_label_->SetFont(ftTen);
                name_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);

                fans_label_ = new BililiveLabel();
                fans_label_->SetTextColor(SK_ColorWHITE);
                fans_label_->SetFont(ftTen);
                fans_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
                fans_label_->SetImageSize(GetSizeByImgDPIScale(gfx::Size(kLandscapeAvatarSize / 2, kLandscapeAvatarSize / 2)));
                fans_label_->SetImage(GetImageSkiaNamed(IDR_LIVEHIME_FAUX_AUDIENT_EFFECT_AUDIENCE));

                AddChildView(avatar_view_);
                AddChildView(name_label_);
                AddChildView(fans_label_);
            }

            void SetOrientation(bool landscape)
            {
                is_landscape_ = landscape;
                if (landscape)
                {
                    avatar_view_->SetPreferredSize(GetSizeByImgDPIScale(gfx::Size(kLandscapeAvatarSize, kLandscapeAvatarSize)));
                    fans_label_->SetImageSize(GetSizeByImgDPIScale(gfx::Size(kLandscapeAvatarSize / 2, kLandscapeAvatarSize / 2)));
                }
                else
                {
                    avatar_view_->SetPreferredSize(GetSizeByImgDPIScale(gfx::Size(kPortraitAvatarSize, kPortraitAvatarSize)));
                    fans_label_->SetImageSize(GetSizeByImgDPIScale(gfx::Size(kPortraitAvatarSize / 2, kPortraitAvatarSize / 2)));
                }
                PreferredSizeChanged();
            }

            void UpdateData()
            {
                avatar_view_->SetImageBinary(GetBililiveProcess()->secret_core()->user_info().avatar().data(),
                    GetBililiveProcess()->secret_core()->user_info().avatar().size());

                name_label_->SetText(base::UTF8ToUTF16(GetBililiveProcess()->secret_core()->user_info().nickname()));

                std::wstring str;
                int64_t attention = GetBililiveProcess()->bililive_obs()->broadcast_service()->attention();
                if (attention < 10000)
                {
                    str = std::to_wstring(attention);
                }
                else
                {
                    str = std::to_wstring(attention / 10000) + GetLocalizedString(IDS_STATUS_AUDIENCE_UNIT_1E4);
                }
                fans_label_->SetText(str);
            }

            // View
            gfx::Size GetPreferredSize() override
            {
                gfx::Size pref_size = avatar_view_->GetPreferredSize();
                if (is_landscape_)
                {
                    pref_size.set_width(pref_size.width() + kPaddingColWidthForCtrlTips + GetLengthByImgDPIScale(60));
                }
                else
                {
                    pref_size.set_width(pref_size.width() + GetLengthByDPIScale(4) + GetLengthByImgDPIScale(50));
                }
                return pref_size;
            }
        protected:
            //View
            void Layout() override
            {
                gfx::Rect rect = GetLocalBounds();
                gfx::Size pref_size = avatar_view_->GetPreferredSize();
                avatar_view_->SetBounds(0, 0, pref_size.width(), pref_size.height());

                int padding = is_landscape_ ? kPaddingColWidthForCtrlTips : GetLengthByDPIScale(4);
                int label_x = avatar_view_->bounds().right() + padding;
                int label_cy = rect.height() / 2;
                int label_cx = rect.width() - label_x;
                pref_size = name_label_->GetPreferredSize();
                name_label_->SetBounds(label_x, (label_cy - pref_size.height()) / 2,
                    label_cx, pref_size.height());
                fans_label_->SetBounds(label_x, label_cy + (label_cy - pref_size.height()) / 2,
                    label_cx, pref_size.height());
            }

        private:
            bool is_landscape_ = true;
            LivehimeCircleImageView* avatar_view_ = nullptr;
            BililiveLabel* name_label_ = nullptr;
            BililiveLabel* fans_label_ = nullptr;
        };

    public:
        FauxAudientEffectImageView()
        {
            landscape_img_ = GetImageSkiaNamed(IDR_LIVEHIME_FAUX_AUDIENT_EFFECT_LANDSCAPE);
            portrait_img_ = GetImageSkiaNamed(IDR_LIVEHIME_FAUX_AUDIENT_EFFECT_PORTRAIT);
            info_view_ = new UserInfoView();
            AddChildView(info_view_);
        }

        void SetOrientation(bool landscape)
        {
            is_landscape_ = landscape;
            info_view_->SetOrientation(landscape);
            PreferredSizeChanged();
        }

        void UpdateData()
        {
            info_view_->UpdateData();
        }

        // View
        gfx::Size GetPreferredSize() override
        {
            return (is_landscape_ ? landscape_img_->size() : portrait_img_->size());
        }

    protected:
        // View
        void OnPaintBackground(gfx::Canvas* canvas) override
        {
            gfx::ImageSkia* img = (is_landscape_ ? landscape_img_ : portrait_img_);
            canvas->DrawImageInt(*img, 0, 0, img->width(), img->height(),
                0, 0, width(), height(), true);
        }

        void Layout() override
        {
            gfx::Size size = info_view_->GetPreferredSize();
            if (is_landscape_)
            {
                info_view_->SetBounds(GetLengthByImgDPIScale(40), GetLengthByImgDPIScale(28),
                    size.width(), size.height());
            }
            else
            {
                info_view_->SetBounds(GetLengthByImgDPIScale(6), GetLengthByImgDPIScale(20),
                    size.width(), size.height());
            }
        }

    protected:
        bool is_landscape_ = true;
        gfx::ImageSkia* landscape_img_ = nullptr;
        gfx::ImageSkia* portrait_img_ = nullptr;
        UserInfoView* info_view_ = nullptr;
    };

    class LayoutSuggestionButton : public BililiveLabelButton
    {
    public:
        LayoutSuggestionButton(views::ButtonListener* listener, const string16& text)
            : BililiveLabelButton(listener, text)
        {
            static BililiveLabelButton::ColorStruct clrs;

            static std::once_flag reg_btn_style_flag_;
            std::call_once(reg_btn_style_flag_, []()
                {
                    clrs.set_disable = false;
                    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = SkColorSetA(SK_ColorWHITE, kHoverMaskAlpha);
                    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = SkColorSetA(SK_ColorBLACK, kHoverMaskAlpha);;
                    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorWHITE;

                    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetA(SK_ColorWHITE, kInactiveAlpha);
                    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetA(SK_ColorBLACK, kHoverMaskAlpha);
                    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = SK_ColorWHITE;

                    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetA(SK_ColorWHITE, kInactiveAlpha);
                    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetA(SK_ColorBLACK, kHoverMaskAlpha);
                    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = SK_ColorWHITE;
                }
            );

            SetStyle(clrs, false);
            SetFont(ftTwelve);
        }
    };
}

FauxAudientEffectView::FauxAudientEffectView()
{
}

void FauxAudientEffectView::SetOrientation(bool landscape)
{
    landscape_ = landscape;
    faux_img_view_->SetOrientation(landscape);
    // 竖屏预览下不展示布局建议
    layout_suggestion_button_->SetVisible(landscape);
    Layout();
}

void FauxAudientEffectView::ViewHierarchyChanged(const ViewHierarchyChangedDetails &details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();
        }
    }
}

void FauxAudientEffectView::InitViews()
{
    set_background(views::Background::CreateSolidBackground(76, 76, 76, 255));
    obs_preview_control_ = new LivehimeOBSPreviewControl(obs_proxy_ui::PreviewType::PREVIEW_MAIN_PREVIEW);

    faux_img_view_ = new FauxAudientEffectImageView();
    floating_view_ = new BililiveFloatingView(nullptr, faux_img_view_, false);

    layout_suggestion_button_ = new LayoutSuggestionButton(this, GetLocalizedString(IDS_TABAREA_MODEL_LAYOUT_SUGGESTION));

    AddChildView(obs_preview_control_);
    AddChildView(floating_view_);
    AddChildView(layout_suggestion_button_);
}

void FauxAudientEffectView::Layout()
{
    auto bounds = GetContentsBounds();

    gfx::Size pref_size = faux_img_view_->GetPreferredSize();
    int cx = pref_size.width();
    int cy = pref_size.height();
    static float preview_scale = 9 * 1.0f / 16;

    gfx::Size sugg_size = layout_suggestion_button_->GetPreferredSize();
    int faux_need_length = pref_size.width() / 2 + sugg_size.width() + kPaddingRowHeightForGroupCtrls;
    if (landscape_ && bounds.width()/2 < faux_need_length)
    {

        cx = bounds.width() - sugg_size.width()*2 - kPaddingRowHeightForGroupCtrls;
        cy = cx * preview_scale;
    }
    if (!landscape_ && bounds.height() < pref_size.height())
    {
        cy = bounds.height();
        cx = cy * preview_scale;
    }

    floating_view_->SetBounds(
        bounds.x() + (bounds.width() - cx) / 2,
        bounds.y() + (bounds.height() - cy) / 2,
        cx, cy);

    obs_preview_control_->SetBoundsRect(floating_view_->bounds());

    pref_size = layout_suggestion_button_->GetPreferredSize();
    layout_suggestion_button_->SetBounds(
        floating_view_->bounds().right() + kPaddingRowHeightForCtrlTips,
        floating_view_->bounds().y() + kPaddingColWidthForGroupCtrls,
        pref_size.width(), pref_size.height());
}

void FauxAudientEffectView::VisibilityChanged(View* starting_from, bool is_visible)
{
    if (is_visible)
    {
        faux_img_view_->UpdateData();
    }
}

void FauxAudientEffectView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    bililive::ShellExecute(bililive::ExecURLType::LAYOUT_MODEL_SUGGESTION);
}
