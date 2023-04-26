#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_PATH_ENTITY_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_PATH_ENTITY_H_

#include <string>

#include "base/strings/string_tokenizer.h"

#include "ui/gfx/path.h"


namespace svga {

    class SVGAPathEntity {
    public:
        explicit SVGAPathEntity(const std::string& origin_val);

        void buildPath(gfx::Path* to_path);

    private:
        void operate(
            gfx::Path& final_path, const std::string& method, base::StringTokenizer& args);

        bool StringToFloat(const std::string& str, float* val);

        gfx::Path cached_path_;
        std::string replaced_val_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_PATH_ENTITY_H_