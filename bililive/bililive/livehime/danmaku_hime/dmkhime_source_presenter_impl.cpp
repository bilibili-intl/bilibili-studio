#include "bililive/bililive/livehime/danmaku_hime/dmkhime_source_presenter_impl.h"

#include <chrono>

#include "base/ext/bind_lambda.h"

#include "ui/gfx/canvas.h"

#include "bililive/public/bililive/bililive_thread.h"

#include "obs/obs-studio/libobs/obs-module.h"


// DmkhimeSourceEventHandler
DmkhimeSourceEventHandler::DmkhimeSourceEventHandler(contracts::DmkhimeSourceView* view)
    : view_(view) {
}

void DmkhimeSourceEventHandler::OnRefreshTexture(bool first) {
    WPtr weak_self = shared_from_this();

    BililiveThread::PostTask(
        BililiveThread::UI,
        FROM_HERE,
        base::BindLambda([weak_self, first]
    {
        if (auto ptr = weak_self.lock()) {
            auto view = static_cast<DmkhimeSourceEventHandler*>(ptr.get())->view_;
            if (view) {
                view->OnOutgoing(first);
            }
        }
    }));
}

void DmkhimeSourceEventHandler::OnAutism() {
    WPtr weak_self = shared_from_this();

    BililiveThread::PostTask(
        BililiveThread::UI,
        FROM_HERE,
        base::BindLambda([weak_self]
    {
        if (auto ptr = weak_self.lock()) {
            auto view = static_cast<DmkhimeSourceEventHandler*>(ptr.get())->view_;
            if (view) {
                view->OnAutism();
            }
        }
    }));
}


// DmkhimeSourcePresenterImpl
DmkhimeSourcePresenterImpl::DmkhimeSourcePresenterImpl(contracts::DmkhimeSourceView* view)
    : callback_(std::make_shared<DmkhimeSourceEventHandler>(view)) {}

bool DmkhimeSourcePresenterImpl::Initialize() {
    HMODULE hMod = ::GetModuleHandleW(L"obs-dmkhime.dll");
    if (!hMod) {
        PLOG(WARNING) << "Cannot load obs-dmkhime.dll!";
        return false;
    }

    using GetDmkhimeServiceFactory = bililive::IDmkhimeServiceFactory*(*)();
    auto factory = reinterpret_cast<GetDmkhimeServiceFactory>(GetProcAddress(hMod, "GetDmkhimeServiceFactory"));
    if (!factory) {
        LOG(WARNING) << "Cannot get proc address for GetDmkhimeServiceFactory()!";
        return false;
    }

    dmktex_service_ = factory()->Get();
    if (!dmktex_service_) {
        LOG(WARNING) << "Cannot fetch danmaku texture service!";
        return false;
    }

    return dmktex_service_->Launch(callback_);
}

void DmkhimeSourcePresenterImpl::Render(gfx::Canvas* canvas) {
    if (!dmktex_service_) return;
    if (!dmktex_service_->HasActivatedSource()) return;

    auto sk_bitmap = canvas->sk_canvas()->getDevice()->accessBitmap(false);
    auto cfg = sk_bitmap.config();
    if (cfg != SkBitmap::kARGB_8888_Config) {
        return;
    }

    obs_source_frame* result = obs_source_frame_create(
        VIDEO_FORMAT_BGRA, sk_bitmap.width(), sk_bitmap.height());
    if (!result) {
        return;
    }
    memcpy(result->data[0], sk_bitmap.pixelRef()->pixels(), sk_bitmap.width() * sk_bitmap.height() * 4);

    auto ts = std::chrono::steady_clock::now().time_since_epoch();
    result->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(ts).count() * 100;

    std::shared_ptr<obs_source_frame> sp_result(result, [](obs_source_frame* ptr) {
        obs_source_frame_destroy(ptr);
    });

    dmktex_service_->Dispatch(sp_result);
}