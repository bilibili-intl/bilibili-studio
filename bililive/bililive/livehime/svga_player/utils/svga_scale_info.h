#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_UTILS_SVGA_SCALE_INFO_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_UTILS_SVGA_SCALE_INFO_H_


namespace svga {

    enum class ScaleType {
        CENTER,
        CENTER_CROP,
        CENTER_INSIDE,
        FIT_CENTER,
        FIT_START,
        FIT_END,
        FIT_XY,
        MATRIX,
    };

    class SVGAScaleInfo {
    public:
        SVGAScaleInfo();

        void performScaleType(
            float canvas_width, float canvas_height,
            float video_width, float video_height, ScaleType scale_type);

    private:
        void resetVars();

    public:
        float tran_fx_ = 0;
        float tran_fy_ = 0;
        float scale_fx_ = 1;
        float scale_fy_ = 1;
        float ratio_ = 1;
        bool ratio_x_ = false;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_UTILS_SVGA_SCALE_INFO_H_