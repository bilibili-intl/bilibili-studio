// Copyright (c) 2020 ucclkp <ucclkp@gmail.com>.
// This file is part of media project.
//
// This program is licensed under GPLv3 license that can be
// found in the LICENSE file.

#ifndef MEDIA_ISO_BOX_SKIP_BOX_H_
#define MEDIA_ISO_BOX_SKIP_BOX_H_

#include "iso_box.h"


namespace media {
namespace iso {

    /**
     * 'free','skip'
     * Container: File or other box
     * Mandatory: No
     * Zero or more
     */
    class SkipBox : public Box {
    public:
        explicit SkipBox(const Box& b)
            : Box(b) {}

        bool parse(std::istream& s) override;

        std::shared_ptr<uint8_t> data;
        size_t data_count;
    };

}
}

#endif  // MEDIA_ISO_BOX_SKIP_BOX_H_