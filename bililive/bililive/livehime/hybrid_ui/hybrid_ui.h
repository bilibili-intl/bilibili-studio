#ifndef BILILIVE_BILILIVE_LIVEHIME_HYBRID_UI_HYBRID_UI_H_
#define BILILIVE_BILILIVE_LIVEHIME_HYBRID_UI_HYBRID_UI_H_

#include <string>


namespace hybrid_ui
{
    struct LivehimeWebUrlDetails
    {
        std::string url;
        std::string ex_width;
        std::string ex_height;
        std::string ex_background;
        std::string ex_close;
    };

    LivehimeWebUrlDetails GetUrlExInfo(const std::string& url);
}
#endif //BILILIVE_BILILIVE_LIVEHIME_HYBRID_UI_HYBRID_UI_H_