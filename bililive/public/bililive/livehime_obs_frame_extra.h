#pragma once

#include <atomic>
#include <vector>
#include <typeindex>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>

#include "obs/obs-studio/libobs/obs.h"

namespace bililive
{
    struct LiveMaskData
    {
        std::atomic_bool done_flag;

        int width;
        int height;
        int stride;
        std::vector<uint8_t> mask_data;

        LiveMaskData()
            : done_flag(false)
            , width(0)
            , height(0)
            , stride(0)
        {
        }

        virtual ~LiveMaskData()
        {
        };
    };

#define OBS_FRAME_EXTRA_BILILIVE 1

    class OBSFrameExtra;
    using OBSFrameExtraPtr = boost::intrusive_ptr<OBSFrameExtra>;

    static class OBSFrameExtraOp : public obs_source_frame_extra_op
    {
    public:
        OBSFrameExtraOp()
        {
            as_type = [](int type, void* ctx)->void*
            {
                if (type == OBS_FRAME_EXTRA_BILILIVE)
                    return static_cast<OBSFrameExtra*>(ctx);
                else
                    return nullptr;
            };

            destroy = [](void* ctx)->void
            {
                auto thiz = static_cast<OBSFrameExtra*>(ctx);
                OBSFrameExtraPtr(thiz, false);
            };

            copy = [](void* ctx)->void*
            {
                auto thiz = static_cast<OBSFrameExtra*>(ctx);
                return OBSFrameExtraPtr(thiz).detach();
            };
        }
    } s_obs_frame_extra_op;


    class OBSFrameExtra : public boost::intrusive_ref_counter<OBSFrameExtra>
    {
        LiveMaskData live_mask_data_;

    public:
        virtual ~OBSFrameExtra()
        {
        }

        OBSFrameExtra* AddRef()
        {
            return OBSFrameExtraPtr(this).detach();
        }

        void Release()
        {
            OBSFrameExtraPtr(this, false);
        }

        obs_source_frame_extra_op* get_op()
        {
            return &s_obs_frame_extra_op;
        }

        LiveMaskData& live_mask()
        {
            return live_mask_data_;
        }
    };
};
