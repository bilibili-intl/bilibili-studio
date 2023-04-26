#include "bililive/bililive/livehime/svga_player/proto/shape_entity.h"


namespace svga {

    // RGBAColor
    RGBAColor RGBAColor::decode(protobuf::ProtoReader& reader) {
        float r = 0;
        float g = 0;
        float b = 0;
        float a = 0;

        bool succeeded = reader.beginEmbedded();
        if (succeeded) {
            for (;;) {
                int tag = reader.nextTag();
                if (tag == -1) {
                    break;
                }

                switch (tag) {
                case 1: reader.getFloat(&r); break;
                case 2: reader.getFloat(&g); break;
                case 3: reader.getFloat(&b); break;
                case 4: reader.getFloat(&a); break;
                default: reader.skipValue(); break;
                }
            }

            reader.endEmbedded();
        }

        return RGBAColor(r, g, b, a, succeeded);
    }

    RGBAColor::RGBAColor(float r, float g, float b, float a, bool available)
        : r_(r), g_(g), b_(b), a_(a), is_available_(available) {}


    // ShapeArgs
    ShapeArgs ShapeArgs::decode(protobuf::ProtoReader& reader) {
        std::string d;

        bool succeeded = reader.beginEmbedded();
        if (succeeded) {
            for (;;) {
                int tag = reader.nextTag();
                if (tag == -1) {
                    break;
                }

                switch (tag) {
                case 1: reader.getBytes(&d); break;
                default: reader.skipValue(); break;
                }
            }
            reader.endEmbedded();
        }

        return ShapeArgs(d, succeeded);
    }

    ShapeArgs::ShapeArgs(const std::string& d, bool available)
        : d_(d), is_available_(available) {}


    // RectArgs
    RectArgs RectArgs::decode(protobuf::ProtoReader& reader) {
        float x = 0;
        float y = 0;
        float width = 0;
        float height = 0;
        float corner_r = 0;

        bool succeeded = reader.beginEmbedded();
        if (succeeded) {
            for (;;) {
                int tag = reader.nextTag();
                if (tag == -1) {
                    break;
                }

                switch (tag) {
                case 1: reader.getFloat(&x); break;
                case 2: reader.getFloat(&y); break;
                case 3: reader.getFloat(&width); break;
                case 4: reader.getFloat(&height); break;
                case 5: reader.getFloat(&corner_r); break;
                default: reader.skipValue(); break;
                }
            }
            reader.endEmbedded();
        }

        return RectArgs(x, y, width, height, corner_r, succeeded);
    }

    RectArgs::RectArgs(
        float x, float y, float width, float height, float corner_r,
        bool available)
        : x_(x), y_(y), width_(width),
          height_(height), corner_r_(corner_r),
          is_available_(available) {}


    // EllipseArgs
    EllipseArgs EllipseArgs::decode(protobuf::ProtoReader& reader) {
        float x = 0;
        float y = 0;
        float r_x = 0;
        float r_y = 0;

        bool succeeded = reader.beginEmbedded();
        if (succeeded) {
            for (;;) {
                int tag = reader.nextTag();
                if (tag == -1) {
                    break;
                }

                switch (tag) {
                case 1: reader.getFloat(&x); break;
                case 2: reader.getFloat(&y); break;
                case 3: reader.getFloat(&r_x); break;
                case 4: reader.getFloat(&r_y); break;
                default: reader.skipValue(); break;
                }
            }
            reader.endEmbedded();
        }

        return EllipseArgs(x, y, r_x, r_y, succeeded);
    }

    EllipseArgs::EllipseArgs(float x, float y, float r_x, float r_y, bool available)
        : x_(x), y_(y), r_x_(r_x), r_y_(r_y), is_available_(available) {}


    // ShapeStyle
    ShapeStyle ShapeStyle::decode(protobuf::ProtoReader& reader) {
        RGBAColor fill(0, 0, 0, 0);
        RGBAColor stroke(0, 0, 0, 0);
        float stroke_width = 0;
        LineCap cap = LineCap::LineCap_BUTT;
        LineJoin join = LineJoin::LineJoin_MITER;
        float miter_limit = 0;
        float line_dash_i = 0;
        float line_dash_ii = 0;
        float line_dash_iii = 0;

        bool succeeded = reader.beginEmbedded();
        if (succeeded) {
            for (;;) {
                int tag = reader.nextTag();
                if (tag == -1) {
                    break;
                }

                switch (tag) {
                case 1: fill = RGBAColor::decode(reader); break;
                case 2: stroke = RGBAColor::decode(reader); break;
                case 3: reader.getFloat(&stroke_width); break;
                case 4: reader.getEnum(&cap); break;
                case 5: reader.getEnum(&join); break;
                case 6: reader.getFloat(&miter_limit); break;
                case 7: reader.getFloat(&line_dash_i); break;
                case 8: reader.getFloat(&line_dash_ii); break;
                case 9: reader.getFloat(&line_dash_iii); break;
                default: reader.skipValue(); break;
                }
            }
            reader.endEmbedded();
        }

        return ShapeStyle(
            fill, stroke, stroke_width, cap, join,
            miter_limit, line_dash_i, line_dash_ii, line_dash_iii, succeeded);
    }

    ShapeStyle::ShapeStyle(
        const RGBAColor& fill, const RGBAColor& stroke,
        float stroke_width, LineCap cap, LineJoin join,
        float miter_limit, float line_dash_i, float line_dash_ii, float line_dash_iii,
        bool available)
        : fill_(fill), stroke_(stroke),
          stroke_width_(stroke_width), cap_(cap), join_(join),
          miter_limit_(miter_limit), line_dash_i_(line_dash_i),
          line_dash_ii_(line_dash_ii), line_dash_iii_(line_dash_iii),
          is_available_(available) {}


    // ShapeEntity
    ShapeEntity ShapeEntity::decode(protobuf::ProtoReader& reader) {
        ShapeType type = ShapeType::SHAPE;
        ShapeStyle style(
            RGBAColor(0, 0, 0, 0), RGBAColor(0, 0, 0, 0), 0,
            LineCap::LineCap_BUTT, LineJoin::LineJoin_MITER, 0, 0, 0, 0);
        Transform transform(0, 0, 0, 0, 0, 0);
        ShapeArgs shape("");
        RectArgs rect(0, 0, 0, 0, 0);
        EllipseArgs ellipse(0, 0, 0, 0);

        bool succeeded = reader.beginEmbedded();
        if (succeeded) {
            for (;;) {
                int tag = reader.nextTag();
                if (tag == -1) {
                    break;
                }

                switch (tag) {
                case 1: reader.getEnum(&type); break;
                case 10: style = ShapeStyle::decode(reader); break;
                case 11: transform = Transform::decode(reader); break;
                case 2: shape = ShapeArgs::decode(reader); break;
                case 3: rect = RectArgs::decode(reader); break;
                case 4: ellipse = EllipseArgs::decode(reader); break;
                default: reader.skipValue(); break;
                }
            }
            reader.endEmbedded();
        }

        return ShapeEntity(type, style, transform, shape, rect, ellipse, succeeded);
    }

    ShapeEntity::ShapeEntity(
        ShapeType type, const ShapeStyle& style,
        const Transform& transform, const ShapeArgs& shape, const RectArgs& rect,
        const EllipseArgs& ellipse, bool available)
        : type_(type), styles_(style), transform_(transform),
          shape_(shape), rect_(rect), ellipse_(ellipse), is_available_(available) {}

}
