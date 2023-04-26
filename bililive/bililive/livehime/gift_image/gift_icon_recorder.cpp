#include "bililive/bililive/livehime/gift_image/gift_icon_recorder.h"

#include <fstream>

#include "base/file_util.h"

#define READ_STREAM(var, size)  \
    file.read(reinterpret_cast<char*>(&var), size);  \
    if (!file.good()) return false;

#define WRITE_STREAM(var, size)  \
    file.write(reinterpret_cast<const char*>(&var), size);  \
    if (!file.good()) return false;


namespace livehime {

    GiftIconRecorder::GiftIconRecorder()
        : header_() {}

    GiftIconRecorder::~GiftIconRecorder() {
        close();
    }

    void GiftIconRecorder::open(const std::wstring& path) {
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

    void GiftIconRecorder::put(int64_t id, const GiftIconData& data) {
        if (path_.empty()) {
            return;
        }

        contents_[id] = data;
    }

    bool GiftIconRecorder::get(int64_t id, GiftIconData* data) {
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

    void GiftIconRecorder::remove(int64_t id) {
        if (path_.empty()) {
            return;
        }

        auto it = contents_.find(id);
        if (it != contents_.end()) {
            contents_.erase(it);
        }
    }

    void GiftIconRecorder::close() {
        if (path_.empty()) {
            return;
        }

        if (!writeFile(path_)) {
            base::DeleteFileW(base::FilePath(path_), false);
        }
        path_.clear();
    }

    bool GiftIconRecorder::readFile(const std::wstring& path) {
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

        std::map<int64_t, GiftIconData> contents;

        // 读取礼物 id 和 url 的对应关系
        for (uint32_t i = 0; i < header.total_count; ++i) {

            GiftIconData data;
            uint32_t length;

            READ_STREAM(data.id, 8);
            READ_STREAM(length, 4);
            if (length == 0 || length >= 2048) {
                return false;
            }
            data.icon_url.resize(length);
            READ_STREAM(*data.icon_url.begin(), length);

            READ_STREAM(length, 4);
            if (length == 0 || length >= 2048) {
                return false;
            }
            data.gif_url.resize(length);
            READ_STREAM(*data.gif_url.begin(), length);

            contents[data.id] = data;
        }

        header_ = header;
        contents_ = std::move(contents);
        return true;
    }

    bool GiftIconRecorder::writeFile(const std::wstring& path) {
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
            WRITE_STREAM(pair.first, 8);

            uint32_t size = uint32_t(pair.second.icon_url.size());
            WRITE_STREAM(size, 4);
            WRITE_STREAM(pair.second.icon_url.data()[0], size);

            size = uint32_t(pair.second.gif_url.size());
            WRITE_STREAM(size, 4);
            WRITE_STREAM(pair.second.gif_url.data()[0], size);
        }

        return true;
    }

}
