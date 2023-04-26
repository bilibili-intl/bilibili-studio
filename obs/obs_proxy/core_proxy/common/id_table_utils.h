#ifndef OBS_OBS_PROXY_CORE_PROXY_COMMON_ID_TABLE_UTILS_H_
#define OBS_OBS_PROXY_CORE_PROXY_COMMON_ID_TABLE_UTILS_H_

#include <algorithm>

#include "base/logging.h"

namespace obs_proxy {

template<typename E, size_t N>
const char* EnumValueToTableEntry(E value, const char* const (&table)[N])
{
    auto i = static_cast<std::underlying_type_t<decltype(value)>>(value);
    DCHECK(i < N);
    return table[i];
}

template<typename E, size_t N>
E TableEntryToEnumValue(const char* id, const char* const (&table)[N])
{
    DCHECK(id != nullptr && *id != '\0');
    auto it = std::find_if(std::begin(table), std::end(table), [id](const char* filter_id) {
                               return strcmp(id, filter_id) == 0;
                           });
    DCHECK(it != std::end(table));
    size_t i = std::distance(std::begin(table), it);
    return E(i);
}

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_CORE_PROXY_COMMON_ID_TABLE_UTILS_H_