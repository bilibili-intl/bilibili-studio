#include "bililive/bililive/livehime/svga_player/proto/movie_entity.h"


namespace svga {

    MovieEntity MovieEntity::decode(protobuf::ProtoReader& reader) {
        std::string version;
        MovieParams params(0, 0, 0, 0);
        std::map<std::string, std::string> images;
        std::vector<SpriteEntity> sprites;
        std::vector<AudioEntity> audios;

        bool succeeded = reader.beginEmbedded();
        if (succeeded) {
            for (;;) {
                protobuf::ProtoReader::WireType type;
                int tag = reader.nextTag(&type);
                if (tag == -1) {
                    break;
                }

                switch (tag) {
                case 1: reader.getBytes(&version); break;
                case 2: params = MovieParams::decode(reader); break;
                case 3: reader.getMap(&images); break;
                case 4: sprites.push_back(SpriteEntity::decode(reader)); break;
                case 5: audios.push_back(AudioEntity::decode(reader)); break;
                default: reader.skipValue(); break;
                }
            }

            reader.endEmbedded();
        }

        return MovieEntity(version, params, images, sprites, audios, succeeded);
    }

    MovieEntity::MovieEntity(
        const std::string& version, const MovieParams& params,
        const std::map<std::string, std::string>& images,
        const std::vector<SpriteEntity>& sprites, const std::vector<AudioEntity>& audios,
        bool available)
        : version_(version), params_(params),
          images_(images), sprites_(sprites), audios_(audios),
          is_available_(available) {}

}
