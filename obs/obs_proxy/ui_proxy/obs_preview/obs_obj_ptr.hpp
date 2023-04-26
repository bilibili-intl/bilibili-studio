#ifndef BILILIVE_OBS_OBS_PROXY_UI_PROXY_OBS_PREVIEW_OBS_OBJ_PTR_H_
#define BILILIVE_OBS_OBS_PROXY_UI_PROXY_OBS_PREVIEW_OBS_OBJ_PTR_H_

#include <memory>


template <typename Ty>
class OBSObjWrapper {
public:
    OBSObjWrapper() {}
    ~OBSObjWrapper() {}

    Ty obj;
};

template <typename Ty>
using OBSObjPtr = std::shared_ptr<OBSObjWrapper<Ty>>;


#endif  // BILILIVE_OBS_OBS_PROXY_UI_PROXY_OBS_PREVIEW_OBS_OBJ_PTR_H_