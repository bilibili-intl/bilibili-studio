// Copyright (c) 2020 ucclkp <ucclkp@gmail.com>.
// This file is part of media project.
//
// This program is licensed under GPLv3 license that can be
// found in the LICENSE file.

#ifndef MEDIA_ISO_ISO_CONSTANTS_H_
#define MEDIA_ISO_ISO_CONSTANTS_H_

#include <cstdint>


namespace media {
namespace iso {

    const uint32_t kBoxType_uuid = 0x75756964;
    const uint32_t kBoxType_blpc = 'blpc';

    const uint32_t kBoxType_free = 0x66726565;
    const uint32_t kBoxType_skip = 0x736B6970;

}
}

#endif  // MEDIA_ISO_ISO_CONSTANTS_H_