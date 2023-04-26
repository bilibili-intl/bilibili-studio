#ifndef BILILIVE_ROOMURL_TAG_DEFINE_H
#define BILILIVE_ROOMURL_TAG_DEFINE_H

#include <string>

#include "base/logging.h"

class BililiveRoomurlTag {
public:
    enum TagType {
        SETUP_LIVEROOM = 0,   //"open"
        ENTER_LIVEROOM,       //""
        ROOM_MANAGE,          //"admin"
        INFO_MODIFY           //"info"
    };

    BililiveRoomurlTag(TagType type)
    {
        switch (type)
        {
        case SETUP_LIVEROOM:
            tag_ = "open";
            break;
        case ENTER_LIVEROOM:
            break;
        case ROOM_MANAGE:
            tag_ = "admin";
            break;
        case INFO_MODIFY:
            tag_ = "info";
            break;
        default:
            NOTREACHED();
            break;
        }
    }

    const std::string tag() const
    {
        return tag_;
    }

private:
    std::string tag_;
};

#endif