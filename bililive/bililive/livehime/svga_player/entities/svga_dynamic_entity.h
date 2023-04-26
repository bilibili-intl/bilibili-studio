#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_DYNAMIC_ENTITY_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_DYNAMIC_ENTITY_H_

#include <functional>
#include <map>
#include <string>

#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/canvas.h"


namespace svga {

    class SVGADynamicEntity {
    public:
        using DrawerFunc = std::function<bool(gfx::Canvas* c, int frame_index)>;
        using DrawerSizedFunc = std::function<bool(gfx::Canvas* c, int frame_index, int width, int height)>;

        SVGADynamicEntity();

        void setHidden(bool value, const std::string& for_key);
        void setDynamicImage(const gfx::ImageSkia& img, const std::string& for_key);
        void setDynamicText(const std::wstring& text, const std::string& for_key);
        void setDynamicDrawer(const DrawerFunc& fun, const std::string& for_key);
        void setDynamicDrawerSized(const DrawerSizedFunc& fun, const std::string& for_key);

        void clearDynamicObjects();


        std::map<std::string, bool> dynamic_hidden_;
        std::map<std::string, gfx::ImageSkia> dynamic_img_;
        std::map<std::string, std::wstring> dynamic_text_;
        std::map<std::string, SkPaint> dynamic_text_paint_;
        std::map<std::string, DrawerFunc> dynamic_drawer_;
        std::map<std::string, DrawerSizedFunc> dynamic_drawer_sized_;
        bool is_text_dirty_ = false;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_DYNAMIC_ENTITY_H_