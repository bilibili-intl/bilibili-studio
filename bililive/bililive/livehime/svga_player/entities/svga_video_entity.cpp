#include "bililive/bililive/livehime/svga_player/entities/svga_video_entity.h"

#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/bililive/utils/image_util.h"


namespace svga {

    SVGAVideoEntity::SVGAVideoEntity() {}

    SVGAVideoEntity::SVGAVideoEntity(const Value& obj, const base::FilePath& cache_dir) {
        cache_dir_ = cache_dir;

        const DictionaryValue* val = nullptr;
        if (obj.GetType() == Value::TYPE_DICTIONARY &&
            obj.GetAsDictionary(&val))
        {
            const DictionaryValue* box_obj = nullptr;
            if (val->GetDictionary("movie.viewBox", &box_obj)) {
                double width = 0;
                double height = 0;
                box_obj->GetDouble("width", &width);
                box_obj->GetDouble("height", &height);

                video_size_.SetRect(0, 0, width, height);
            }

            val->GetInteger("movie.fps", &fps_);
            val->GetInteger("movie.frames", &frames_);
        }

        resetImages(obj);
        resetSprites(obj);
    }

    SVGAVideoEntity::SVGAVideoEntity(const MovieEntity& obj, const base::FilePath& cache_dir) {
        movie_item_.reset(new MovieEntity(obj));
        cache_dir_ = cache_dir;

        if (obj.params_.isAvailable()) {
            video_size_.SetRect(
                0, 0, obj.params_.viewbox_width_, obj.params_.viewbox_height_);
            fps_ = obj.params_.fps_;
            frames_ = obj.params_.frames_;
        }

        resetImages(obj);
        resetSprites(obj);
    }

    void SVGAVideoEntity::prepare() {
        if (movie_item_) {
            resetAudios(*movie_item_);
        }
    }

    void SVGAVideoEntity::resetImages(const Value& obj) {
        const DictionaryValue* val = nullptr;
        if (obj.GetType() == Value::TYPE_DICTIONARY &&
            obj.GetAsDictionary(&val))
        {
            const DictionaryValue* img_obj = nullptr;
            if (val->GetDictionary("images", &img_obj)) {
                for (DictionaryValue::Iterator it(*img_obj); !it.IsAtEnd(); it.Advance()) {
                    std::string name;
                    if (it.value().GetAsString(&name)) {
                        base::FilePath file_path = cache_dir_.Append(UTF8ToUTF16(name));
                        auto img_skia = decodeImgFromFile(file_path);
                        if (!img_skia.isNull()) {
                            images_[it.key()] = img_skia;
                        } else {
                            file_path = cache_dir_.Append(UTF8ToUTF16(it.key())).AddExtension(L"png");
                            img_skia = decodeImgFromFile(file_path);
                            if (!img_skia.isNull()) {
                                images_[it.key()] = img_skia;
                            }
                        }
                    }
                }
            }
        }
    }

    void SVGAVideoEntity::resetImages(const MovieEntity& obj) {
        for (const auto& pair : obj.images_) {
            auto img_key = pair.first;
            auto bytes = pair.second;
            if (bytes.size() < 4) {
                continue;
            }

            auto file_tag = bytes.substr(0, 4);
            if (file_tag[0] == 73 && file_tag[1] == 68 && file_tag[2] == 51 && file_tag[3] == 3) {
                // ???
            } else {
                auto img_skia = bililive::MakeSkiaImage(
                    reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size());
                if (!img_skia.isNull()) {
                    images_[img_key] = img_skia;
                } else {
                    auto file_path = cache_dir_.Append(UTF8ToUTF16(bytes));
                    img_skia = decodeImgFromFile(file_path);
                    if (!img_skia.isNull()) {
                        images_[img_key] = img_skia;
                    } else {
                        file_path = cache_dir_.Append(UTF8ToUTF16(img_key)).AddExtension(L"png");
                        img_skia = decodeImgFromFile(file_path);
                        if (!img_skia.isNull()) {
                            images_[img_key] = img_skia;
                        }
                    }
                }
            }
        }
    }

    void SVGAVideoEntity::resetSprites(const Value& obj) {
        const DictionaryValue* val = nullptr;
        if (obj.GetType() == Value::TYPE_DICTIONARY &&
            obj.GetAsDictionary(&val))
        {
            const ListValue* list_val = nullptr;
            if (val->GetList("sprites", &list_val)) {
                for (std::size_t i = 0; i < list_val->GetSize(); ++i) {
                    const DictionaryValue* obj_val = nullptr;
                    if (list_val->GetDictionary(i, &obj_val)) {
                        sprites_.push_back(SVGAVideoSpriteEntity(*obj_val));
                    }
                }
            }
        }
    }

    void SVGAVideoEntity::resetSprites(const MovieEntity& obj) {
        for (const auto& s : obj.sprites_) {
            sprites_.push_back(SVGAVideoSpriteEntity(s));
        }
    }

    void SVGAVideoEntity::resetAudios(const MovieEntity& obj) {
        // 当前不播放音频
        return;

        /*std::map<std::string, std::string> audio_data;
        for (const auto& entry : obj.images_) {
            auto& img_key = entry.first;
            auto& bytes = entry.second;
            if (bytes.size() < 4) {
                continue;
            }

            if (bytes[0] == 73 && bytes[1] == 68 && bytes[2] == 51 && bytes[3] == 3) {
                audio_data[img_key] = bytes;
            }
        }

        std::map<std::string, std::wstring> audio_path;
        for (const auto& audio : audio_data) {
            auto path = cache_dir_.Append(UTF8ToUTF16(audio.first)).AddExtension(L"mp3");
            std::ofstream file(path.value(), std::ios::binary);
            file.write(audio.second.data(), audio.second.size());

            audio_path[audio.first] = path.value();
        }

        for (const auto& audio : obj.audios_) {
            SVGAAudioEntity item(audio);
            auto it = audio_path.find(audio.audio_key_);
            if (it != audio_path.end()) {
                item.path = it->second;
            }
            audios_.push_back(std::move(item));
        }*/
    }

    gfx::ImageSkia SVGAVideoEntity::decodeImgFromFile(const base::FilePath& file_name) {
        if (base::PathExists(file_name) && !base::DirectoryExists(file_name)) {
            auto in_bytes = bililive::GetBinaryFromFile(file_name);
            if (!in_bytes.empty()) {
                return bililive::MakeSkiaImage(
                    reinterpret_cast<const uint8_t*>(in_bytes.data()), in_bytes.size());
            }
        }
        return {};
    }

}