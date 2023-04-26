#include "bililive/bililive/livehime/hybrid_ui/hybrid_ui.h"

#include <regex>

// Ð­ÒéÎÄµµ£º
// @ https://info.bilibili.co/pages/viewpage.action?pageId=30131705


namespace hybrid_ui
{
    static const char kExInfoRegexPattern[] = ".*?\\?.*?pc_ui=(\\d*),(\\d*),([0-9a-fA-F]*?),(\\d*).*";

    LivehimeWebUrlDetails GetUrlExInfo(const std::string& url)
    {
        LivehimeWebUrlDetails url_ex;

        std::string ex_width = "";
        std::string ex_height = "";
        std::string ex_background = "";
        std::string ex_close = "";
        std::smatch mat;
        std::regex reg(kExInfoRegexPattern);

        if (regex_match(url, mat, reg))
        {
            ex_width = mat.str(1);
            ex_height = mat.str(2);
            ex_background = mat.str(3);
            ex_close = mat.str(4);
        }

        url_ex.url = url;
        url_ex.ex_width = ex_width;
        url_ex.ex_height = ex_height;
        url_ex.ex_background = ex_background;
        url_ex.ex_close = ex_close;

        return url_ex;
    }
}