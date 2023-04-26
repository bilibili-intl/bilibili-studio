/*
 @ 0xCCCCCCCC
*/

#include "bililive/bililive/utils/sequenced_image_package.h"

#include "base/file_util.h"
#include "base/logging.h"
#include "base/threading/thread_restrictions.h"

namespace {

bool ReadFileToString(const base::FilePath& path, std::string& data)
{
    auto real_path = path.ReferencesParent() ? base::MakeAbsoluteFilePath(path) : path;
    return file_util::ReadFileToString(real_path, &data);;
}

}   // namespace

namespace bililive {

ImageChunkRef::ImageChunkRef(uint32_t id, const char* chunk_ptr)
    : id_(id), chunk_ptr_(chunk_ptr)
{
    image_count_ = *reinterpret_cast<const uint32_t*>(chunk_ptr);
    img_info_ptr_ = reinterpret_cast<const ImageInfo*>(chunk_ptr + sizeof(uint32));
}

ImageChunkRef::DataRange ImageChunkRef::GetImageAt(size_t index) const
{
    DCHECK(index < image_count());

    auto target_image_info = img_info_ptr_ + index;
    auto data = chunk_ptr_ + target_image_info->offset;
    auto size = target_image_info->size;

    return DataRange(data, size);
}

std::vector<ImageChunkRef::DataRange> ImageChunkRef::GetAllImages() const
{
    return std::vector<ImageChunkRef::DataRange>(begin(), end());
}

ImageChunkRef::iterator ImageChunkRef::begin() const
{
    return iterator(*this);
}

ImageChunkRef::iterator ImageChunkRef::end() const
{
    return iterator(*this, end_iterator_tag());
}

// -*- ImageChunkRef::Iterator implementations -*-

ImageChunkRef::Iterator::Iterator(const ImageChunkRef& chunk)
    : chunk_ptr_(chunk.chunk_ptr_), current_img_info_ptr_(chunk.img_info_ptr_)
{
    ReadImageData();
}

ImageChunkRef::Iterator::Iterator(const ImageChunkRef& chunk, end_iterator_tag)
    : chunk_ptr_(chunk.chunk_ptr_), current_img_info_ptr_(chunk.img_info_ptr_ + chunk.image_count_)
{}

ImageChunkRef::Iterator& ImageChunkRef::Iterator::operator++()
{
    ++current_img_info_ptr_;

    ReadImageData();

    return *this;
}

ImageChunkRef::Iterator::reference ImageChunkRef::Iterator::operator*() const
{
    return image_data_;
}

ImageChunkRef::Iterator::pointer ImageChunkRef::Iterator::operator->() const
{
    return &image_data_;
}

bool operator==(const ImageChunkRef::Iterator& lhs, const ImageChunkRef::Iterator& rhs)
{
    return lhs.chunk_ptr_ == rhs.chunk_ptr_ &&
           lhs.current_img_info_ptr_ == rhs.current_img_info_ptr_;
}

bool operator!=(const ImageChunkRef::Iterator& lhs, const ImageChunkRef::Iterator& rhs)
{
    return !(lhs == rhs);
}

void ImageChunkRef::Iterator::ReadImageData()
{
    auto data = chunk_ptr_ + current_img_info_ptr_->offset;
    auto size = current_img_info_ptr_->size;
    image_data_ = value_type(data, size);
}

// -*- SequencedImagePackaged implementations -*-

SequencedImagePackage::SequencedImagePackage(const base::FilePath& package_path)
{
    {
        base::ThreadRestrictions::ScopedAllowIO allow_io;
        CHECK(ReadFileToString(package_path, packaged_data_));
    }

    Resolve();
}

void SequencedImagePackage::Resolve()
{
    const char* data_ptr = packaged_data_.data();
    auto read_ptr = reinterpret_cast<const uint32_t*>(data_ptr);

    uint32_t num_of_chunks = *read_ptr++;
    chunks_.reserve(num_of_chunks);
    for (uint32_t i = 0; i < num_of_chunks; ++i) {
        uint32_t id = *read_ptr++;
        uint32_t chunk_offset = *read_ptr++;
        const char* chunk_ptr = data_ptr + chunk_offset;
        chunks_.push_back(ImageChunkRef(id, chunk_ptr));
    }
}

const ImageChunkRef* SequencedImagePackage::GetChunkForID(uint32_t id) const
{
    auto it = std::find_if(chunks_.begin(), chunks_.end(), [id](const ImageChunkRef& chunk) {
                  return chunk.id() == id;
              });

    return it == chunks_.end() ? nullptr : &(*it);
}

}   // namespace bililive
