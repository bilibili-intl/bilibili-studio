#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_VIDEO_SHAPE_ENTITY_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_VIDEO_SHAPE_ENTITY_H_

#include "base/json/json_reader.h"
#include "base/values.h"

#include "ui/gfx/path.h"
#include "SkColor.h"

#include "bililive/bililive/livehime/svga_player/proto/shape_entity.h"


namespace svga {

    class SVGAVideoShapeEntity {
    public:
        enum Type {
            Shape,
            Rect,
            Ellipse,
            Keep,
        };

        struct Styles {
            SkColor fill = 0;
            SkColor stroke = 0;
            float stroke_width = 0;
            std::string line_cap = "butt";
            std::string line_join = "miter";
            int miter_limit = 0;
            std::vector<float> line_dash;
        };

        explicit SVGAVideoShapeEntity(const Value& obj);
        explicit SVGAVideoShapeEntity(const ShapeEntity& obj);
        explicit SVGAVideoShapeEntity(const SVGAVideoShapeEntity& rhs);

        SVGAVideoShapeEntity& operator=(const SVGAVideoShapeEntity& rhs);

        void buildPath();

        bool isKeep() const;

    private:
        void parseType(const Value& obj);
        void parseType(const ShapeEntity& obj);

        void parseArgs(const Value& obj);
        void parseArgs(const ShapeEntity& obj);

        void parseStyles(const Value& obj);
        void parseStyles(const ShapeEntity& obj);

        void parseTransform(const Value& obj);
        void parseTransform(const ShapeEntity& obj);

        bool GetNumber(const std::shared_ptr<Value>& obj, double* val);

    public:
        Type type_ = Shape;
        std::map<std::string, std::shared_ptr<Value>> args_;
        Styles styles_;
        SkMatrix transform_;
        gfx::Path shape_path_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_VIDEO_SHAPE_ENTITY_H_