#include "livehime_user_related_ctrls.h"

#include "base/ext/callable_callback.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/box_layout.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/public/danmaku_hime_service.h"

#include "grit/theme_resources.h"


namespace
{
    const gfx::Size kAvatarSize(GetLengthByDPIScale(32), GetLengthByDPIScale(32));
    const gfx::Size kVerfyFlagSize(GetLengthByDPIScale(12), GetLengthByDPIScale(12));
    const gfx::Size kNftSize(GetSizeByDPIScale({ 14, 14 }));
    const gfx::Insets kAvatarPadding(
        GetLengthByDPIScale(4), GetLengthByDPIScale(4),
        GetLengthByDPIScale(4), GetLengthByDPIScale(4));

    gfx::ImageSkia* GetVerifyTypeFlagIcon(UserOfficalVerifyType verify_type)
    {
        gfx::ImageSkia* image = nullptr;
        switch (verify_type)
        {
        case UserOfficalVerifyType::Official:
            image = ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEMAIN_USER_VERFY_OFFICIAL);
            break;
        case UserOfficalVerifyType::Enterprise:
            image = ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEMAIN_USER_VERFY_ENTERPRISE);
            break;
        default:
            break;
        }
        return image;
    }
}

LivehimeUserPhotoView::LivehimeUserPhotoView(views::ButtonListener* listener/* = nullptr*/)
    : BililiveImageButton(listener)
    , verify_type_(UserOfficalVerifyType::Unverified)
    , weakptr_factory_(this)
{
    BililiveImageButton::SetPreferredSize(kAvatarSize);
    SetStretchPaint(true);
    SetEnabled(false);
    SetEnableGif(true);
    SetAvatar("");
}

LivehimeUserPhotoView::~LivehimeUserPhotoView()
{
}

void LivehimeUserPhotoView::SetAvatarByUrl(const std::string &url)
{
    if (url == url_) {
        return;
    }

    SetAvatar("");
    url_ = url;
    if (url.empty()) {
        return;
    }
    weakptr_factory_.InvalidateWeakPtrs();

    auto callback = base::MakeCallable(base::Bind(
        &LivehimeUserPhotoView::OnAvatarByUrl,
        weakptr_factory_.GetWeakPtr()));

    GetBililiveProcess()->secret_core()->danmaku_hime_service()->DownloadImage(
        url_, callback).Call();
}

void LivehimeUserPhotoView::OnAvatarByUrl(bool valid_response, const std::string& data)
{
    if (valid_response)
    {
        SetAvatar(data);
    }
}

void LivehimeUserPhotoView::SetAvatar(const std::string& avatar)
{
    SetAvatar(reinterpret_cast<const unsigned char*>(avatar.data()), avatar.size());
}

void LivehimeUserPhotoView::SetAvatar(const unsigned char* data, size_t size)
{
    gfx::ImageSkia avatar_img;
    if (data && size > 0)
    {
        avatar_img = bililive::MakeSkiaImage(data, size);
    }
    else
    {
        ResourceBundle &rb = ResourceBundle::GetSharedInstance();
        avatar_img = *rb.GetImageSkiaNamed(IDR_LIVEMAIN_TITLEBAR_USER_FACE);
    }
    SetAvatar(avatar_img);
}

void LivehimeUserPhotoView::SetAvatar(const gfx::ImageSkia& avatar_img)
{
    raw_image_ = avatar_img;
    auto tmp = bililive::CreateCircleImageEx(raw_image_, GetPreferredSize());
    SetAllStateImage(&tmp);
}

void LivehimeUserPhotoView::SetVerifyType(UserOfficalVerifyType verify_type)
{
    verify_type_ = verify_type;
    SchedulePaint();
}

void LivehimeUserPhotoView::SetPreferredSize(const gfx::Size& preferred_size)
{
    BililiveImageButton::SetPreferredSize(preferred_size);
    SetAvatar(raw_image_);
}

void LivehimeUserPhotoView::OnPaint(gfx::Canvas * canvas)
{
    __super::OnPaint(canvas);

    if (UserOfficalVerifyType::Unverified != verify_type_)
    {
        gfx::ImageSkia* verfy_flag = GetVerifyTypeFlagIcon(verify_type_);
        if (verfy_flag)
        {
            SkPaint paint;
            paint.setFilterBitmap(true);
            paint.setFilterLevel(SkPaint::kHigh_FilterLevel);
            canvas->DrawImageInt(*verfy_flag, 0, 0, verfy_flag->width(), verfy_flag->height(),
                                 width() - kVerfyFlagSize.width(),
                                 height() - kVerfyFlagSize.height(),
                                 kVerfyFlagSize.width(),
                                 kVerfyFlagSize.height(), true, paint);
        }
    }
}


// LivehimeUserPhotoExView
LivehimeUserPhotoExView::LivehimeUserPhotoExView(views::ButtonListener* listener, const gfx::Size& avatar_size)
    : LivehimeImageView(listener),
      padding_(kAvatarPadding)
{
    SetImageHoriAlign(TRAILING, VERIFY);
    SetImageVertAlign(TRAILING, VERIFY);
    SetImageBoundSize(true, icon_size_, VERIFY);

    SetImageHoriAlign(TRAILING, NFT);
    SetImageVertAlign(TRAILING, NFT);
    SetImageBoundSize(true, GetSizeByDPIScale({14, 14}), NFT);
    if (avatar_size.IsEmpty()) {
        SetPreferredSize(kAvatarSize);
    }
    else {
        SetPreferredSize(avatar_size);
    }
    SetScaleType(ScaleType::ST_SHOW_ALL);

    ResourceBundle &rb = ResourceBundle::GetSharedInstance();
    SetImagePlaceholder(*rb.GetImageSkiaNamed(IDR_LIVEMAIN_TITLEBAR_USER_FACE), AVATAR);
}

LivehimeUserPhotoExView::~LivehimeUserPhotoExView()
{
    if (nft_gif_)
    {
        nft_gif_->CleanUp();
    }
}

void LivehimeUserPhotoExView::SetAvatarUrl(const std::string& url) {
    SetImageUrl(url, AVATAR);
}

void LivehimeUserPhotoExView::SetAvatar(const uint8_t* data, uint32_t size)
{
    gfx::ImageSkia avatar_img;
    if (data && size > 0)
    {
        avatar_img = bililive::MakeSkiaImage(data, size);
    }
    else
    {
        ResourceBundle& rb = ResourceBundle::GetSharedInstance();
        avatar_img = *rb.GetImageSkiaNamed(IDR_LIVEMAIN_TITLEBAR_USER_FACE);
    }
    SetImage(avatar_img, AVATAR);
}

void LivehimeUserPhotoExView::SetAvatarPadding(const gfx::Insets& padding) {
    padding_ = padding;
    if (!GetOrgImage(false, FRAME).isNull()) {
        SetImagePadding(padding_, AVATAR);
    }
}

void LivehimeUserPhotoExView::SetAvatarFrameUrl(const std::string& url) {
    SetImageUrl(url, FRAME);
}

void LivehimeUserPhotoExView::SetVerifyType(UserOfficalVerifyType type)
{
    auto& rb = ResourceBundle::GetSharedInstance();
    verift_type_ = type;

    switch (verift_type_)
    {
    case UserOfficalVerifyType::Official:
        SetImage(*rb.GetImageSkiaNamed(IDR_LIVEMAIN_USER_VERFY_OFFICIAL), VERIFY);
        break;
    case UserOfficalVerifyType::Enterprise:
        SetImage(*rb.GetImageSkiaNamed(IDR_LIVEMAIN_USER_VERFY_ENTERPRISE), VERIFY);
        break;
    default:
        SetImage({}, VERIFY);
        break;
    }

    AdjustPadding();
}

void LivehimeUserPhotoExView::SetNFT(bool nft)
{
    auto& rb = ResourceBundle::GetSharedInstance();
    is_nft_ = nft;
    if (is_nft_)
    {
        if (!nft_gif_)
        {
            auto gif_data = LoadDataResourceBytes(IDR_LIVEMAIN_NFT_GIF);
            nft_gif_ = new BililiveImageRender(this);
            nft_gif_->SetEnableGif(true);
            nft_gif_->InitFromBinary(gif_data->front(), gif_data->size(), false);
        }
    }
    else
    {
        if (nft_gif_)
        {
            nft_gif_->CleanUp();
            nft_gif_ = nullptr;
        }

        SetImage({}, NFT);
    }

    AdjustPadding();
}

void LivehimeUserPhotoExView::RemoveAvatarFrame() {
    SetImagePadding(gfx::Insets(), AVATAR);
    SetImageUrl("", FRAME);
}

void LivehimeUserPhotoExView::SetIconSize(const gfx::Size& icon_size)
{
    icon_size_ = icon_size;
}

gfx::ImageSkia LivehimeUserPhotoExView::OnImageChanging(
    const gfx::ImageSkia& img, int id, bool is_placeholder)
{
    if (id == AVATAR) {
        return bililive::CreateCircleImageEx(img);
    }
    return img;
}

void LivehimeUserPhotoExView::OnImageDownloaded(int id) {
    if (id == FRAME) {
        SetImagePadding(padding_, AVATAR);
    }
}

void LivehimeUserPhotoExView::AdjustPadding()
{
    SetImageHoriAlign(TRAILING, VERIFY);
    SetImageVertAlign(TRAILING, VERIFY);
    SetImageBoundSize(true, icon_size_, VERIFY);

    SetImageHoriAlign(TRAILING, NFT);
    SetImageVertAlign(TRAILING, NFT);
    SetImageBoundSize(true, icon_size_, NFT);

    if (is_nft_ &&
        verift_type_ != UserOfficalVerifyType::Unverified)
    {
        SetImagePadding({ 0, 0, 0, icon_size_.width() }, NFT);
    }
    else
    {
        SetImagePadding({ 0, 0, 0, 0 }, NFT);
    }
}

void LivehimeUserPhotoExView::OnImageFrameChanged(const gfx::ImageSkia* image)
{
    if (is_nft_)
    {
        SetImage(*image, NFT);
        SchedulePaint();
    }
}


// LivehimeUserLevelView
LivehimeUserLevelView::LivehimeUserLevelView(bool draw_level_name_bk/* = false*/)
    : level_(0)
    , level_name_(L"UP")
    , draw_name_bk_(draw_level_name_bk)
{
    int thickness = draw_level_name_bk ? GetLengthByDPIScale(4) : ftTwelve.GetStringWidth(L" ");
    SetLayoutManager(new views::BoxLayout(views::BoxLayout::kHorizontal, GetLengthByDPIScale(2), 0, thickness));
    name_label_ = new BililiveLabel(level_name_, ftTwelve);
    level_label_ = new BililiveLabel(std::to_wstring(level_), ftTwelve);
    AddChildView(name_label_);
    AddChildView(level_label_);
}

LivehimeUserLevelView::~LivehimeUserLevelView()
{
}

void LivehimeUserLevelView::SetLevel(int level)
{
    SetLevel(level_name_, level);
}

void LivehimeUserLevelView::SetLevel(int level, SkColor color) {
    SetLevel(level_name_, level, color);
}

void LivehimeUserLevelView::SetLevel(const base::string16& level_name, int level)
{
    SetLevel(level_name_, level, GetUserLevelColor(level));
}

void LivehimeUserLevelView::SetLevel(const base::string16& level_name, int level, SkColor color)
{
    level_name_ = level_name;
    level_ = level;
    name_label_->SetText(level_name_);
    level_label_->SetText(std::to_wstring(level_));
    name_label_->SetTextColor(draw_name_bk_ ? SK_ColorWHITE : color);
    level_label_->SetTextColor(color);
    PreferredSizeChanged();
}

void LivehimeUserLevelView::OnPaintBackground(gfx::Canvas* canvas)
{
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(level_label_->enabled_color());

    static const int rad = GetLengthByDPIScale(2);
    if (draw_name_bk_)
    {
        SkPath path;
        SkRect sk_rect = SkRect::MakeXYWH(0, 0, name_label_->bounds().right() + 2, height());
        SkScalar redius[] = {
                    rad, rad,
                    0, 0,
                    0, 0,
                    rad, rad
        };
        path.addRoundRect(sk_rect, redius);

        canvas->Save();

        canvas->sk_canvas()->clipPath(path, SkRegion::kIntersect_Op, true);
        canvas->DrawColor(level_label_->enabled_color());

        canvas->Restore();
    }

    bililive::DrawRoundRect(canvas, 0, 0, level_label_->bounds().right() + rad, height(), rad, paint);
}