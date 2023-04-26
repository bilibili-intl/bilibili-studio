#include "bililive/bililive/livehime/svga_player/svga_parser.h"

#include "base/file_util.h"
#include "base/json/json_reader.h"

#include "third_party/zlib/google/zip.h"
#include "third_party/zlib/zlib.h"

#include "bililive/bililive/livehime/svga_player/entities/svga_video_entity.h"


namespace svga {

    SVGAParser::SVGAParser() {}

    void SVGAParser::setCachePath(const std::wstring& file_path) {
        cache_dir_path_ = file_path;
    }

    bool SVGAParser::parseFromFile(const std::wstring& file_name, SVGAVideoEntity* out) {
        std::ifstream svga_file(file_name, std::ios::in | std::ios::binary);
        if (!svga_file) {
            return false;
        }

        char buf[4];
        if (svga_file.read(buf, 4) &&
            svga_file.gcount() == 4 &&
            buf[0] == 80 && buf[1] == 75 && buf[2] == 3 && buf[3] == 4)
        {
            if (cache_dir_path_.empty()) {
                return false;
            }

            if (!unzipFromFile(file_name, cache_dir_path_)) {
                return false;
            }

            if (!decodeFolder(cache_dir_path_, out)) {
                return false;
            }
        } else if (svga_file) {
            svga_file.seekg(0, std::ios::beg);
            std::string data;
            if (!decompressFromFile(svga_file, &data)) {
                return false;
            }

            protobuf::ProtoReader reader(data);
            auto video_item = SVGAVideoEntity(MovieEntity::decode(reader), base::FilePath(cache_dir_path_));
            video_item.prepare();
            *out = std::move(video_item);
        } else {
            return false;
        }

        return true;
    }

    bool SVGAParser::parseFromData(const std::string& data, SVGAVideoEntity* out) {
        char buf[4];
        if (data.size() > 4 &&
            buf[0] == 80 && buf[1] == 75 && buf[2] == 3 && buf[3] == 4)
        {
            if (cache_dir_path_.empty()) {
                return false;
            }

            if (!unzipFromData(data, cache_dir_path_)) {
                return false;
            }

            if (!decodeFolder(cache_dir_path_, out)) {
                return false;
            }
        } else {
            std::string raw_data;
            if (!decompressFromData(const_cast<char*>(data.data()), data.size(), &raw_data)) {
                return false;
            }

            protobuf::ProtoReader reader(raw_data);
            auto video_item = SVGAVideoEntity(MovieEntity::decode(reader), base::FilePath(cache_dir_path_));
            video_item.prepare();
            *out = std::move(video_item);
        }

        return true;
    }

    bool SVGAParser::decodeFolder(const std::wstring& path, SVGAVideoEntity* out) {
        auto bin_file = base::FilePath(path).AppendASCII("movie.binary");
        if (base::PathExists(bin_file) && !base::DirectoryExists(bin_file)) {
            std::ifstream file(bin_file.value(), std::ios::in | std::ios::binary);
            if (!file) {
                return false;
            }

            protobuf::ProtoReader reader(&file);
            SVGAVideoEntity entity(MovieEntity::decode(reader), base::FilePath(path));
            *out = std::move(entity);
        }

        auto spec_file = base::FilePath(path).AppendASCII("movie.spec");
        if (base::PathExists(spec_file) && !base::DirectoryExists(spec_file)) {
            std::ifstream file(spec_file.value(), std::ios::in | std::ios::binary);
            if (!file) {
                return false;
            }

            auto pos = file.tellg();
            file.seekg(0, std::ios::end);
            auto size = file.tellg() - pos;
            file.seekg(pos, std::ios::beg);

            std::unique_ptr<char[]> buf(new char[size]);
            if (!file.read(buf.get(), size) || file.gcount() != size) {
                return false;
            }

            std::string data(buf.get(), size);
            std::unique_ptr<Value> ptr(base::JSONReader::Read(data));
            if (!ptr) {
                return false;
            }

            SVGAVideoEntity entity(*ptr, base::FilePath(path));
            *out = std::move(entity);
        }

        return true;
    }

    bool SVGAParser::unzipFromFile(const std::wstring& in_file_name, const std::wstring& out_file_name) {
        return zip::Unzip(base::FilePath(in_file_name), base::FilePath(out_file_name));
    }

    bool SVGAParser::unzipFromData(const std::string& data, const std::wstring& out_file_name) {
        return zip::UnzipFromString(data, base::FilePath(out_file_name));
    }

    bool SVGAParser::decompressFromFile(std::ifstream& file, std::string* out_data) {
        auto pos = file.tellg();
        file.seekg(0, std::ios::end);
        auto size = file.tellg() - pos;
        file.seekg(pos, std::ios::beg);

        std::unique_ptr<char[]> in_ptr(new char[size]);
        if (!file.read(in_ptr.get(), size) || file.gcount() != size) {
            return false;
        }

        return decompressFromData(in_ptr.get(), size, out_data);
    }

    bool SVGAParser::decompressFromData(char* in_data, size_t length, std::string* out_data) {
        auto in_str = in_data;
        size_t in_len = length;

        if (!in_str) {
            return false;
        }

        z_stream strm;
        std::memset(&strm, 0, sizeof(strm));
        int ret = inflateInit(&strm);
        if (ret != Z_OK) {
            return false;
        }

        std::shared_ptr<z_stream> sp_strm(&strm, [](z_stream* strm) {
            (void)inflateEnd(strm);
        });

        int flush;
        const int chunk = 512;
        unsigned char out[chunk];
        const char* end = in_str + in_len;

        do {
            size_t distance = end - in_str;
            strm.avail_in = (distance >= chunk) ? chunk : distance;
            strm.next_in = reinterpret_cast<Bytef*>(in_str);

            in_str += strm.avail_in;
            flush = (in_str == end) ? Z_FINISH : Z_NO_FLUSH;

            do {
                strm.avail_out = chunk;
                strm.next_out = out;
                ret = ::inflate(&strm, Z_NO_FLUSH);
                if (ret == Z_STREAM_ERROR)
                    break;
                switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    return false;
                default:
                    break;
                }
                int have = chunk - strm.avail_out;
                out_data->insert(out_data->end(), out, out + have);
            } while (strm.avail_out == 0);
        } while (flush != Z_FINISH);

        return ret == Z_STREAM_END ? true : false;
    }

}