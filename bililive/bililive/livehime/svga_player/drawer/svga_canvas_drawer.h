#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_DRAWER_SVGA_CANVAS_DRAWER_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_DRAWER_SVGA_CANVAS_DRAWER_H_

#include "SkMatrix.h"
#include "SkPaint.h"

#include "bililive/bililive/livehime/svga_player/drawer/svga_drawer.h"
#include "bililive/bililive/livehime/svga_player/entities/svga_dynamic_entity.h"


namespace svga {

    class ShareValues {
    public:
        ShareValues();

        SkPaint& sharedPaint();
        gfx::Path& sharedPath();
        gfx::Path& sharedPath2();
        SkMatrix& sharedMatrix();
        SkMatrix& sharedMatrix2();

        SkPaint shared_paint;
        gfx::Path shared_path;
        gfx::Path shared_path2;
        SkMatrix shared_matrix;
        SkMatrix shared_matrix2;
    };


    class SVGACanvasDrawer : public SVGADrawer {
    public:
        SVGACanvasDrawer(
            const std::shared_ptr<SVGAVideoEntity>& video_item,
            const std::shared_ptr<SVGADynamicEntity>& dynamic_item);

        void drawFrame(
            gfx::Canvas* c, const gfx::Size& view_size,
            int frame_index, ScaleType scale_type) override;

    private:
        void playAudio(int frame_index);
        SkMatrix& shareFrameMatrix(const SkMatrix& transform);
        void drawSprite(SVGADrawerSprite& sprite, gfx::Canvas* c, int frame_index);
        void drawImage(const SVGADrawerSprite& sprite, gfx::Canvas* c);
        void drawTextOnBitmap(
            gfx::Canvas* c, const gfx::ImageSkia& img, const SVGADrawerSprite& sprite,
            const SkMatrix& frame_matrix);
        void drawShape(SVGADrawerSprite& sprite, gfx::Canvas* c);
        float matrixScale(const SkMatrix& matrix);
        void drawDynamic(const SVGADrawerSprite& sprite, gfx::Canvas* c, int frame_index);

        ShareValues shared_values_;
        std::shared_ptr<SVGADynamicEntity> dynamic_item_;
        std::map<std::string, gfx::ImageSkia> draw_text_cache_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_DRAWER_SVGA_CANVAS_DRAWER_H_