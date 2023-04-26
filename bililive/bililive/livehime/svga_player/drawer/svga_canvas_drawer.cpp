#include "bililive/bililive/livehime/svga_player/drawer/svga_canvas_drawer.h"

//#include "mmsystem.h"

#include "SkShader.h"
#include "SkDashPathEffect.h"

#include "base/strings/string_util.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

//#pragma comment(lib, "WINMM.LIB")


namespace svga {

    // ShareValues
    ShareValues::ShareValues()
        : shared_matrix(SkMatrix::I()),
          shared_matrix2(SkMatrix::I()) {
    }

    SkPaint& ShareValues::sharedPaint() {
        shared_paint.reset();
        return shared_paint;
    }

    gfx::Path& ShareValues::sharedPath() {
        shared_path.reset();
        return shared_path;
    }

    gfx::Path& ShareValues::sharedPath2() {
        shared_path2.reset();
        return shared_path2;
    }

    SkMatrix& ShareValues::sharedMatrix() {
        shared_matrix.reset();
        return shared_matrix;
    }

    SkMatrix& ShareValues::sharedMatrix2() {
        shared_matrix2.reset();
        return shared_matrix2;
    }


    // SVGACanvasDrawer
    SVGACanvasDrawer::SVGACanvasDrawer(
        const std::shared_ptr<SVGAVideoEntity>& video_item,
        const std::shared_ptr<SVGADynamicEntity>& dynamic_item)
        : SVGADrawer(video_item), dynamic_item_(dynamic_item) {}

    void SVGACanvasDrawer::drawFrame(
        gfx::Canvas* c, const gfx::Size& view_size,
        int frame_index, ScaleType scale_type)
    {
        SVGADrawer::drawFrame(c, view_size, frame_index, scale_type);

        std::vector<SVGADrawerSprite> sprites;
        requestFrameSprites(frame_index, &sprites);
        for (auto& s : sprites) {
            drawSprite(s, c, frame_index);
        }
        playAudio(frame_index);
    }

    void SVGACanvasDrawer::playAudio(int frame_index) {
        // 当前不播放音频
        return;

        /*for (const auto& audio : video_item_->audios_) {
            if (audio.start_frame == frame_index) {
                std::wstring cmd = L"open ";
                cmd.append(audio.path.c_str());
                auto ret = ::mciSendStringW(cmd.c_str(), nullptr, 0, nullptr);

                cmd = L"play ";
                cmd.append(audio.path.c_str());
                ret = ::mciSendStringW(cmd.c_str(), nullptr, 0, nullptr);
            }
            if (audio.end_frame <= frame_index) {
                std::wstring cmd = L"stop ";
                cmd.append(audio.path.c_str());
                auto ret = ::mciSendStringW(cmd.c_str(), nullptr, 0, nullptr);

                cmd = L"close ";
                cmd.append(audio.path.c_str());
                ret = ::mciSendStringW(cmd.c_str(), nullptr, 0, nullptr);
            }
        }*/
    }

    SkMatrix& SVGACanvasDrawer::shareFrameMatrix(const SkMatrix& transform) {
        auto& matrix = shared_values_.sharedMatrix();
        matrix.postScale(scale_info_.scale_fx_, scale_info_.scale_fy_);
        matrix.postTranslate(scale_info_.tran_fx_, scale_info_.tran_fy_);
        matrix.preConcat(transform);
        return matrix;
    }

    void SVGACanvasDrawer::drawSprite(SVGADrawerSprite& sprite, gfx::Canvas* c, int frame_index) {
        drawImage(sprite, c);
        drawShape(sprite, c);
        drawDynamic(sprite, c, frame_index);
    }

    void SVGACanvasDrawer::drawImage(const SVGADrawerSprite& sprite, gfx::Canvas* c) {
        auto img_key = sprite.img_key;
        bool is_hidden = dynamic_item_->dynamic_hidden_[img_key];
        if (is_hidden) {
            return;
        }

        gfx::ImageSkia drawing_img = dynamic_item_->dynamic_img_[img_key];
        if (drawing_img.isNull()) {
            auto it = video_item_->images_.find(img_key);
            if (it == video_item_->images_.end()) {
                return;
            }
            drawing_img = it->second;
            if (drawing_img.isNull()) {
                return;
            }
        }

        auto& frame_matrix = shareFrameMatrix(sprite.frame_entity->transform_);
        auto& paint = shared_values_.sharedPaint();
        paint.setAntiAlias(video_item_->antialias_);
        paint.setFilterBitmap(video_item_->antialias_);
        paint.setAlpha(sprite.frame_entity->alpha_ * 255);

        if (sprite.frame_entity->mask_path_) {
            auto mask_path = sprite.frame_entity->mask_path_;
            c->Save();
            paint.reset();
            auto& path = shared_values_.sharedPath();
            mask_path->buildPath(&path);
            path.transform(frame_matrix);
            c->ClipPath(path);
            frame_matrix.preScale(
                sprite.frame_entity->layout_.width() / float(drawing_img.width()),
                sprite.frame_entity->layout_.width() / float(drawing_img.width()));
            c->sk_canvas()->drawBitmapMatrix(*drawing_img.bitmap(), frame_matrix, &paint);
            c->Restore();
        } else {
            frame_matrix.preScale(
                sprite.frame_entity->layout_.width() / float(drawing_img.width()),
                sprite.frame_entity->layout_.width() / float(drawing_img.width()));
            c->sk_canvas()->drawBitmapMatrix(*drawing_img.bitmap(), frame_matrix, &paint);
        }

        drawTextOnBitmap(c, drawing_img, sprite, frame_matrix);
    }

    void SVGACanvasDrawer::drawTextOnBitmap(
        gfx::Canvas* c, const gfx::ImageSkia& img, const SVGADrawerSprite& sprite,
        const SkMatrix& frame_matrix)
    {
        if (dynamic_item_->is_text_dirty_) {
            draw_text_cache_.clear();
            dynamic_item_->is_text_dirty_ = false;
        }

        auto img_key = sprite.img_key;
        gfx::ImageSkia text_bitmap;

        auto dt_it = dynamic_item_->dynamic_text_.find(img_key);
        if (dt_it != dynamic_item_->dynamic_text_.end()) {
            auto dtp_it = dynamic_item_->dynamic_text_paint_.find(img_key);
            if (dtp_it != dynamic_item_->dynamic_text_paint_.end()) {
                auto drawing_text = dt_it->second;
                auto drawing_text_paint = dtp_it->second;
                auto dtc_it = draw_text_cache_.find(img_key);
                if (dtc_it != draw_text_cache_.end()) {
                    text_bitmap = dtc_it->second;
                } else {
                    gfx::Canvas text_canvas(img.size(), ui::SCALE_FACTOR_100P, false);
                    drawing_text_paint.setAntiAlias(true);

                    int width = 0;
                    int height = 0;
                    gfx::Canvas::SizeStringInt(
                        drawing_text, ftPrimary, &width, &height, 0, gfx::Canvas::NO_ELLIPSIS);
                    auto x = (img.width() - width) / 2.f;
                    float target_rect_top = 0;
                    float target_rect_bottom = img.height();
                    auto y = (target_rect_bottom + target_rect_top - height) / 2.f;
                    text_canvas.DrawStringInt(
                        drawing_text, ftPrimary, drawing_text_paint.getColor(), x, y, width, height);
                    text_bitmap = gfx::ImageSkia(text_canvas.ExtractImageRep());
                    draw_text_cache_[img_key] = text_bitmap;
                }
            }
        }

        // TODO: Layout Text

        if (!text_bitmap.isNull()) {
            auto& paint = shared_values_.sharedPaint();
            paint.setAntiAlias(video_item_->antialias_);
            if (sprite.frame_entity->mask_path_) {
                auto mask_path = sprite.frame_entity->mask_path_;
                c->Save();
                c->sk_canvas()->concat(frame_matrix);
                c->ClipRect(gfx::Rect(0, 0, img.width(), img.height()));
                paint.setShader(
                    SkShader::CreateBitmapShader(
                        *text_bitmap.bitmap(), SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode));
                auto& path = shared_values_.sharedPath();
                mask_path->buildPath(&path);
                c->DrawPath(path, paint);
                c->Restore();
            } else {
                paint.setFilterBitmap(video_item_->antialias_);
                c->sk_canvas()->drawBitmapMatrix(*text_bitmap.bitmap(), frame_matrix, &paint);
            }
        }
    }

    void SVGACanvasDrawer::drawShape(SVGADrawerSprite& sprite, gfx::Canvas* c) {
        auto& frame_matrix = shareFrameMatrix(sprite.frame_entity->transform_);
        for (auto& shape : sprite.frame_entity->shapes_) {
            shape->buildPath();
            if (!shape->shape_path_.isEmpty()) {
                auto& paint = shared_values_.sharedPaint();
                paint.reset();
                paint.setAntiAlias(video_item_->antialias_);
                paint.setAlpha(sprite.frame_entity->alpha_ * 255);

                auto& path = shared_values_.sharedPath();
                path.reset();
                path.addPath(shape->shape_path_);

                auto& shape_matrix = shared_values_.sharedMatrix2();
                shape_matrix.reset();
                shape_matrix.postConcat(shape->transform_);
                shape_matrix.postConcat(frame_matrix);
                path.transform(shape_matrix);

                if (shape->styles_.fill != 0) {
                    paint.setStyle(SkPaint::kFill_Style);
                    paint.setColor(shape->styles_.fill);
                    paint.setAlpha(std::min(255, std::max(0, int(sprite.frame_entity->alpha_ * 255))));
                    if (sprite.frame_entity->mask_path_) {
                        c->Save();
                        auto mask_path = sprite.frame_entity->mask_path_;
                        auto& path2 = shared_values_.sharedPath2();
                        mask_path->buildPath(&path2);
                        path2.transform(frame_matrix);
                        c->ClipPath(path2);
                    }

                    c->DrawPath(path, paint);

                    if (sprite.frame_entity->mask_path_) {
                        c->Restore();
                    }
                }

                if (shape->styles_.stroke_width > 0) {
                    paint.setStyle(SkPaint::kStroke_Style);
                    if (shape->styles_.stroke != 0) {
                        paint.setColor(shape->styles_.stroke);
                        paint.setAlpha(std::min(255, std::max(0, int(sprite.frame_entity->alpha_ * 255))));
                    }

                    auto scale = matrixScale(frame_matrix);
                    paint.setStrokeWidth(shape->styles_.stroke_width);

                    if (!shape->styles_.line_cap.empty()) {
                        if (base::EqualsCaseInsensitiveASCII(shape->styles_.line_cap, "butt")) {
                            paint.setStrokeCap(SkPaint::kButt_Cap);
                        } else if (base::EqualsCaseInsensitiveASCII(shape->styles_.line_cap, "round")) {
                            paint.setStrokeCap(SkPaint::kRound_Cap);
                        } else if (base::EqualsCaseInsensitiveASCII(shape->styles_.line_cap, "square")) {
                            paint.setStrokeCap(SkPaint::kSquare_Cap);
                        }
                    }

                    if (!shape->styles_.line_join.empty()) {
                        if (base::EqualsCaseInsensitiveASCII(shape->styles_.line_join, "miter")) {
                            paint.setStrokeJoin(SkPaint::kMiter_Join);
                        } else if (base::EqualsCaseInsensitiveASCII(shape->styles_.line_join, "round")) {
                            paint.setStrokeJoin(SkPaint::kRound_Join);
                        } else if (base::EqualsCaseInsensitiveASCII(shape->styles_.line_join, "bevel")) {
                            paint.setStrokeJoin(SkPaint::kBevel_Join);
                        }
                    }

                    if (shape->styles_.miter_limit != 0) {
                        paint.setStrokeMiter(shape->styles_.miter_limit * scale);
                    }

                    if (!shape->styles_.line_dash.empty()) {
                        auto it = shape->styles_.line_dash;
                        if (it.size() == 3 && (it[0] > 0 || it[1] > 0)) {
                            SkScalar intv[] {
                                    (it[0] < 1 ? 1 : it[0]) * scale,
                                    (it[1] < 0.1f ? 0.1f : it[1]) * scale
                            };
                            auto effect = new SkDashPathEffect(intv, 2, it[2] * scale);
                            paint.setPathEffect(effect);
                        }
                    }

                    if (sprite.frame_entity->mask_path_) {
                        c->Save();
                        auto mask_path = sprite.frame_entity->mask_path_;
                        auto& path2 = shared_values_.sharedPath2();
                        mask_path->buildPath(&path2);
                        path2.transform(frame_matrix);
                        c->ClipPath(path2);
                    }

                    c->DrawPath(path, paint);

                    if (sprite.frame_entity->mask_path_) {
                        c->Restore();
                    }
                }
            }
        }
    }

    float SVGACanvasDrawer::matrixScale(const SkMatrix& matrix) {
        if (matrix[0] == 0) {
            return 0;
        }

        auto A = matrix[0];
        auto B = matrix[3];
        auto C = matrix[1];
        auto D = matrix[4];

        if (A * D == B * C) {
            return 0;
        }

        auto scale_x = std::sqrt(A * A + B * B);
        A /= scale_x;
        B /= scale_x;
        auto skew = A * C + B * D;
        C -= A * skew;
        D -= B * skew;
        auto scale_y = std::sqrt(C * C + D * D);
        C /= scale_y;
        D /= scale_y;
        skew /= scale_y;
        if (A * D < B * C) {
            scale_x = -scale_x;
        }
        return scale_info_.ratio_x_ ? std::abs(scale_x) : std::abs(scale_y);
    }

    void SVGACanvasDrawer::drawDynamic(const SVGADrawerSprite& sprite, gfx::Canvas* c, int frame_index) {
        auto img_key = sprite.img_key;
        auto dd_it = dynamic_item_->dynamic_drawer_.find(img_key);
        if (dd_it != dynamic_item_->dynamic_drawer_.end()) {
            auto& frame_matrix = shareFrameMatrix(sprite.frame_entity->transform_);
            c->Save();
            c->sk_canvas()->concat(frame_matrix);
            dd_it->second(c, frame_index);
            c->Restore();
        }

        auto dds_it = dynamic_item_->dynamic_drawer_sized_.find(img_key);
        if (dds_it != dynamic_item_->dynamic_drawer_sized_.end()) {
            auto& frame_matrix = shareFrameMatrix(sprite.frame_entity->transform_);
            c->Save();
            c->sk_canvas()->concat(frame_matrix);
            dds_it->second(
                c, frame_index,
                sprite.frame_entity->layout_.width(),
                sprite.frame_entity->layout_.height());
            c->Restore();
        }
    }

}