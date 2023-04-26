#include "bililive/bililive/livehime/svga_player/entities/svga_dynamic_entity.h"


namespace svga {

    SVGADynamicEntity::SVGADynamicEntity() {}

    void SVGADynamicEntity::setHidden(bool value, const std::string& for_key) {
        dynamic_hidden_[for_key] = value;
    }

    void SVGADynamicEntity::setDynamicImage(const gfx::ImageSkia& img, const std::string& for_key) {
        dynamic_img_[for_key] = img;
    }

    void SVGADynamicEntity::setDynamicText(const std::wstring& text, const std::string& for_key) {
        is_text_dirty_ = true;
        dynamic_text_[for_key] = text;
    }

    void SVGADynamicEntity::setDynamicDrawer(const DrawerFunc& fun, const std::string& for_key) {
        dynamic_drawer_[for_key] = fun;
    }

    void SVGADynamicEntity::setDynamicDrawerSized(const DrawerSizedFunc& fun, const std::string& for_key) {
        dynamic_drawer_sized_[for_key] = fun;
    }

    void SVGADynamicEntity::clearDynamicObjects() {
        is_text_dirty_ = true;
        dynamic_hidden_.clear();
        dynamic_img_.clear();
        dynamic_text_.clear();
        dynamic_drawer_.clear();
        dynamic_drawer_sized_.clear();
    }

}