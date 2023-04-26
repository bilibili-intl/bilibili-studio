/*
 @ 0xCCCCCCCC
*/

#ifndef BILIBASE_DATA_VIEW_H_
#define BILIBASE_DATA_VIEW_H_

namespace bilibase {

// A simplified and well tailored implementation of gsl::span.
template<typename T>
class data_view {
public:
    using value_type = T;
    using const_pointer = const T*;
    using const_iterator = const_pointer;

    data_view()
        : ptr_(nullptr), size_(0)
    {}

    data_view(const T* ptr, size_t size)
        : ptr_(ptr), size_(size)
    {}

    // TODO: construct from array or contiguous containers.

    ~data_view() = default;

    bool empty() const
    {
        return size_ == 0;
    }

    size_t size() const
    {
        return size_;
    }

    const_pointer data() const
    {
        return ptr_;
    }

    const_iterator begin() const
    {
        return ptr_;
    }

    const_iterator end() const
    {
        return ptr_ + size_;
    }

private:
    const T* ptr_;
    size_t size_;
};

template<typename T>
data_view<T> make_data_view(const T* ptr, size_t size)
{
    return data_view<T>(ptr, size);
}

}   // namespace bilibase

#endif  // BILIBASE_DATA_VIEW_H_
