// Copyright (c) 2020 ucclkp <ucclkp@gmail.com>.
// This file is part of media project.
//
// This program is licensed under GPLv3 license that can be
// found in the LICENSE file.

#include "iso_box.h"

#include <cassert>

#include "utils/stream_utils.h"

#include "../iso_constants.h"
#include "media/iso/box/skip_box.h"


namespace media {
namespace iso {

    // Box
    uint32_t Box::getBoxFieldSize() const {
        return box_field_size_;
    }

    const std::vector<std::shared_ptr<Box>>& Box::getChildren() const {
        return children_;
    }

    bool Box::parse(std::istream& s) {
        auto prev_pos = s.tellg();

        uint32_t size32;
        READ_STREAM_BE(size32, 4);
        READ_STREAM_BE(type, 4);
        if (size32 == 1) {
            READ_STREAM_BE(size, 8);
        } else {
            size = size32;
        }
        if (type == kBoxType_uuid) {
            READ_STREAM(user_type[0], 16);
        }

        box_field_size_ = uint32_t(s.tellg() - prev_pos);
        return true;
    }

    bool Box::isContainer() const {
        return false;
    }

    std::string Box::toString() const {
        return "";
    }

    bool Box::parseContainer(std::istream& s, std::function<bool(const Box&)>&& f) {
        auto end_pos = s.tellg();
        end_pos += size - getBoxFieldSize();

        for (;;) {
            auto pos = s.tellg();
            // 超出表大小，解析失败
            if (pos > end_pos) {
                return false;
            }
            // 到达表结尾，返回
            if (pos == end_pos) {
                break;
            }

            Box box;
            if (!box.parse(s)) {
                return false;
            }

            if (box.type == kBoxType_free ||
                box.type == kBoxType_skip)
            {
                SkipBox skip_box(box);
                if (!skip_box.parse(s)) return false;
            } else {
                if (!f(box)) {
                    return false;
                }
            }

            // 确保尾部一致
            if (s.tellg() - pos != box.size) {
                assert(false);
                return false;
            }
        }

        return true;
    }


    // FullBox
    FullBox::FullBox(const Box& b)
        : Box(b) {}

    bool FullBox::parse(std::istream& s) {
        READ_STREAM(version, 1);
        flags = 0;
        READ_STREAM_MLBE(flags, 3);

        return true;
    }

}
}