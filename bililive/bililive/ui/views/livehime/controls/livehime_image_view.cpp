#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"

#include "base/ext/callable_callback.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/canvas.h"
#include "SkRRect.h"
#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "grit/theme_resources.h"


LivehimeImageView::LivehimeImageView(views::ButtonListener* listener)
    : CustomButton(listener),
      s_type_(ST_SHOW_ALL),
      cursor_(nullptr),
      is_spec_preferred_size_(false),
      weakptr_factory_(this) {
}

gfx::Size LivehimeImageView::GetPreferredSize() {
    gfx::Size size;
    if (is_spec_preferred_size_) {
        size = preferred_size_;
    } else {
        for (const auto& pair : img_layer_ents_) {
            auto& entity = pair.second;
            if (entity.using_size) {
                auto cur_size = entity.size;
                cur_size.Enlarge(entity.padding.width(), entity.padding.height());
                size.SetToMax(cur_size);
            } else {
                if (!entity.org_img.isNull()) {
                    auto cur_size = entity.org_img.size();
                    cur_size.Enlarge(entity.padding.width(), entity.padding.height());
                    size.SetToMax(entity.org_img.size());
                }
                if (!entity.org_ph_img.isNull()) {
                    auto cur_size = entity.org_ph_img.size();
                    cur_size.Enlarge(entity.padding.width(), entity.padding.height());
                    size.SetToMax(entity.org_ph_img.size());
                }
            }
        }
    }

    size.Enlarge(GetInsets().width(), GetInsets().height());
    return size;
}

void LivehimeImageView::OnPaint(gfx::Canvas* canvas) {
    __super::OnPaint(canvas);

     
        for (auto& pair : img_layer_ents_) {
            auto& entity = pair.second;

            auto details = GetImageToPaint(entity);
            if (!details.image.isNull())
            {
                if (is_round_) {
                    int radius = GetLengthByDPIScale(radius_);
                    SkRRect rrect;
                    SkVector sk_vector[]{ SkVector::Make(radius,radius),SkVector::Make(radius,radius) ,SkVector::Make(radius,radius),SkVector::Make(radius,radius) };
                    rrect.setRectRadii(SkRect::MakeXYWH(details.point.x(), details.point.y(), details.image.width(), details.image.height()), sk_vector);
                    canvas->Save();
                    canvas->sk_canvas()->clipRRect(rrect, SkRegion::kIntersect_Op, true);
                    canvas->DrawImageInt(details.image, details.point.x(), details.point.y());
                    canvas->Restore();
                
                }
                else
                {
                    canvas->DrawImageInt(details.image, details.point.x(), details.point.y());
                }
                
            }
        }
     
 
    
}   

gfx::ImageSkia LivehimeImageView::GetImage()
{
    for (auto& pair : img_layer_ents_)
    {
        auto& entity = pair.second;

        auto details = GetImageToPaint(entity);
        if (!details.image.isNull())
        {
            return details.image;
        }
    }
    return gfx::ImageSkia();
}

gfx::ImageSkia LivehimeImageView::GetOrgImage(bool is_placeholder, int id) const {
    auto it = img_layer_ents_.find(id);
    if (it != img_layer_ents_.end()) {
        return is_placeholder ? it->second.org_ph_img : it->second.org_img;
    }

    return {};
}

bool LivehimeImageView::IsURLImage(int id) {
    auto it = img_layer_ents_.find(id);
    if (it != img_layer_ents_.end()) {
        return it->second.using_url;
    }
    return false;
}

LivehimeImageView::DrawImageDetails LivehimeImageView::GetImageToPaint(ImageLayerEntity& entity)
{
    DrawImageDetails details;

    auto bounds = GetContentsBounds();
    if (bounds.IsEmpty()) {
        return details;
    }

    gfx::ImageSkia* cur_img = nullptr;
    gfx::Point point;

    gfx::Rect cur_bounds(bounds);
    if (s_type_ == ST_NONE) {
        if (!entity.org_img.isNull()) {
            // 图片
            cur_img = &entity.org_img;
            point = GetImagePoint(cur_img->size(), cur_bounds, entity.hori_align, entity.vert_align);
        } else if (!entity.org_ph_img.isNull()) {
            // 占位图
            cur_img = &entity.org_ph_img;
            point = GetImagePoint(cur_img->size(), cur_bounds, entity.hori_align, entity.vert_align);
        }
    } else {
        if (!entity.img_cache.scaled_img.isNull()) {
            // 图片
            cur_img = &entity.img_cache.scaled_img;
            cur_bounds.Inset(entity.padding);
            point = GetImagePoint(cur_img->size(), cur_bounds, entity.hori_align, entity.vert_align);
        } else if (!entity.ph_img_cache.scaled_img.isNull()) {
            // 占位图
            cur_img = &entity.ph_img_cache.scaled_img;
            cur_bounds.Inset(entity.padding);
            point = GetImagePoint(cur_img->size(), cur_bounds, entity.hori_align, entity.vert_align);
        }
    }

    if (cur_img) {
        details.image = *cur_img;
    }
    details.point = point;

    return details;
}

void LivehimeImageView::ResizeImages() {
    auto bounds = GetContentsBounds();
    if (bounds.IsEmpty()) {
        return;
    }
    if (s_type_ == ST_NONE) {
        return;
    }

    bool fill = (s_type_ == ST_FILL);
    auto img_size = bounds.size();

    for (auto& pair : img_layer_ents_) {
        auto& entity = pair.second;

        if (!entity.org_img.isNull()) {
            // 图片
            if (entity.using_size) {
                auto ava_size = img_size;
                ava_size.Enlarge(-entity.padding.width(), -entity.padding.height());
                ava_size.SetToMin(entity.size);

                if (!entity.img_cache.Hit(ava_size)) {
                    auto scaled_img = bililive::CreateScaledImageEx(entity.org_img, ava_size, fill);
                    entity.img_cache.Set(OnImageChanging(scaled_img, *entity.id, false));
                }
            } else {
                auto ava_size = img_size;
                ava_size.Enlarge(-entity.padding.width(), -entity.padding.height());

                if (!entity.img_cache.Hit(ava_size)) {
                    auto scaled_img = bililive::CreateScaledImageEx(entity.org_img, ava_size, fill);
                    entity.img_cache.Set(OnImageChanging(scaled_img, *entity.id, false));
                }
            }
        } else if (!entity.org_ph_img.isNull()) {
            // 占位图
            if (entity.using_size) {
                auto ava_size = img_size;
                ava_size.Enlarge(-entity.padding.width(), -entity.padding.height());
                ava_size.SetToMin(entity.size);

                if (!entity.ph_img_cache.Hit(ava_size)) {
                    auto scaled_img = bililive::CreateScaledImageEx(entity.org_ph_img, ava_size, fill);
                    entity.ph_img_cache.Set(OnImageChanging(scaled_img, *entity.id, true));
                }
            } else {
                auto ava_size = img_size;
                ava_size.Enlarge(-entity.padding.width(), -entity.padding.height());

                if (!entity.ph_img_cache.Hit(ava_size)) {
                    auto scaled_img = bililive::CreateScaledImageEx(entity.org_ph_img, ava_size, fill);
                    entity.ph_img_cache.Set(OnImageChanging(scaled_img, *entity.id, true));
                }
            }
        }
    }
}

gfx::NativeCursor LivehimeImageView::GetCursor(const ui::MouseEvent& event)
{
    if (!enabled())
        return gfx::kNullCursor;

    if (cursor_)
    {
        return cursor_;
    }
    return __super::GetCursor(event);
}

bool LivehimeImageView::HitTestRect(const gfx::Rect& rect) const
{
    return enabled() ? __super::HitTestRect(rect) : false;
}

void LivehimeImageView::OnBoundsChanged(const gfx::Rect& previous_bounds) {
    ResizeImages();
}

void LivehimeImageView::NotifyClick(const ui::Event& event)
{
    // crash发现个现象：在有些view里（用户信息面板）点击按钮跳转系统浏览器时，
    // 可能会因为窗口丢失焦点而直接关闭（UI线程不排队等待mouserelease事件处理完毕？）
    // 从而引起接下来的ReportBehaviorEvent使用到已被释放的资源引发崩溃。
    // 所以这里改成ReportBehaviorEvent置于__super::NotifyClick之前，看看效果。
    // 注意！由于这里把埋点置于按钮事件触发之前，所以如果是在点击时上报动态参数的场景，
    // 就不要采用按钮自动上报的方式，应该改用在按钮listener里进行埋点
    ReportBehaviorEvent();

    __super::NotifyClick(event);
}

gfx::ImageSkia LivehimeImageView::OnImageChanging(
    const gfx::ImageSkia& img, int id, bool is_placeholder)
{
    return img;
}

void LivehimeImageView::SetImageBinary(const unsigned char* data, unsigned int len, int id /*= -1*/)
{
    if (data && len > 0)
    {
        auto img = bililive::MakeSkiaImage(data, len);
        SetImage(img, id);
    }
    else
    {
        SetImage(gfx::ImageSkia());
    }
}

void LivehimeImageView::SetImage(const gfx::ImageSkia& img, int id) {
    auto entity = MakeImageLayerEntity(id);
    if (!entity) {
        return;
    }

    auto prev_size = GetPreferredSize();

    // 吊销弱指针，防止过期回调回来
    if (entity->using_url) {
        entity->using_url = false;
        entity->id = std::make_shared<int>(id);
    }

    entity->org_img = img;
    entity->img_cache.Reset();

    if (!is_spec_preferred_size_ &&
        parent() &&
        prev_size != GetPreferredSize())
    {
        InvalidateLayout();
        parent()->Layout();
    }

    ResizeImages();
    SchedulePaint();
}

void LivehimeImageView::SetImageUrl(const std::string& url, int id) {
    auto entity = MakeImageLayerEntity(id);
    if (!entity) {
        return;
    }

    // 之前已经请求过了，直接返回
    if (entity->using_url && entity->url == url) {
        return;
    }

    // 吊销弱指针，防止过期回调回来
    if (entity->using_url) {
        entity->id = std::make_shared<int>(id);
    }

    auto prev_size = GetPreferredSize();
    bool need_redraw = !entity->org_img.isNull();

    entity->url = url;
    entity->using_url = true;
    entity->org_img = gfx::ImageSkia();
    entity->img_cache.Reset();

    if (!entity->url.empty()) {
        auto callback = base::MakeCallable(base::Bind(
            &LivehimeImageView::OnGetImageByUrl,
            weakptr_factory_.GetWeakPtr()));

        using namespace std::placeholders;
        std::weak_ptr<int> weak_ptr = entity->id;
        auto handler = std::bind(callback, _1, _2, weak_ptr);

        GetBililiveProcess()->secret_core()->danmaku_hime_service()->DownloadImage(
            entity->url, handler).Call();
    }

    if (!is_spec_preferred_size_ &&
        parent() &&
        prev_size != GetPreferredSize())
    {
        InvalidateLayout();
        parent()->Layout();
    }
    ResizeImages();
    if (need_redraw) {
        SchedulePaint();
    }
}

void LivehimeImageView::SetImagePlaceholder(const gfx::ImageSkia& img, int id) {
    auto entity = MakeImageLayerEntity(id);
    if (!entity) {
        return;
    }

    auto prev_size = GetPreferredSize();

    entity->org_ph_img = img;
    entity->ph_img_cache.Reset();

    if (!is_spec_preferred_size_ &&
        parent() &&
        prev_size != GetPreferredSize())
    {
        InvalidateLayout();
        parent()->Layout();
    }
    ResizeImages();
    SchedulePaint();
}

void LivehimeImageView::SetImagePadding(const gfx::Insets& padding, int id) {
    auto entity = MakeImageLayerEntity(id);
    if (!entity) {
        return;
    }

    if (entity->padding != padding) {
        entity->padding = padding;

        if (!is_spec_preferred_size_ &&
            parent())
        {
            InvalidateLayout();
            parent()->Layout();
        }
        ResizeImages();
        SchedulePaint();
    }
}

void LivehimeImageView::SetImageBoundSize(bool enabled, const gfx::Size& size, int id) {
    auto entity = MakeImageLayerEntity(id);
    if (!entity) {
        return;
    }

    if (entity->using_size == enabled) {
        if (!enabled || (enabled && entity->size == size)) {
            return;
        }
    }

    auto prev_size = GetPreferredSize();

    entity->using_size = enabled;
    entity->size = size;

    if (!is_spec_preferred_size_ &&
        parent() &&
        prev_size != GetPreferredSize())
    {
        InvalidateLayout();
        parent()->Layout();
    }
    ResizeImages();
    SchedulePaint();
}

void LivehimeImageView::SetImageHoriAlign(Alignment align, int id) {
    auto entity = MakeImageLayerEntity(id);
    if (!entity) {
        return;
    }

    if (entity->hori_align != align) {
        entity->hori_align = align;
        SchedulePaint();
    }
}

void LivehimeImageView::SetImageVertAlign(Alignment align, int id) {
    auto entity = MakeImageLayerEntity(id);
    if (!entity) {
        return;
    }

    if (entity->vert_align != align) {
        entity->vert_align = align;
        SchedulePaint();
    }
}

void LivehimeImageView::SetScaleType(ScaleType type) {
    if (type != s_type_) {
        s_type_ = type;
        for (auto& pair : img_layer_ents_) {
            pair.second.img_cache.Reset();
            pair.second.ph_img_cache.Reset();
        }
        ResizeImages();
        SchedulePaint();
    }
}

void LivehimeImageView::RemoveImage(int id) {
    auto prev_size = GetPreferredSize();

    auto it = img_layer_ents_.find(id);
    if (it != img_layer_ents_.end()) {
        img_layer_ents_.erase(it);

        if (!is_spec_preferred_size_ &&
            parent() &&
            prev_size != GetPreferredSize())
        {
            InvalidateLayout();
            parent()->Layout();
        }
        ResizeImages();
        SchedulePaint();
    }
}

void LivehimeImageView::RemoveAllImages() {
    if (!img_layer_ents_.empty()) {
        img_layer_ents_.clear();

        if (!is_spec_preferred_size_ && parent()) {
            InvalidateLayout();
            parent()->Layout();
        }
        SchedulePaint();
    }
}

void LivehimeImageView::RefreshCache(int id) {
    auto it = img_layer_ents_.find(id);
    if (it == img_layer_ents_.end()) {
        return;
    }

    it->second.img_cache.Reset();
    it->second.ph_img_cache.Reset();

    ResizeImages();
    SchedulePaint();
}

void LivehimeImageView::RefreshAllCaches() {
    for (auto& e : img_layer_ents_) {
        e.second.img_cache.Reset();
        e.second.ph_img_cache.Reset();
    }

    ResizeImages();
    SchedulePaint();
}

void LivehimeImageView::SetPreferredSize(const gfx::Size& size) {
    if (is_spec_preferred_size_ && size == preferred_size_) {
        return;
    }

    is_spec_preferred_size_ = true;
    preferred_size_ = size;

    for (auto& entity : img_layer_ents_) {
        entity.second.img_cache.Reset();
        entity.second.ph_img_cache.Reset();
    }

    if (parent()) {
        InvalidateLayout();
        parent()->Layout();
    }
    ResizeImages();
    SchedulePaint();
}

LivehimeImageView::ImageLayerEntity* LivehimeImageView::MakeImageLayerEntity(int id) {
    // 查找指定 id 的 ImageLayerEntity。
    // 如果没找到就新建，并放到 map 里；如果找到就直接返回。
    auto it = img_layer_ents_.find(id);
    if (it == img_layer_ents_.end()) {
        ImageLayerEntity entity;
        entity.id = std::make_shared<int>(id);
        auto ret_pair = img_layer_ents_.insert({ id, entity });
        if (ret_pair.second) {
            it = ret_pair.first;
        } else {
            NOTREACHED();
            return nullptr;
        }
    }

    return &it->second;
}

gfx::Point LivehimeImageView::GetImagePoint(
    const gfx::Size& img_size, const gfx::Rect& bounds, Alignment ha, Alignment va) const
{
    gfx::Point point;
    switch (ha) {
    case LEADING: point.set_x(bounds.x()); break;
    case CENTER: point.set_x(bounds.x() + (bounds.width() - img_size.width()) / 2); break;
    case TRAILING: point.set_x(bounds.right() - img_size.width()); break;
    }

    switch (va) {
    case LEADING: point.set_y(bounds.y()); break;
    case CENTER: point.set_y(bounds.y() + (bounds.height() - img_size.height()) / 2); break;
    case TRAILING: point.set_y(bounds.bottom() - img_size.height()); break;
    }
    return point;
}

void LivehimeImageView::OnGetImageByUrl(
    bool valid_response, const std::string& data,
    const std::weak_ptr<int>& weak_id)
{
    if (!valid_response || data.empty()) {
        return;
    }

    // 是否过期？
    auto id_ptr = weak_id.lock();
    if (!id_ptr) {
        return;
    }

    auto it = img_layer_ents_.find(*id_ptr);
    if (it == img_layer_ents_.end()) {
        NOTREACHED();
        return;
    }

    auto img = bililive::MakeSkiaImage(
        reinterpret_cast<const unsigned char*>(data.data()), data.length());
    if (img.isNull()) {
        return;
    }

    OnImageDownloaded(*id_ptr);

    auto prev_size = GetPreferredSize();

    it->second.org_img = img;
    it->second.img_cache.Reset();

    if (!is_spec_preferred_size_ &&
        parent() &&
        prev_size != GetPreferredSize())
    {
        InvalidateLayout();
        parent()->Layout();
    }
    ResizeImages();
    SchedulePaint();
}



// LivehimeCircleImageView
LivehimeCircleImageView::LivehimeCircleImageView(views::ButtonListener* listener /*= nullptr*/)
    : LivehimeImageView(listener) {
}

gfx::ImageSkia LivehimeCircleImageView::OnImageChanging(const gfx::ImageSkia& img, int id, bool is_placeholder)
{
    return bililive::CreateCircleImageEx(img);
}



// LivehimeGuardPhotoView
LivehimeGuardPhotoView::LivehimeGuardPhotoView(views::ButtonListener* listener /*= nullptr*/)
    : LivehimeImageView(listener)
{}

void LivehimeGuardPhotoView::SetGuardFrame(int guard, int size) {
    auto& rb = ui::ResourceBundle::GetSharedInstance();
    switch (guard) {
    case 1:
        SetImage(*rb.GetImageSkiaNamed(IDR_LIVEHIME_AVATAR_FRAME_GOVERNOR), 1);
        SetImageBoundSize(true, gfx::Size(size, size));
        break;
    case 2:
        SetImage(*rb.GetImageSkiaNamed(IDR_LIVEHIME_AVATAR_FRAME_ADMIRAL), 1);
        SetImageBoundSize(true, gfx::Size(size, size));
        break;
    case 3:
        SetImage(*rb.GetImageSkiaNamed(IDR_LIVEHIME_AVATAR_FRAME_CAPTAIN), 1);
        SetImageBoundSize(true, gfx::Size(size, size));
        break;
    default:
        SetImage({}, 1);
        SetImageBoundSize(false, {});
        break;
    }
}

gfx::ImageSkia LivehimeGuardPhotoView::OnImageChanging(const gfx::ImageSkia& img, int id, bool is_placeholder)
{
    if (id == -1) {
        return bililive::CreateCircleImageEx(img);
    }
    return img;
}