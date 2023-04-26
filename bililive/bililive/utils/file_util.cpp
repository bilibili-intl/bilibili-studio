#include "file_util.h"

namespace bililive
{
    std::string GetFileName(const std::string url)
    {
        auto pos_p = url.rfind('/');
        auto pos_r = url.rfind('\\');

        if (pos_p == std::string::npos &&
            pos_r == std::string::npos)
        {
            return "";
        }

        pos_p = pos_p != std::string::npos ? pos_p : 0;
        pos_r = pos_r != std::string::npos ? pos_r : 0;
        auto pos = std::max(pos_p, pos_r);

        int sub_pos = pos + 1;
        int sub_len = url.size() - pos - 1;

        if (static_cast<size_t>(sub_pos + sub_len) > url.size())
        {
            return "";
        }

        return url.substr(sub_pos, sub_len);
    }
}
