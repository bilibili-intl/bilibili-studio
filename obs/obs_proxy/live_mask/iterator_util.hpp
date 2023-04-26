#pragma once

#include <iterator>

template<class It, class Diff, class T>
bool try_advance_impl(It& it, const Diff& diff, T)
{
    std::advance(it, diff);
    return true;
}

template<class It, class Diff>
bool try_advance_impl(It& it, const Diff& diff, std::output_iterator_tag)
{
    return false;
}

template<class It, class Diff>
bool try_advance(It& it, const Diff& diff)
{
    return try_advance_impl<>(it, diff, typename It::iterator_category());
}
