#include "bililive/bililive/livehime/svga_player/entities/svga_video_shape_entity.h"

#include "base/strings/string_util.h"

#include "bililive/bililive/livehime/svga_player/entities/svga_path_entity.h"


namespace svga {

    SVGAVideoShapeEntity::SVGAVideoShapeEntity(const Value& obj)
        : transform_(SkMatrix::I())
    {
        parseType(obj);
        parseArgs(obj);
        parseStyles(obj);
        parseTransform(obj);
    }

    SVGAVideoShapeEntity::SVGAVideoShapeEntity(const ShapeEntity& obj)
        : transform_(SkMatrix::I())
    {
        parseType(obj);
        parseArgs(obj);
        parseStyles(obj);
        parseTransform(obj);
    }

    SVGAVideoShapeEntity::SVGAVideoShapeEntity(const SVGAVideoShapeEntity& rhs)
        : type_(rhs.type_),
          args_(rhs.args_),
          styles_(rhs.styles_),
          transform_(rhs.transform_)
    {
        shape_path_.addPath(rhs.shape_path_);
    }

    SVGAVideoShapeEntity& SVGAVideoShapeEntity::operator=(const SVGAVideoShapeEntity& rhs) {
        type_ = rhs.type_;
        args_ = rhs.args_;
        styles_ = rhs.styles_;
        transform_ = rhs.transform_;

        shape_path_.reset();
        shape_path_.addPath(rhs.shape_path_);

        return *this;
    }

    void SVGAVideoShapeEntity::buildPath() {
        if (!shape_path_.isEmpty()) {
            return;
        }

        gfx::Path shared_path;
        if (type_ == Shape) {
            auto it = args_.find("d");
            if (it != args_.end() && it->second->IsType(Value::TYPE_STRING)) {
                std::string val;
                if (it->second->GetAsString(&val)) {
                    SVGAPathEntity(val).buildPath(&shared_path);
                }
            }
        } else if (type_ == Ellipse) {
            double xv;
            auto it = args_.find("x");
            if (it == args_.end() || !GetNumber(it->second, &xv)) { return; }

            double yv;
            it = args_.find("y");
            if (it == args_.end() || !GetNumber(it->second, &yv)) { return; }

            double rxv;
            it = args_.find("radiusX");
            if (it == args_.end() || !GetNumber(it->second, &rxv)) { return; }

            double ryv;
            it = args_.find("radiusY");
            if (it == args_.end() || !GetNumber(it->second, &ryv)) { return; }

            shared_path.addOval(
                SkRect::MakeLTRB(xv - rxv, yv - ryv, xv + rxv, yv + ryv), SkPath::kCW_Direction);
        } else if (type_ == Rect) {
            double xv;
            auto it = args_.find("x");
            if (it == args_.end() || !GetNumber(it->second, &xv)) { return; }

            double yv;
            it = args_.find("y");
            if (it == args_.end() || !GetNumber(it->second, &yv)) { return; }

            double wv;
            it = args_.find("width");
            if (it == args_.end() || !GetNumber(it->second, &wv)) { return; }

            double hv;
            it = args_.find("height");
            if (it == args_.end() || !GetNumber(it->second, &hv)) { return; }

            double crv;
            it = args_.find("cornerRadius");
            if (it == args_.end() || !GetNumber(it->second, &crv)) { return; }

            shared_path.addRoundRect(
                SkRect::MakeLTRB(xv, yv, xv + wv, yv + hv), crv, crv, SkPath::kCW_Direction);
        }

        shape_path_.reset();
        shape_path_.addPath(shared_path);
    }

    bool SVGAVideoShapeEntity::isKeep() const {
        return type_ == Keep;
    }

    void SVGAVideoShapeEntity::parseType(const Value& obj) {
        const DictionaryValue* val = nullptr;
        if (obj.GetType() == Value::TYPE_DICTIONARY &&
            obj.GetAsDictionary(&val))
        {
            std::string type;
            if (val->GetString("type", &type)) {
                if (base::EqualsCaseInsensitiveASCII(type, "shape")) {
                    type_ = Shape;
                } else if (base::EqualsCaseInsensitiveASCII(type, "rect")) {
                    type_ = Rect;
                } else if (base::EqualsCaseInsensitiveASCII(type, "ellipse")) {
                    type_ = Ellipse;
                } else if (base::EqualsCaseInsensitiveASCII(type, "keep")) {
                    type_ = Keep;
                }
            }
        }
    }

    void SVGAVideoShapeEntity::parseType(const ShapeEntity& obj) {
        if (obj.type_ == ShapeEntity::SHAPE) {
            type_ = Shape;
        } else if (obj.type_ == ShapeEntity::RECT) {
            type_ = Rect;
        } else if (obj.type_ == ShapeEntity::ELLIPSE) {
            type_ = Ellipse;
        } else if (obj.type_ == ShapeEntity::KEEP) {
            type_ = Keep;
        }
    }

    void SVGAVideoShapeEntity::parseArgs(const Value& obj) {
        std::map<std::string, std::shared_ptr<Value>> args;

        const DictionaryValue* val = nullptr;
        if (obj.GetType() == Value::TYPE_DICTIONARY &&
            obj.GetAsDictionary(&val))
        {
            const DictionaryValue* values = nullptr;
            if (val->GetDictionary("args", &values)) {
                for (DictionaryValue::Iterator it(*values); !it.IsAtEnd(); it.Advance()) {
                    args[it.key()] = std::shared_ptr<Value>(it.value().DeepCopy());
                }
            }
        }

        args_ = std::move(args);
    }

    void SVGAVideoShapeEntity::parseArgs(const ShapeEntity& obj) {
        std::map<std::string, std::shared_ptr<Value>> args;

        if (obj.shape_.isAvailable()) {
            if (!obj.shape_.d_.empty()) {
                args["d"] = std::shared_ptr<Value>(Value::CreateStringValue(obj.shape_.d_));
            }
        }

        if (obj.ellipse_.isAvailable()) {
            args["x"] = std::shared_ptr<Value>(Value::CreateDoubleValue(obj.ellipse_.x_));
            args["y"] = std::shared_ptr<Value>(Value::CreateDoubleValue(obj.ellipse_.y_));
            args["radiusX"] = std::shared_ptr<Value>(Value::CreateDoubleValue(obj.ellipse_.r_x_));
            args["radiusY"] = std::shared_ptr<Value>(Value::CreateDoubleValue(obj.ellipse_.r_y_));
        }

        if (obj.rect_.isAvailable()) {
            args["x"] = std::shared_ptr<Value>(Value::CreateDoubleValue(obj.rect_.x_));
            args["y"] = std::shared_ptr<Value>(Value::CreateDoubleValue(obj.rect_.y_));
            args["width"] = std::shared_ptr<Value>(Value::CreateDoubleValue(obj.rect_.width_));
            args["height"] = std::shared_ptr<Value>(Value::CreateDoubleValue(obj.rect_.height_));
            args["cornerRadius"] = std::shared_ptr<Value>(Value::CreateDoubleValue(obj.rect_.corner_r_));
        }

        args_ = std::move(args);
    }

    void SVGAVideoShapeEntity::parseStyles(const Value& obj) {
        const DictionaryValue* val = nullptr;
        if (obj.GetType() == Value::TYPE_DICTIONARY &&
            obj.GetAsDictionary(&val))
        {
            const DictionaryValue* style_val = nullptr;
            if (val->GetDictionary("styles", &style_val)) {
                Styles styles;
                const ListValue* list_val = nullptr;
                if (style_val->GetList("fill", &list_val)) {
                    if (list_val->GetSize() == 4) {
                        double red = 0;
                        double green = 0;
                        double blue = 0;
                        double alpha = 0;
                        list_val->GetDouble(3, &alpha);
                        list_val->GetDouble(0, &red);
                        list_val->GetDouble(1, &green);
                        list_val->GetDouble(2, &blue);
                        styles.fill = SkColorSetARGB(alpha * 255, red * 255, green * 255, blue * 255);
                    }
                }

                if (style_val->GetList("stroke", &list_val)) {
                    if (list_val->GetSize() == 4) {
                        double red = 0;
                        double green = 0;
                        double blue = 0;
                        double alpha = 0;
                        list_val->GetDouble(3, &alpha);
                        list_val->GetDouble(0, &red);
                        list_val->GetDouble(1, &green);
                        list_val->GetDouble(2, &blue);
                        styles.stroke = SkColorSetARGB(alpha * 255, red * 255, green * 255, blue * 255);
                    }
                }

                double stroke_width = 0;
                style_val->GetDouble("strokeWidth", &stroke_width);
                styles.stroke_width = stroke_width;

                style_val->GetString("lineCap", &styles.line_cap);
                style_val->GetString("lineJoin", &styles.line_join);
                style_val->GetInteger("miterLimit", &styles.miter_limit);

                if (style_val->GetList("lineDash", &list_val)) {
                    for (size_t i = 0; i < list_val->GetSize(); ++i) {
                        double dash = 0;
                        list_val->GetDouble(i, &dash);
                        styles.line_dash.push_back(dash);
                    }
                }

                styles_ = std::move(styles);
            }
        }
    }

    void SVGAVideoShapeEntity::parseStyles(const ShapeEntity& obj) {
        if (!obj.styles_.isAvailable()) {
            return;
        }

        Styles styles;
        if (obj.styles_.fill_.isAvailable()) {
            styles.fill = SkColorSetARGB(
                obj.styles_.fill_.a_ * 255,
                obj.styles_.fill_.r_ * 255,
                obj.styles_.fill_.g_ * 255,
                obj.styles_.fill_.b_ * 255);
        }
        if (obj.styles_.stroke_.isAvailable()) {
            styles.stroke = SkColorSetARGB(
                obj.styles_.stroke_.a_ * 255,
                obj.styles_.stroke_.r_ * 255,
                obj.styles_.stroke_.g_ * 255,
                obj.styles_.stroke_.b_ * 255);
        }

        styles.stroke_width = obj.styles_.stroke_width_;

        switch (obj.styles_.cap_) {
        case LineCap::LineCap_BUTT: styles.line_cap = "butt"; break;
        case LineCap::LineCap_ROUND: styles.line_cap = "round"; break;
        case LineCap::LineCap_SQUARE: styles.line_cap = "square"; break;
        }

        switch (obj.styles_.join_) {
        case LineJoin::LineJoin_BEVEL: styles.line_join = "bevel"; break;
        case LineJoin::LineJoin_MITER: styles.line_join = "miter"; break;
        case LineJoin::LineJoin_ROUND: styles.line_join = "round"; break;
        }

        styles.miter_limit = obj.styles_.miter_limit_;
        styles.line_dash.clear();
        styles.line_dash.push_back(obj.styles_.line_dash_i_);
        styles.line_dash.push_back(obj.styles_.line_dash_ii_);
        styles.line_dash.push_back(obj.styles_.line_dash_iii_);
    }

    void SVGAVideoShapeEntity::parseTransform(const Value& obj) {
        auto matrix = SkMatrix::I();

        const DictionaryValue* val = nullptr;
        if (obj.GetType() == Value::TYPE_DICTIONARY &&
            obj.GetAsDictionary(&val))
        {
            const DictionaryValue* trans_val = nullptr;
            if (val->GetDictionary("transform", &trans_val)) {
                double a = 1;
                double b = 0;
                double c = 0;
                double d = 1;
                double tx = 0;
                double ty = 0;

                trans_val->GetDouble("a", &a);
                trans_val->GetDouble("b", &b);
                trans_val->GetDouble("c", &c);
                trans_val->GetDouble("d", &d);
                trans_val->GetDouble("tx", &tx);
                trans_val->GetDouble("ty", &ty);

                matrix.setAll(a, c, tx, b, d, ty, 0, 0, 1);

                transform_ = matrix;
            }
        }
    }

    void SVGAVideoShapeEntity::parseTransform(const ShapeEntity& obj) {
        if (obj.transform_.isAvailable()) {
            auto matrix = SkMatrix::I();
            matrix.setAll(
                obj.transform_.a_, obj.transform_.c_, obj.transform_.tx_,
                obj.transform_.b_, obj.transform_.d_, obj.transform_.ty_,
                0, 0, 1);
            transform_ = matrix;
        }
    }

    bool SVGAVideoShapeEntity::GetNumber(const std::shared_ptr<Value>& obj, double* val) {
        if (obj->IsType(Value::TYPE_DOUBLE)) {
            return obj->GetAsDouble(val);
        }
        if (obj->IsType(Value::TYPE_INTEGER)) {
            int out;
            if (obj->GetAsInteger(&out)) {
                *val = out;
                return true;
            }
        }
        return false;
    }

}