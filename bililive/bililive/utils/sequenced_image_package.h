/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_BILILIVE_UTILS_SEQUENCED_IMAGE_PACKAGE_H_
#define BILILIVE_BILILIVE_UTILS_SEQUENCED_IMAGE_PACKAGE_H_

#include <cstdint>

#include "base/files/file_path.h"

#include "bilibase/basic_macros.h"
#include "bilibase/data_view.h"

namespace bililive {

// For online doc and packing tool, see http://info.bilibili.co/pages/viewpage.action?pageId=8725138.

class ImageChunkRef {
public:
    using DataRange = bilibase::data_view<char>;

    struct ImageInfo {
        uint32_t offset;
        uint32_t size;
    };

    struct end_iterator_tag {};

    // Theoretically, this iterator can be random-access, but making so would cost me a lot of work.
    // So for now we just assume having only forward iteration is enough.
    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = DataRange;
        using difference_type = ptrdiff_t;
        using pointer = const DataRange*;
        using reference = const DataRange&;

        explicit Iterator(const ImageChunkRef& chunk);

        Iterator(const ImageChunkRef& chunk, end_iterator_tag);

        ~Iterator() = default;

        Iterator& operator++();

        reference operator*() const;

        pointer operator->() const;

        friend bool operator==(const Iterator& lhs, const Iterator& rhs);

        friend bool operator!=(const Iterator& lhs, const Iterator& rhs);

    private:
        void ReadImageData();

    private:
        const char* chunk_ptr_;
        const ImageInfo* current_img_info_ptr_;
        value_type image_data_;
    };

    using iterator = Iterator;

    ImageChunkRef(uint32_t id, const char* chunk_ptr);

    ~ImageChunkRef() = default;

    uint32_t id() const
    {
        return id_;
    }

    uint32_t image_count() const
    {
        return image_count_;
    }

    // Zero-based sequence index.
    DataRange GetImageAt(size_t index) const;

    std::vector<DataRange> GetAllImages() const;

    iterator begin() const;

    iterator end() const;

private:
    uint32_t id_;
    uint32_t image_count_;
    const char* chunk_ptr_;
    const ImageInfo* img_info_ptr_;
};

// A SequencedImagePackage instance loads a consolidated image package off the disk, and it consistents
// of one or several image chunks which further stashes images in order.
class SequencedImagePackage {
public:
    explicit SequencedImagePackage(const base::FilePath& pack_path);

    ~SequencedImagePackage() = default;

    DISABLE_COPY(SequencedImagePackage);

    const ImageChunkRef* GetChunkForID(uint32_t id) const;

    const std::vector<ImageChunkRef>& chunks() const
    {
        return chunks_;
    }

private:
    void Resolve();

private:
    std::string packaged_data_;
    std::vector<ImageChunkRef> chunks_;
};

}   // namespace bililive

#endif  // BILILIVE_BILILIVE_UTILS_SEQUENCED_IMAGE_PACKAGE_H_
