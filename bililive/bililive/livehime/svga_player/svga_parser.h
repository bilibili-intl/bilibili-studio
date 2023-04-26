#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_SVGA_PARSER_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_SVGA_PARSER_H_

#include <fstream>
#include <string>


/**
 * 基于 Android SVGA v2.4.7 版本的解析器
 * https://github.com/yyued/SVGAPlayer-Android
 */

namespace svga {

    class SVGAVideoEntity;

    class SVGAParser {
    public:
        SVGAParser();

        void setCachePath(const std::wstring& file_path);

        bool parseFromFile(const std::wstring& file_name, SVGAVideoEntity* out);
        bool parseFromData(const std::string& data, SVGAVideoEntity* out);

    private:
        bool decodeFolder(const std::wstring& path, SVGAVideoEntity* out);

        bool unzipFromFile(const std::wstring& in_file_name, const std::wstring& out_file_name);
        bool unzipFromData(const std::string& data, const std::wstring& out_file_name);

        bool decompressFromFile(std::ifstream& file, std::string* out_data);
        bool decompressFromData(char* in_data, size_t length, std::string* out_data);

        std::wstring cache_dir_path_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_SVGA_PARSER_H_