#include "bililive/bililive/livehime/gift_image/image_recorder.h"

#include <fstream>

#include "base/file_util.h"

namespace livehime {

#define READ_STREAM(var, size)  \
    file.read(reinterpret_cast<char*>(&var), size);  \
    if (!file.good()) return false;

#define WRITE_STREAM(var, size)  \
    file.write(reinterpret_cast<const char*>(&var), size);  \
    if (!file.good()) return false;


    ImageRecorder::ImageRecorder()
        : header_() {}

    ImageRecorder::~ImageRecorder() {
        close();
    }

    void ImageRecorder::open(const std::wstring& path) {
        path_ = path;
        if (path_.empty()) {
            return;
        }

        if (!readFile(path)) {
            base::DeleteFileW(base::FilePath(path), false);

            header_.magic = 'R';
            header_.major_ver = 0;
            header_.minor_ver = 2;
            header_.total_count = 0;
        }
    }

    void ImageRecorder::put(const std::string& id, const ImageData& data) {
        if (path_.empty()) {
            return;
        }

        contents_[id] = data;
    }

    bool ImageRecorder::get(const std::string& id, ImageData* data) {
        if (path_.empty()) {
            return false;
        }

        auto it = contents_.find(id);
        if (it == contents_.end()) {
            return false;
        }

        *data = it->second;
        return true;
    }

    void ImageRecorder::remove(const std::string& id) {
        if (path_.empty()) {
            return;
        }

        auto it = contents_.find(id);
        if (it != contents_.end()) {
            contents_.erase(it);
        }
    }

    void ImageRecorder::close() {
        if (path_.empty()) {
            return;
        }

        if (!writeFile(path_)) {
            base::DeleteFileW(base::FilePath(path_), false);
        }
        path_.clear();
    }

    bool ImageRecorder::readFile(const std::wstring& path) {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file) {
            return false;
        }

        Header header;
        // 看下文件头对不对
        READ_STREAM(header.magic, 1);
        if (header.magic != 'R') {
            return false;
        }

        // 看下主版本对不对
        READ_STREAM(header.major_ver, 2);
        if (header.major_ver != 0) {
            return false;
        }

        // 看下次版本对不对
        READ_STREAM(header.minor_ver, 2);
        if (header.minor_ver != 2) {
            return false;
        }

        // 记录数如果超过了一千条，就重新开始记录
        READ_STREAM(header.total_count, 4);
        if (header.total_count > 1000) {
            return false;
        }

        std::map<std::string, ImageData> contents;

        // 读取礼物 id 和 url 的对应关系
        for (uint32_t i = 0; i < header.total_count; ++i) {
            ImageData data;
            uint32_t length;

            READ_STREAM(length, 4);
            if (length == 0 || length >= 2048) {
                return false;
            }
            data.id.resize(length);
            READ_STREAM(*data.id.begin(), length);

            READ_STREAM(length, 4);
            if (length == 0 || length >= 2048) {
                return false;
            }
            data.url.resize(length);
            READ_STREAM(*data.url.begin(), length);
            contents[data.id] = data;
        }

        header_ = header;
        contents_ = std::move(contents);
        return true;
    }

    bool ImageRecorder::writeFile(const std::wstring& path) {
        std::ofstream file(path, std::ios::trunc | std::ios::binary);
        if (!file) {
            return false;
        }

        header_.total_count = uint32_t(contents_.size());

        // 先写文件头
        WRITE_STREAM(header_.magic, 1);
        WRITE_STREAM(header_.major_ver, 2);
        WRITE_STREAM(header_.minor_ver, 2);
        WRITE_STREAM(header_.total_count, 4);

        // 再写记录项
        for (const auto& pair : contents_) {
            // 写入id
            uint32_t size = uint32_t(pair.second.id.size());
            WRITE_STREAM(size, 4);
            WRITE_STREAM(pair.second.id.data()[0], size);

            // 写入url
            size = uint32_t(pair.second.url.size());
            WRITE_STREAM(size, 4);
            WRITE_STREAM(pair.second.url.data()[0], size);
        }
        return true;
    }

}
