// Copyright (c) 2020 ucclkp <ucclkp@gmail.com>.
// This file is part of media project.
//
// This program is licensed under GPLv3 license that can be
// found in the LICENSE file.

#ifndef MEDIA_ISO_BOX_UNKNOWN_BOX_H_
#define MEDIA_ISO_BOX_UNKNOWN_BOX_H_

#include "iso_box.h"


namespace media {
namespace iso {

    /**
     * 自定义 Box，用于处理未知 Box
     */
    class UnknownBox : public Box {
    public:
        explicit UnknownBox(const Box& b)
            : Box(b) {}

        bool parse(std::istream& s) override;
    };

}
}

#endif  // MEDIA_ISO_BOX_UNKNOWN_BOX_H_