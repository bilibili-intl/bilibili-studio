#include "blpc_box.h"

#include <cassert>

#include "utils/stream_utils.h"


namespace media {
namespace iso {

    bool BLPCBox::parse(std::istream& s) {
        uint64_t remainder;
        if (size > 0) {
            auto box_size = getBoxFieldSize();
            assert(box_size <= size);
            remainder = size - box_size;
        } else {
            // 直到文件末尾
            auto prev_pos = s.tellg();
            s.seekg(0, std::ios::end);
            remainder = s.tellg() - prev_pos;
            s.seekg(prev_pos, std::ios::beg);
        }

        if (remainder == 0) {
            return true;
        }

        size_t count = size_t(remainder);

        data.reset(new uint8_t[count]);
        READ_STREAM(data.get()[0], count);
        data_count = count;

        return true;
    }

}
}