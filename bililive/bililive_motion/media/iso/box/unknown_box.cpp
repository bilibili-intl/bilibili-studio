// Copyright (c) 2020 ucclkp <ucclkp@gmail.com>.
// This file is part of media project.
//
// This program is licensed under GPLv3 license that can be
// found in the LICENSE file.

#include "unknown_box.h"

#include "utils/stream_utils.h"


namespace media {
namespace iso {

    bool UnknownBox::parse(std::istream& s) {
        SKIP_BYTES(size - getBoxFieldSize());
        return true;
    }

}
}
