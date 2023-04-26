#ifndef MEDIA_ISO_BOX_BLPC_BOX_H_
#define MEDIA_ISO_BOX_BLPC_BOX_H_

#include "iso_box.h"


namespace media {
namespace iso {

    /**
     * 'blpc'
     * Container: File
     * Mandatory: Yes
     * Exactly one
     *
     * B站直播自定义 Box，包含一个 JSON，用于描述帧布局
     */
    class BLPCBox : public Box {
    public:
        explicit BLPCBox(const Box& b)
            : Box(b) {}

        bool parse(std::istream& s) override;

        std::shared_ptr<uint8_t> data;
        size_t data_count;
    };

}
}

#endif  // MEDIA_ISO_BOX_BLPC_BOX_H_