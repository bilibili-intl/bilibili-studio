#include "bililive/bililive/livehime/sources_properties/source_camera_property_presenter_impl.h"

#include "base/ext/bind_lambda.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "obs/obs_proxy/utils/obs_wrapper_impl_cast.h"
#include "source_camera_property_presenter_impl.h"

namespace
{

template<class T>
static void FunctorTaskRunner(const std::function<T()>& x)
{
    x();
}

template<class T>
static base::Callback<void()> MakeThreadTask(const T& x)
{
    using R = decltype(x());
    return base::Bind(&FunctorTaskRunner<R>, std::function<R()>(x));
}


const int kUnsignedMin = 0;
const int kUnsignedMax = 100;

const int kMin = -50;
const int kMax = 50;

enum faceunity_use_status
{
    SDK_IS_NOT_USE = 0,
    SDK_IS_USE
};

static const char kCameraSignal[] = "camera_signal";
static const char kCameratatus[] = "camera_status";
static const char kCameraErrorSignal[] = "camera_error_signal";
static const char kLoadErrorPath[] = "camrea_load_error_path";
static const char kLoadErrorType[] = "camera_load_error_type";

}   // namespace

// SourceCameraPropertyPresenterImpl
SourceCameraPropertyPresenterImpl::SourceCameraPropertyPresenterImpl(
    obs_proxy::SceneItem* scene_item,
    contracts::SourceCameraPropertyView* view,
    contracts::SourceCameraCallback* callback,
    bool need_regist_signal, const std::string& tab_type)
    : contracts::SourceCameraPropertyPresenter(scene_item),
      callback_(callback),
      need_regist_signal_(need_regist_signal),
      camera_scene_item_(scene_item),
      view_(view),
      wptr_factory_(this)
{
    canary_.reset(new int(0));
}

bool SourceCameraPropertyPresenterImpl::Initialize() {
    if (need_regist_signal_)
    {
        auto source = impl_cast(camera_scene_item_.GetItem())->AsSource();
        RegistSignal(source);
    }
    return true;
}

float SourceCameraPropertyPresenterImpl::GetVolumeValue()
{
    return MapFloatFromInt(camera_scene_item_.Volume());
}

void SourceCameraPropertyPresenterImpl::SetVolumeValue(float value)
{
    camera_scene_item_.Volume(MapFloatToInt(value));
}

bool SourceCameraPropertyPresenterImpl::IsMuted()
{
    return camera_scene_item_.IsMuted();
}

void SourceCameraPropertyPresenterImpl::SetMuted(bool muted)
{
    camera_scene_item_.SetMuted(muted);
}

int SourceCameraPropertyPresenterImpl::GetAudioMonitoringType()
{
    return camera_scene_item_.GetMoinitoringType();
}

void SourceCameraPropertyPresenterImpl::SetAudioMonitoringType(int status)
{
    return camera_scene_item_.SetMonitoringType(status);
}

PropertyList<std::string> SourceCameraPropertyPresenterImpl::GetCameraList(const string16& default_item)
{
    auto list = camera_scene_item_.CameraList();
    if (!list.empty() && !default_item.empty())
    {
        list.insert(list.begin(), std::make_tuple(default_item, std::string{}, true));
    }

    return list;
}

std::string SourceCameraPropertyPresenterImpl::GetSelectedCamera()
{
    return camera_scene_item_.SelectedCamera();
}

void SourceCameraPropertyPresenterImpl::SetSelectedCamera(const std::string& value)
{
    camera_scene_item_.SelectedCamera(value);
}


std::string SourceCameraPropertyPresenterImpl::SelectedAudioDevice()
{
    return camera_scene_item_.SelectedAudioDevice();
}

void SourceCameraPropertyPresenterImpl::SelectedAudioDevice(std::string val)
{
    camera_scene_item_.SelectedAudioDevice(val);
}

bool SourceCameraPropertyPresenterImpl::IsCustomAudioDevice()
{
    return camera_scene_item_.IsCustomAudioDevice();
}

void SourceCameraPropertyPresenterImpl::SetCustomAudioDevice(bool val)
{
    camera_scene_item_.SetCustomAudioDevice(val);
}

PropertyList<std::string> SourceCameraPropertyPresenterImpl::CustomAudioDeviceList()
{
    return camera_scene_item_.CustomAudioDeviceList();
}

void SourceCameraPropertyPresenterImpl::LaunchConfig()
{
    camera_scene_item_.LaunchConfig();
}

void SourceCameraPropertyPresenterImpl::VerticalFlip()
{
    camera_scene_item_.VerticalFlip();
}

void SourceCameraPropertyPresenterImpl::HorizontalFlip()
{
    camera_scene_item_.HorizontalFlip();
}

PropertyList<std::string> SourceCameraPropertyPresenterImpl::GetResolutionList(const string16& default_item)
{
    auto list = camera_scene_item_.ResolutionList();

    if (!list.empty())
    {
        std::get<0>(list.front()) = default_item;
    }

    auto newend = std::remove_if(list.begin(), list.end(), [](const std::tuple<std::wstring, std::string, bool>& item)
    {
        if (std::get<1>(item).empty())
        {
            return false;
        }

        int width = 0, height = 0, useless = 0;
        char c = 0;
        if (sscanf(std::get<1>(item).c_str(), "%d%c%d%c", &width, &c, &height, (char*)&useless) == 3)
        {
            if (c == 'x' && width <= 1920 && height <= 1920)
            {
                return false;
            }
        }

        return true;
    });

    list.erase(newend, list.end());

    return list;
}
std::string SourceCameraPropertyPresenterImpl::GetSelectedResolution()
{
    return camera_scene_item_.SelectedResolution();
}

void SourceCameraPropertyPresenterImpl::SetSelectedResolution(const std::string& value)
{
    camera_scene_item_.SelectedResolution(value);
}

PropertyList<int64> SourceCameraPropertyPresenterImpl::GetAudioOutputList()
{
    return camera_scene_item_.AudioOutputList();
}
int64 SourceCameraPropertyPresenterImpl::GetSelectedAudioOutput()
{
    return camera_scene_item_.SelectedAudioOutput();
}

void SourceCameraPropertyPresenterImpl::SetSelectedAudioOutput(int64 value)
{
    camera_scene_item_.SelectedAudioOutput(value);
}

void SourceCameraPropertyPresenterImpl::RegisterSceneItemDeferredUpdate()
{
    // 生一个弱指针出来
    auto wptr = wptr_factory_.GetWeakPtr();

    RegisterSceneItemDeferredUpdateCallback(camera_scene_item_.GetItem(), [this, wptr](){
        auto functor = [this, wptr]() {
            // 看下弱指针指向的对象是不是已经死了
            if (wptr.get()) {
                DCHECK(view_);
                view_->RefreshResolutionListCallback();

                LOG(INFO) << "[camera source property dialog] device changed, updated";
            }
        };

        BililiveThread::PostTask(BililiveThread::UI,
            FROM_HERE,
            MakeThreadTask(functor)
            );
    });

    LOG(INFO) << "[camera source property dialog] device changed, updating";

    Update();
}

std::vector<std::wstring> SourceCameraPropertyPresenterImpl::LoadColorFilterList(
    const string16& filter_str)
{
    std::vector<std::wstring> color_filter;

    int index = 0;
    size_t from_pos = 0;
    while (from_pos < filter_str.size())
    {
        size_t end_pos = filter_str.find(L'#', from_pos);
        if (end_pos == std::wstring::npos)
        {
            break;
        }
        color_filter.push_back(filter_str.substr(from_pos, end_pos - from_pos));

        ++index;
        from_pos = end_pos + 1;
    }

    return color_filter;
}

void SourceCameraPropertyPresenterImpl::CameraSnapshot()
{
    camera_snapshot_ = livehime::CameraPropertySnapshot::NewTake(&camera_scene_item_);
}

void SourceCameraPropertyPresenterImpl::CameraRestore()
{
    DCHECK(camera_snapshot_);

    camera_snapshot_->Restore();
}

void SourceCameraPropertyPresenterImpl::RegistSignal(const obs_source_t* source)
{
    auto signal_handle = obs_source_get_signal_handler(source);
    camera_signal_.Connect(signal_handle, kCameraSignal, &SourceCameraPropertyPresenterImpl::OnCameraSignal, this);
}

// static
void SourceCameraPropertyPresenterImpl::OnCameraSignal(void* data, calldata_t* params)
{
    auto This = static_cast<SourceCameraPropertyPresenterImpl*>(data);
    int status = static_cast<int>(calldata_int(params, kCameratatus));

    std::weak_ptr<int> weak_self = This->canary_;
    if (status == SDK_IS_NOT_USE)
    {
        BililiveThread::PostTask(
            BililiveThread::UI,
            FROM_HERE,
            base::BindLambda([weak_self, This]
        {
            auto ptr = weak_self.lock();
            if (ptr && This->callback_)
            {
                This->callback_->CameraShowBeauty(SDK_IS_NOT_USE);
            }
        }));
    }
    else if (status == SDK_IS_USE)
    {
        BililiveThread::PostTask(
            BililiveThread::UI,
            FROM_HERE,
            base::BindLambda([weak_self, This]
        {
            auto ptr = weak_self.lock();
            if (ptr && This->callback_)
            {
                This->callback_->CameraShowBeauty(SDK_IS_USE);
            }
        }));
    }
}

void SourceCameraPropertyPresenterImpl::ClockwiseRotateCamera() {

    camera_scene_item_.ColckwiseRotate();
}

float SourceCameraPropertyPresenterImpl::GetSourcesItemRotateVal() {
    return camera_scene_item_.GetSourcesItemRotateVal();
}

//垂直翻转保存
void SourceCameraPropertyPresenterImpl::SetVerticalFlip(bool val)
{
    camera_scene_item_.SetVerticalFlip(val);
}
bool SourceCameraPropertyPresenterImpl::GetVerticalFlip()
{
    return camera_scene_item_.GetVerticalFlip();
}

//水平翻转保存
void SourceCameraPropertyPresenterImpl::SetHorizontalFlip(bool val)
{
    camera_scene_item_.SetHorizontalFlip(val);
}
bool SourceCameraPropertyPresenterImpl::GetHorizontalFlip()
{
    return camera_scene_item_.GetHorizontalFlip();
}

//90度旋转保存
void SourceCameraPropertyPresenterImpl::SetCameraRotate(int val)
{
    camera_scene_item_.SetCameraRotate(val);
}

int SourceCameraPropertyPresenterImpl::GetCameraRotate()
{
    return camera_scene_item_.GetCameraRotate();
}

void SourceCameraPropertyPresenterImpl::SetCameraXScale(bool val)
{
    camera_scene_item_.SetCameraXScale(val);
}

bool SourceCameraPropertyPresenterImpl::GetCameraXScale()
{
    return camera_scene_item_.GetCameraXScale();
}

void SourceCameraPropertyPresenterImpl::SetCameraYScale(bool val)
{
    camera_scene_item_.SetCameraYScale(val);
}

bool SourceCameraPropertyPresenterImpl::GetCameraYScale()
{
    return camera_scene_item_.GetCameraYScale();
}
