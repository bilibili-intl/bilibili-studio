#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_SHAPE_ENTITY_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_SHAPE_ENTITY_H_

#include "bililive/bililive/livehime/svga_player/protobuf/proto_reader.h"
#include "bililive/bililive/livehime/svga_player/proto/transform.h"


namespace svga {

    enum class LineCap {
        LineCap_BUTT = 0,
        LineCap_ROUND = 1,
        LineCap_SQUARE = 2,
    };

    enum class LineJoin {
        LineJoin_MITER = 0,
        LineJoin_ROUND = 1,
        LineJoin_BEVEL = 2,
    };


    class RGBAColor {
    public:
        static RGBAColor decode(protobuf::ProtoReader& reader);

        RGBAColor(float r, float g, float b, float a, bool available = false);

        bool isAvailable() const { return is_available_; }

        // [1:float]
        float r_;

        // [1:float]
        float g_;

        // [1:float]
        float b_;

        // [1:float]
        float a_;

    private:
        bool is_available_;
    };


    class ShapeArgs {
    public:
        static ShapeArgs decode(protobuf::ProtoReader& reader);

        explicit ShapeArgs(const std::string& d, bool available = false);

        bool isAvailable() const { return is_available_; }

        // [1:string]
        std::string d_;

    private:
        bool is_available_;
    };


    class RectArgs {
    public:
        static RectArgs decode(protobuf::ProtoReader& reader);

        RectArgs(
            float x, float y, float width, float height, float corner_r,
            bool available = false);

        bool isAvailable() const { return is_available_; }

        // [1:float]
        float x_;

        // [2:float]
        float y_;

        // [3:float]
        float width_;

        // [4:float]
        float height_;

        // [5:float]
        float corner_r_;

    private:
        bool is_available_;
    };


    class EllipseArgs {
    public:
        static EllipseArgs decode(protobuf::ProtoReader& reader);

        EllipseArgs(float x, float y, float r_x, float r_y, bool available = false);

        bool isAvailable() const { return is_available_; }

        // [1:float]
        float x_;

        // [2:float]
        float y_;

        // [3:float]
        float r_x_;

        // [4:float]
        float r_y_;

    private:
        bool is_available_;
    };


    class ShapeStyle {
    public:
        static ShapeStyle decode(protobuf::ProtoReader& reader);

        ShapeStyle(
            const RGBAColor& fill, const RGBAColor& stroke,
            float stroke_width, LineCap cap, LineJoin join,
            float miter_limit, float line_dash_i, float line_dash_ii, float line_dash_iii,
            bool available = false);

        bool isAvailable() const { return is_available_; }

        // [1:embedded]
        RGBAColor fill_;

        // [2:embedded]
        RGBAColor stroke_;

        // [3:float]
        float stroke_width_;

        // [4:enum]
        LineCap cap_;

        // [5:enum]
        LineJoin join_;

        // [6:float]
        float miter_limit_;

        // [7:float]
        float line_dash_i_;

        // [8:float]
        float line_dash_ii_;

        // [9:float]
        float line_dash_iii_;

    private:
        bool is_available_;
    };


    class ShapeEntity {
    public:
        enum ShapeType {
            // 路径
            SHAPE = 0,

            // 矩形
            RECT = 1,

            // 圆形
            ELLIPSE = 2,

            // 与前帧一致
            KEEP = 3,
        };

        static ShapeEntity decode(protobuf::ProtoReader& reader);

        ShapeEntity(
            ShapeType type, const ShapeStyle& style,
            const Transform& transform, const ShapeArgs& shape, const RectArgs& rect,
            const EllipseArgs& ellipse, bool available = false);

        bool isAvailable() const { return is_available_; }

        // [1:enum]
        ShapeType type_;

        // [10:embedded]
        ShapeStyle styles_;

        // [11:embedded]
        Transform transform_;

        // [2:embedded]
        ShapeArgs shape_;

        // [3:embedded]
        RectArgs rect_;

        // [4:embedded]
        EllipseArgs ellipse_;

    private:
        bool is_available_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_SHAPE_ENTITY_H_