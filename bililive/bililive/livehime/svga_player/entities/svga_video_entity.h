#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_VIDEO_ENTITY_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_VIDEO_ENTITY_H_

#include "base/values.h"
#include "base/file_util.h"

#include "ui/gfx/rect.h"
#include "ui/gfx/image/image_skia.h"

#include "bililive/bililive/livehime/svga_player/proto/movie_entity.h"
#include "bililive/bililive/livehime/svga_player/entities/svga_video_sprite_entity.h"
#include "bililive/bililive/livehime/svga_player/entities/svga_audio_entity.h"


namespace svga {

    class SVGAVideoEntity {
    public:
        SVGAVideoEntity();
        SVGAVideoEntity(const Value& obj, const base::FilePath& cache_dir);
        SVGAVideoEntity(const MovieEntity& obj, const base::FilePath& cache_dir);

        void prepare();

    private:
        void resetImages(const Value& obj);
        void resetImages(const MovieEntity& obj);

        void resetSprites(const Value& obj);
        void resetSprites(const MovieEntity& obj);

        void resetAudios(const MovieEntity& obj);

        gfx::ImageSkia decodeImgFromFile(const base::FilePath& file_name);

    public:
        bool antialias_ = true;
        gfx::RectF video_size_;
        int fps_ = 15;
        int frames_ = 0;
        std::shared_ptr<MovieEntity> movie_item_;
        std::vector<SVGAVideoSpriteEntity> sprites_;
        std::vector<SVGAAudioEntity> audios_;
        std::map<std::string, gfx::ImageSkia> images_;
        base::FilePath cache_dir_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_VIDEO_ENTITY_H_