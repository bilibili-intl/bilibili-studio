#include "bililive/bililive/livehime/svga_player/entities/svga_video_sprite_frame_entity.h"


namespace svga {

    SVGAVideoSpriteFrameEntity::SVGAVideoSpriteFrameEntity(const Value& obj)
        : alpha_(0),
          transform_(SkMatrix::I())
    {
        const DictionaryValue* val = nullptr;
        if (obj.GetType() == Value::TYPE_DICTIONARY &&
            obj.GetAsDictionary(&val))
        {
            val->GetDouble("alpha", &alpha_);

            const DictionaryValue* layout_val = nullptr;
            if (val->GetDictionary("layout", &layout_val)) {
                double x = 0;
                double y = 0;
                double width = 0;
                double height = 0;
                layout_val->GetDouble("x", &x);
                layout_val->GetDouble("y", &y);
                layout_val->GetDouble("width", &width);
                layout_val->GetDouble("height", &height);
                layout_.SetRect(x, y, width, height);
            }

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

                transform_.setAll(a, c, tx, b, d, ty, 0, 0, 1);
            }

            std::string d;
            if (val->GetString("clipPath", &d) && !d.empty()) {
                mask_path_.reset(new SVGAPathEntity(d));
            }

            const ListValue* list_val = nullptr;
            if (val->GetList("shapes", &list_val)) {
                for (size_t i = 0; i < list_val->GetSize(); ++i) {
                    const DictionaryValue* obj_val = nullptr;
                    if (list_val->GetDictionary(i, &obj_val)) {
                        shapes_.push_back(std::make_shared<SVGAVideoShapeEntity>(*obj_val));
                    }
                }
            }
        }
    }

    SVGAVideoSpriteFrameEntity::SVGAVideoSpriteFrameEntity(const FrameEntity& obj)
        : alpha_(0),
          transform_(SkMatrix::I())
    {
        alpha_ = obj.alpha_;
        if (obj.layout_.isAvailable()) {
            layout_.SetRect(
                obj.layout_.x_, obj.layout_.y_, obj.layout_.width_, obj.layout_.height_);
        }
        if (obj.transform_.isAvailable()) {
            transform_.setAll(
                obj.transform_.a_, obj.transform_.c_, obj.transform_.tx_,
                obj.transform_.b_, obj.transform_.d_, obj.transform_.ty_,
                0, 0, 1);
        }
        if (!obj.clip_path_.empty()) {
            mask_path_.reset(new SVGAPathEntity(obj.clip_path_));
        }
        for (const auto& s : obj.shapes_) {
            shapes_.push_back(std::make_shared<SVGAVideoShapeEntity>(s));
        }
    }

}