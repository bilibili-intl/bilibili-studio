#include "bililive/bililive/livehime/svga_player/entities/svga_video_sprite_entity.h"


namespace svga {

    SVGAVideoSpriteEntity::SVGAVideoSpriteEntity(const Value& obj) {
        const DictionaryValue* val = nullptr;
        if (obj.GetType() == Value::TYPE_DICTIONARY &&
            obj.GetAsDictionary(&val))
        {
            val->GetString("imageKey", &img_key_);

            std::vector<std::shared_ptr<SVGAVideoSpriteFrameEntity>> frames;
            const ListValue* list_val = nullptr;
            if (val->GetList("frames", &list_val)) {
                for (size_t i = 0; i < list_val->GetSize(); ++i) {
                    const DictionaryValue* obj_val = nullptr;
                    if (list_val->GetDictionary(i, &obj_val)) {
                        auto frame_item = std::make_shared<SVGAVideoSpriteFrameEntity>(*obj_val);

                        if (!frame_item->shapes_.empty()) {
                            auto& it = frame_item->shapes_.front();
                            if (it->isKeep() && !frames.empty()) {
                                frame_item->shapes_ = frames.back()->shapes_;
                            }
                        }
                        frames.push_back(std::move(frame_item));
                    }
                }
            }

            frames_ = std::move(frames);
        }
    }

    SVGAVideoSpriteEntity::SVGAVideoSpriteEntity(const SpriteEntity& obj) {
        img_key_ = obj.img_key_;
        std::shared_ptr<SVGAVideoSpriteFrameEntity> last_frame;

        for (const auto& f : obj.frames_) {
            auto frame_item = std::make_shared<SVGAVideoSpriteFrameEntity>(f);
            if (!frame_item->shapes_.empty()) {
                auto& it = frame_item->shapes_.front();
                if (it->isKeep()) {
                    if (last_frame) {
                        frame_item->shapes_ = last_frame->shapes_;
                    }
                }
            }
            frames_.push_back(std::move(frame_item));
            last_frame = frames_.back();
        }
    }

}