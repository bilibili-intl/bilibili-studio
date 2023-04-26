#include "srcprop_ui_util.h"

#include "SkPixelRef.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"

#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"
#include "obs/obs_proxy/public/proxy/obs_volume_controller.h"
#include "obs/obs_proxy/utils/obs_wrapper_impl_cast.h"

#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive_motion/bililive_motion.h"


using namespace obs_proxy;

namespace {

    const wchar_t windows_internal_prefix[] = L"[WindowsInternal.ComposableShell.Experiences";

    inline int ReverseColorInt(int val)
    {
        //ARGB <-> ABGR
        //for little endian only
        int tmp = 0xff;
        assert(*(unsigned char*)&tmp == 0xff);
        assert(sizeof(int) == 4);

        char* s = (char*)&val;
        char* e = s + sizeof(val);
        std::reverse(s, e - 1);
        return val;
    }


    template<class T>
    T GetPropertyValue(obs_proxy::PropertyValue& srcConfig, const std::string& name, const T& defValue = T())
    {
        T r;
        if (srcConfig.Get(name, r))
            return r;
        else
            return defValue;
    }

    template<>
    inline obs_proxy::PropertyValue GetPropertyValue<obs_proxy::PropertyValue>(obs_proxy::PropertyValue& srcConfig, const std::string& name, const obs_proxy::PropertyValue& defValue)
    {
        obs_proxy::PropertyValue r;
        srcConfig.Get(name, r);
        return r;
    }

    template<>
    inline int GetPropertyValue<int>(obs_proxy::PropertyValue& srcConfig, const std::string& name, const int& defValue)
    {
        return GetPropertyValue<long long>(srcConfig, name, defValue);
    }

    template<>
    inline std::wstring GetPropertyValue<std::wstring>(obs_proxy::PropertyValue& srcConfig, const std::string& name, const std::wstring& defValue)
    {
        return base::UTF8ToWide(GetPropertyValue<std::string>(srcConfig, name));
    }

    template<>
    inline SkColor GetPropertyValue<SkColor>(obs_proxy::PropertyValue& srcConfig, const std::string& name, const SkColor& defValue)
    {
        return ReverseColorInt(GetPropertyValue<int>(srcConfig, name, ReverseColorInt(defValue)));
    }

    template<class T>
    void SetPropertyValue(obs_proxy::PropertyValue& srcConfig, const std::string& name, const T& value)
    {
        srcConfig.Set(name, value);
    }

    inline void SetPropertyValue(obs_proxy::PropertyValue& srcConfig, const std::string& name, obs_proxy::PropertyValue&& value)
    {
        srcConfig.Set(name, std::move(value));
    }

    template<>
    inline void SetPropertyValue(obs_proxy::PropertyValue& srcConfig, const std::string& name, const int& value)
    {
        SetPropertyValue<long long>(srcConfig, name, value);
    }

    template<>
    inline void SetPropertyValue(obs_proxy::PropertyValue& srcConfig, const std::string& name, const std::wstring& value)
    {
        SetPropertyValue(srcConfig, name, WideToUTF8(value));
    }

    template<>
    inline void SetPropertyValue(obs_proxy::PropertyValue& srcConfig, const std::string& name, const SkColor& value)
    {
        SetPropertyValue(srcConfig, name, ReverseColorInt(value));
    }


    obs_proxy::Filter* GetOrCreateFilter(obs_proxy::SceneItem* sceneItem, obs_proxy::FilterType filterType, const std::string& filterName);

}



Filter* GetOrCreateFilter(SceneItem* sceneItem, FilterType filterType, const std::string& filterName)
{
    if (sceneItem == nullptr)
        return nullptr;

    Filter* r = sceneItem->GetFilter(filterName);
    if (!r)
        r = sceneItem->AddNewFilter(filterType, filterName);

    return r;
}

template<class T1, class T2>
static PropertyList<T1> GetPropertyList(T1 useless, const Properties& prop, const std::string& name, const T2& GetVal)
{
    PropertyList<T1> r;
    const Properties::iterator propItem = prop.find(name);
    if (propItem != prop.end())
    {
        auto propItemImpl = property_cast<ListProperty>(*propItem);
        int count = propItemImpl->GetItemCount();
        for (int i = 0; i < count; ++i)
        {
            r.push_back(std::make_tuple(base::UTF8ToWide(propItemImpl->GetItemName(i)), GetVal(*propItemImpl, i), propItemImpl->GetItemEnableStatus(i)));
        }
    }

    return r;
}

PropertyList<std::string> GetPropertyStringList(const Properties& prop, const std::string& name)
{
    return GetPropertyList(std::string(), prop, name, [](const ListProperty& propImpl, int i)->std::string { return propImpl.GetItemValueAsString(i); });
}

PropertyList<int64> GetPropertyInt64List(const Properties& prop, const std::string& name)
{
    return GetPropertyList(int64(), prop, name, [](const ListProperty& propImpl, int i)->int64 { return propImpl.GetItemValueAsInt(i); });
}

int MapFloatToInt(float val, int minVal, int maxVal)
{
    assert(maxVal > minVal);
    int range = maxVal - minVal;
    return std::round(val * range + minVal);
}

float MapFloatFromInt(int val, int minVal, int maxVal)
{
    assert(maxVal > minVal);
    int range = maxVal - minVal;
    return float(val - minVal) / range;
}

static obs_proxy::SceneCollection* CurrentSceneCollection()
{
    return OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
}

void SetSceneItemVolume(obs_proxy::SceneItem* sceneItem, float volume)
{
    auto volCtrl = CurrentSceneCollection()->GetVolumeControllerForAudioSource(sceneItem);
    if (volCtrl)
        volCtrl->SetDeflection(volume);
}

float GetSceneItemVolume(obs_proxy::SceneItem* sceneItem)
{
    auto volCtrl = CurrentSceneCollection()->GetVolumeControllerForAudioSource(sceneItem);
    if (volCtrl)
        return volCtrl->GetDeflection();
    else
        return 1;
}


bool SceneItemIsMuted(obs_proxy::SceneItem* sceneItem)
{
    auto volCtrl = CurrentSceneCollection()->GetVolumeControllerForAudioSource(sceneItem);
    if (volCtrl)
        return volCtrl->IsMuted();
    else
    {
        return false;
    }
}

void SceneItemSetMuted(obs_proxy::SceneItem* sceneItem, bool muted)
{
    auto volCtrl = CurrentSceneCollection()->GetVolumeControllerForAudioSource(sceneItem);
    if (volCtrl)
        volCtrl->SetMuted(muted);
}

int SceneItemGetMoinitoringType(obs_proxy::SceneItem* sceneItem)
{
    auto volCtrl = CurrentSceneCollection()->GetVolumeControllerForAudioSource(sceneItem);
    if (volCtrl)
        return volCtrl->GetMonitoringType();
    return 0;
}

void SceneItemSetMonitoringType(obs_proxy::SceneItem* sceneItem, int status)
{
    auto volCtrl = CurrentSceneCollection()->GetVolumeControllerForAudioSource(sceneItem);
    if (volCtrl)
        volCtrl->SetMonitoringType(status);
}


static void CallSceneItemDeferredUpdateCallback(void* p)
{
    auto f = static_cast<std::function<void()>*>(p);
    (*f)();
    delete f;
}

void RegisterSceneItemDeferredUpdateCallback(SceneItem* sceneItem, std::function<void()>&& procedure)
{
    if (sceneItem->IsDeferredUpdatedHanlderInstalled() == false)
    {
        std::function<void()>* realProc = new std::function<void()>();
        *realProc = std::move(procedure);

        std::function<void()>* wrapProc = new std::function<void()>();
        *wrapProc = [sceneItem, realProc](){
            sceneItem->InstallDeferredUpdatedHandler(nullptr, nullptr);
            (*realProc)();
            delete realProc;
        };

        sceneItem->InstallDeferredUpdatedHandler(CallSceneItemDeferredUpdateCallback, wrapProc);
    }
}

/////////////////////////////////////////////////////////////////////
// SourceHelper
/////////////////////////////////////////////////////////////////////

template class SourceHelper < obs_proxy::AudioDevice >;
template class SourceHelper < obs_proxy::SceneItem >;

template<class SourceType>
SourceHelper<SourceType>::SourceHelper(SourceType* source)
    : sourceItem_(source)
{
}

template<class SourceType>
SourceHelper<SourceType>::~SourceHelper()
{
}

template<class SourceType>
SourceType* SourceHelper<SourceType>::GetItem()
{
    return sourceItem_;
}

template<class SourceType>
void SourceHelper<SourceType>::Update()
{
    sourceItem_->UpdatePropertyValues();

    UpdateFilters();
}

template<class SourceType>
void SourceHelper<SourceType>::UpdateFilters()
{
    for (auto flt : sourceItem_->GetFilters())
    {
        flt->UpdatePropertyValues();
    }
}

template<class SourceType>
std::wstring SourceHelper<SourceType>::Name()
{
    return base::UTF8ToWide(sourceItem_->name());
}

template<class SourceType>
bool SourceHelper<SourceType>::EnsureFilterExists(obs_proxy::FilterType filterType, const char* filterName)
{
    auto flt = sourceItem_->GetFilter(filterName);
    if (flt)
        return true;

    flt = sourceItem_->AddNewFilter(filterType, filterName);
    if (flt)
    {
        auto filterOrder = GetFilterOrder();
        if (!filterOrder.empty())
        {
            std::vector<Filter*> newOrder;
            for (auto& x : filterOrder)
            {
                auto x_flt = sourceItem_->GetFilter(x);
                if (x_flt)
                    newOrder.push_back(x_flt);
            }

            auto existFilters = sourceItem_->GetFilters();
            for (auto& x : existFilters)
            {
                if (std::find(newOrder.begin(), newOrder.end(), x) == newOrder.end())
                {
                    newOrder.push_back(x);
                }
            }

            sourceItem_->ReorderFilters(newOrder);
        }

        return true;
    }

    return false;
}

template<class SourceType>
void SourceHelper<SourceType>::EnsureFilterNotExists(const char* filterName)
{
    auto flt = sourceItem_->GetFilter(filterName);
    if (flt)
        sourceItem_->RemoveFilter(filterName);
}

template<class SourceType>
template<class T>
T SourceHelper<SourceType>::GetVal(const char* name, const T& def)
{
    return GetPropertyValue<T>(sourceItem_->GetPropertyValues(), name, def);
}

template<class SourceType>
template<class T>
void SourceHelper<SourceType>::SetVal(const char* name, const T& val)
{
    SetPropertyValue<T>(sourceItem_->GetPropertyValues(), name, val);
}

template<class SourceType>
template<class T>
T SourceHelper<SourceType>::GetVal2(const char* name1, const char* name2, const T& def)
{
    PropertyValue pv = GetPropertyValue<PropertyValue>(sourceItem_->GetPropertyValues(), name1);
    return GetPropertyValue<T>(pv, name2, def);
}

template<class SourceType>
template<class T>
void SourceHelper<SourceType>::SetVal2(const char* name1, const char* name2, const T& val)
{
    PropertyValue pv = GetPropertyValue<PropertyValue>(sourceItem_->GetPropertyValues(), name1);
    SetPropertyValue(pv, name2, val);
    SetPropertyValue(sourceItem_->GetPropertyValues(), name1, std::move(pv));
}

template<class SourceType>
template<class T>
std::pair<bool, T> SourceHelper<SourceType>::GetFilterVal(const char* filter, const char* name, const T& def)
{
    auto flt = sourceItem_->GetFilter(filter);
    if (!flt)
        return std::make_pair(false, def);
    return std::make_pair(true, GetPropertyValue<T>(flt->GetPropertyValues(), name, def));
}

template<class SourceType>
template<class T>
bool SourceHelper<SourceType>::SetFilterVal(const char* filter, const char* name, const T& val)
{
    auto flt = sourceItem_->GetFilter(filter);
    if (!flt)
        return false;
    SetPropertyValue(flt->GetPropertyValues(), name, val);
    return true;
}


/////////////////////////////////////////////////////////////////////
// SceneItemHelper
/////////////////////////////////////////////////////////////////////

SceneItemHelper::SceneItemHelper(obs_proxy::SceneItem* sceneItem)
    : SourceHelper(sceneItem)
{
}

SceneItemHelper::~SceneItemHelper()
{
}

bool SceneItemHelper::Name(std::wstring name)
{
    return GetItem()->Rename(base::WideToUTF8(name));
}

void SceneItemHelper::RotateLeft()
{
    GetItem()->DoLeftRotate();
}

void SceneItemHelper::RotateRight()
{
    GetItem()->DoRightRotate();
}

void SceneItemHelper::SourceItemRotateLeft() {
    GetItem()->DoSourceItemRotateLeft();
}

void SceneItemHelper::SourceItemRotateRight() {
    GetItem()->DoSourceItemRotateRight();
}

float SceneItemHelper::SourcesItemRotateVal() {
    return GetItem()->GetSourcesItemRotateVal();
}

void SceneItemHelper::VerticalFlip()
{
    GetItem()->DoVerticalFlip();
}

void SceneItemHelper::HorizontalFlip()
{
    GetItem()->DoHorizontalFlip();
}

void SceneItemHelper::SetFitToScreen(bool fit_to_screen)
{
    GetItem()->SetFitToScreen(fit_to_screen);
}

obs_proxy::TransformState SceneItemHelper::GetTransformState()
{
    return GetItem()->GetTransformState();
}

void SceneItemHelper::SetTransformState(obs_proxy::TransformState state)
{
    GetItem()->SetTransformState(state);
}


/////////////////////////////////////////////////////////////////////
// ImageSceneItemHelper
/////////////////////////////////////////////////////////////////////

int ColorItemHelper::width()
{
    return GetVal<int>("width", 400);
}

void ColorItemHelper::width(int val)
{
    SetVal<int>("width", val);
}

int ColorItemHelper::height()
{
    return GetVal<int>("height", 400);
}

void ColorItemHelper::height(int val)
{
    SetVal<int>("height", val);
}

SkColor ColorItemHelper::color()
{
    return GetVal<SkColor>("color", SkColorSetARGB(0xff, 0xff, 0xff, 0xff));
}

void ColorItemHelper::color(SkColor val)
{
    SetVal<SkColor>("color", val);
}


int ColorItemHelper::Transparent()
{
    return GetFilterVal<int>("color_filter", "opacity", 100).second;
}

void ColorItemHelper::Transparent(int val)
{
    if (val == 100)
        EnsureFilterNotExists("color_filter");
    else
    {
        EnsureFilterExists(obs_proxy::FilterType::Color, "color_filter");
        SetFilterVal("color_filter", "opacity", val);
    }
}
/////////////////////////////////////////////////////////////////////
// ImageSceneItemHelper
/////////////////////////////////////////////////////////////////////

std::wstring ImageSceneItemHelper::FilePath()
{
    return GetVal<std::wstring>("file");
}

void ImageSceneItemHelper::FilePath(std::wstring path)
{
    SetVal("file", path);
}

int ImageSceneItemHelper::Transparent()
{
    return GetFilterVal<int>("color_filter", "opacity", 100).second;
}

void ImageSceneItemHelper::Transparent(int val)
{
    if (val == 100)
        EnsureFilterNotExists("color_filter");
    else
    {
        EnsureFilterExists(obs_proxy::FilterType::Color, "color_filter");
        SetFilterVal("color_filter", "opacity", val);
    }
}



/////////////////////////////////////////////////////////////////////
// ImageSceneItemHelper
/////////////////////////////////////////////////////////////////////

int SlideShowSceneItemHelper::SlideTime()
{
    return GetVal<int>("slide_time");
}

void SlideShowSceneItemHelper::SlideTime(int val)
{
    SetVal<int>("slide_time", val);
}

int SlideShowSceneItemHelper::TransitionTime()
{
    return GetVal<int>("transition_speed");
}

void SlideShowSceneItemHelper::TransitionTime(int val)
{
    SetVal<int>("transition_speed", val);
}

SlideShowSceneItemHelper::TransitionT SlideShowSceneItemHelper::Transition()
{
    std::string transitionVal = GetVal<std::string>("transition");
    if (transitionVal == "cut")
    {
        return TransitionT::kDefault;
    }
    else if (transitionVal == "swipe")
    {
        return TransitionT::kLeftOut;
    }
    else if (transitionVal == "slide")
    {
        return TransitionT::kRightIn;
    }
    else if (transitionVal == "fade")
    {
        return TransitionT::kFadeOut;
    }
    else
    {
        return TransitionT::kDefault;
    }
}

void SlideShowSceneItemHelper::Transition(SlideShowSceneItemHelper::TransitionT val)
{
    std::string newVal;
    switch (val)
    {
    case TransitionT::kDefault:
    default:
        newVal = "cut";
        break;
    case TransitionT::kLeftOut:
        newVal = "swipe";
        break;
    case TransitionT::kRightIn:
        newVal = "slide";
        break;
    case TransitionT::kFadeOut:
        newVal = "fade";
        break;
    }

    SetVal<std::string>("transition", newVal);
}

bool SlideShowSceneItemHelper::Random()
{
    return GetVal<bool>("randomize");
}

void SlideShowSceneItemHelper::Random(bool val)
{
    SetVal<bool>("randomize", val);
}

bool SlideShowSceneItemHelper::Loop()
{
    return GetVal<bool>("loop");
}

void SlideShowSceneItemHelper::Loop(bool val)
{
    SetVal<bool>("loop", val);
}

std::vector<std::wstring> SlideShowSceneItemHelper::FileList()
{
    obs_proxy::PropertyListValue fileList;
    if (!GetItem()->GetPropertyValues().Get("files", fileList))
        return{};

    int fileCount = fileList.size();

    std::vector<std::wstring> result;
    result.reserve(fileCount);
    for (int i = 0; i < fileCount; ++i)
    {
        obs_proxy::PropertyValue fileNode;
        fileList.Get(i, fileNode);
        result.push_back(GetPropertyValue<std::wstring>(fileNode, "value"));
    }

    return result;
}

void SlideShowSceneItemHelper::FileList(const std::vector<std::wstring>& val)
{
    obs_proxy::PropertyListValue newList;
    for (auto& fileName : val)
    {
        obs_proxy::PropertyValue fileNode;
        SetPropertyValue<std::wstring>(fileNode, "value", fileName);
        newList.Append(std::move(fileNode));
    }

    GetItem()->GetPropertyValues().Set("files", std::move(newList));
}


/////////////////////////////////////////////////////////////////////
// MediaSceneItemHelper
/////////////////////////////////////////////////////////////////////

std::wstring MediaSceneItemHelper::FilePath()
{
    if (GetVal<bool>("is_local_file"))
        return GetVal<std::wstring>("local_file");
    else
        return GetVal<std::wstring>("input");
}

void MediaSceneItemHelper::FilePath(const std::wstring& path)
{
    if (path.find(L"://") == path.npos)
    {
        //InstallMP4MotionFilter(path);
        SetVal("is_local_file", true);
        SetVal("local_file", path);
    }
    else
    {
        SetVal("is_local_file", false);
        SetVal("input", path);
    }
}

void MediaSceneItemHelper::MP4MotionFilePath(const std::wstring& path) {
    if (path.find(L"://") == path.npos)
    {
        InstallMP4MotionFilter(path);
    }

    FilePath(path);
}

bool MediaSceneItemHelper::IsLoop()
{
    return GetVal<bool>("looping", false);
}

void MediaSceneItemHelper::IsLoop(bool val)
{
    SetVal("looping", val);
}

int MediaSceneItemHelper::Volume()
{
    return std::round(GetSceneItemVolume(GetItem()) * 100.0f);
}

void MediaSceneItemHelper::Volume(int val)
{
    SetSceneItemVolume(GetItem(), val / 100.0f);
}

bool MediaSceneItemHelper::IsMuted()
{
    return SceneItemIsMuted(GetItem());
}

void MediaSceneItemHelper::SetMuted(bool muted)
{
    SceneItemSetMuted(GetItem(), muted);
}

int MediaSceneItemHelper::GetMoinitoringType()
{
    return SceneItemGetMoinitoringType(GetItem());
}

void MediaSceneItemHelper::SetMonitoringType(int status)
{
    SceneItemSetMonitoringType(GetItem(), status);
}

bool MediaSceneItemHelper::InstallMP4MotionFilter(const std::wstring& path) {
    livehime::motion::BLPCInfo info;
    std::unique_ptr<livehime::motion::MP4Motion> motion(CreateBililiveMotion(nullptr));
    if (!motion->parseBLPC(path, &info)) {
        EnsureFilterNotExists("mp4_motion_filter");
        return false;
    }

    EnsureFilterExists(FilterType::MP4Motion, "mp4_motion_filter");
    SetFilterVal("mp4_motion_filter", "video_width", info.video_width);
    SetFilterVal("mp4_motion_filter", "video_height", info.video_height);

    SetFilterVal("mp4_motion_filter", "real_frame_x", info.real_frame[0]);
    SetFilterVal("mp4_motion_filter", "real_frame_y", info.real_frame[1]);
    SetFilterVal("mp4_motion_filter", "real_frame_width", info.real_frame[2]);
    SetFilterVal("mp4_motion_filter", "real_frame_height", info.real_frame[3]);

    SetFilterVal("mp4_motion_filter", "alpha_frame_x", info.alpha_frame[0]);
    SetFilterVal("mp4_motion_filter", "alpha_frame_y", info.alpha_frame[1]);
    SetFilterVal("mp4_motion_filter", "alpha_frame_width", info.alpha_frame[2]);
    SetFilterVal("mp4_motion_filter", "alpha_frame_height", info.alpha_frame[3]);

    return true;
}

/////////////////////////////////////////////////////////////////////
// TextSceneItemHelper
/////////////////////////////////////////////////////////////////////

std::wstring TextSceneItemHelper::Text()
{
    return GetVal<std::wstring>("text");
}

void TextSceneItemHelper::Text(const std::wstring& text)
{
    SetVal("text", text);
}

bool TextSceneItemHelper::IsFromFile()
{
    return GetVal<bool>("read_from_file", false);
}

void TextSceneItemHelper::IsFromFile(bool val)
{
    SetVal("read_from_file", val);
}

std::wstring TextSceneItemHelper::TextFile()
{
    return GetVal<std::wstring>("file");
}

void TextSceneItemHelper::TextFile(std::wstring path)
{
    SetVal("file", path);
}

std::wstring TextSceneItemHelper::FontName()
{
    return GetVal2<std::wstring>("font", "face");
}

void TextSceneItemHelper::FontName(std::wstring font)
{
    SetVal2("font", "face", font);
}

int TextSceneItemHelper::FontSize()
{
    return GetVal2<int>("font", "size");
}

void TextSceneItemHelper::FontSize(int size)
{
    SetVal2("font", "size", size);
}

bool TextSceneItemHelper::IsBold()
{
    return (GetVal2<int>("font", "flags") & (int)FontProperty::FontStyle::BOLD) != 0;
}

void TextSceneItemHelper::IsBold(bool val)
{
    if (val)
        SetVal2("font", "flags", GetVal2<int>("font", "flags") | (int)FontProperty::FontStyle::BOLD);
    else
        SetVal2("font", "flags", GetVal2<int>("font", "flags") & ~(int)FontProperty::FontStyle::BOLD);
}

bool TextSceneItemHelper::IsItalic()
{
    return (GetVal2<int>("font", "flags") & (int)FontProperty::FontStyle::ITALIC) != 0;
}

void TextSceneItemHelper::IsItalic(bool val)
{
    if (val)
        SetVal2("font", "flags", GetVal2<int>("font", "flags") | (int)FontProperty::FontStyle::ITALIC);
    else
        SetVal2("font", "flags", GetVal2<int>("font", "flags") & ~(int)FontProperty::FontStyle::ITALIC);
}

SkColor TextSceneItemHelper::TextColor()
{
    return GetVal<SkColor>("color");
}

void TextSceneItemHelper::TextColor(SkColor clr)
{
    SetVal("color", clr);
}

int TextSceneItemHelper::Transparent()
{
    return GetVal<int>("opacity");
}

void TextSceneItemHelper::Transparent(int val)
{
    SetVal<int>("opacity", val);
    SetVal<int>("outline_opacity", val);
}

int TextSceneItemHelper::HorizontalScrollSpeed()
{
    return GetFilterVal<int>("scroll_filter", "speed_x", 0).second;
}

void TextSceneItemHelper::HorizontalScrollSpeed(int val)
{
    if (val == 0 && VerticalScrollSpeed() == 0)
        EnsureFilterNotExists("scroll_filter");
    else
    {
        EnsureFilterExists(obs_proxy::FilterType::Scroll, "scroll_filter");
        SetFilterVal<int>("scroll_filter", "speed_x", val);
    }
}

int TextSceneItemHelper::VerticalScrollSpeed()
{
    return GetFilterVal<int>("scroll_filter", "speed_y", 0).second;
}

void TextSceneItemHelper::VerticalScrollSpeed(int val)
{
    if (val == 0 && HorizontalScrollSpeed() == 0)
        EnsureFilterNotExists("scroll_filter");
    else
    {
        EnsureFilterExists(obs_proxy::FilterType::Scroll, "scroll_filter");
        SetFilterVal<int>("scroll_filter", "speed_y", val);
    }
}

bool TextSceneItemHelper::Outline()
{
    return GetVal<bool>("outline");
}

void TextSceneItemHelper::Outline(bool val)
{
    SetVal<bool>("outline", val);
}

int TextSceneItemHelper::OutlineSize()
{
    return GetVal<int>("outline_size");
}

void TextSceneItemHelper::OutlineSize(int val)
{
    SetVal<int>("outline_size", val);
}

SkColor TextSceneItemHelper::OutlineColor()
{
    return GetVal<SkColor>("outline_color");
}

void TextSceneItemHelper::OutlineColor(SkColor val)
{
    SetVal<SkColor>("outline_color", val);
}


/////////////////////////////////////////////////////////////////////
// MonitorSceneItemHelper
/////////////////////////////////////////////////////////////////////

PropertyList<int64> MonitorSceneItemHelper::MonitorList()
{
    return GetPropertyInt64List(GetItem()->GetProperties(obs_proxy::PropertiesRefreshMode::Refresh), "monitor");
}

int64 MonitorSceneItemHelper::SelectedMonitor()
{
    return GetVal<int64>("monitor");
}

void MonitorSceneItemHelper::SelectedMonitor(int64 val)
{
    SetVal("monitor", val);
}

bool MonitorSceneItemHelper::IsCaptureCursor()
{
    return GetVal<bool>("capture_cursor", false);
}

void MonitorSceneItemHelper::IsCaptureCursor(bool val)
{
    SetVal("capture_cursor", val);
}

bool MonitorSceneItemHelper::HasSliMode()
{
    for (auto& x : GetItem()->GetProperties(obs_proxy::PropertiesRefreshMode::Refresh))
        if (x.name().compare("compatibility") == 0)
            return true;
    return false;
}

bool MonitorSceneItemHelper::IsSliMode()
{
    return GetVal<bool>("compatibility", false);
}

void MonitorSceneItemHelper::IsSliMode(bool val)
{
    SetVal("compatibility", val);
}

/////////////////////////////////////////////////////////////////////
// WindowSceneItemHelper
/////////////////////////////////////////////////////////////////////

PropertyList<std::string> WindowSceneItemHelper::WindowList()
{
    auto r =  GetPropertyStringList(
        GetItem()->GetProperties(obs_proxy::PropertiesRefreshMode::Refresh), "window");

    // 过滤 Windows Internal 进程
    for (auto it = r.begin(); it != r.end();) {
        if (StartsWith(std::get<0>(*it), windows_internal_prefix, false)) {
            it = r.erase(it);
        } else {
            ++it;
        }
    }

    return r;
}

PropertyList<std::string> WindowSceneItemHelper::ModelList()
{
    auto ret = GetPropertyStringList(
        GetItem()->GetProperties(obs_proxy::PropertiesRefreshMode::NoRefresh), "method");
    return ret;
}


std::string WindowSceneItemHelper::SelectedWindow()
{
    return GetVal<std::string>("window");
}

void WindowSceneItemHelper::SelectedWindow(std::string val)
{
    SetVal("window", val);
}

bool WindowSceneItemHelper::IsCaptureCursor()
{
    return GetVal<bool>("cursor");
}

void WindowSceneItemHelper::IsCaptureCursor(bool val)
{
    SetVal("cursor", val);
}

bool WindowSceneItemHelper::IsPlayCenterGmae()
{
    return GetVal<bool>("play_center");
}

void WindowSceneItemHelper::IsPlayCenterGmae(bool val)
{
    SetVal("play_center", val);
}

bool WindowSceneItemHelper::IsCompatibleMode()
{
    return GetVal<bool>("capture_from_whole_screen", false);
}

void WindowSceneItemHelper::IsCompatibleMode(bool val)
{
    SetVal("capture_from_whole_screen", val);
}

bool WindowSceneItemHelper::IsSliMode()
{
    return GetVal<bool>("compatibility", false);
}

void WindowSceneItemHelper::IsSliMode(bool val)
{
    SetVal("compatibility", val);
}

void WindowSceneItemHelper::SetCaptureModel(int model)
{
    SetVal<int>("method", model);
}

int WindowSceneItemHelper::GetCaptureModel()
{
    return GetVal<int>("method");
}

void WindowSceneItemHelper::SetUseHwndSpesc(bool val)
{
    SetVal<bool>("use_hwnd_spesc", val);
}

bool WindowSceneItemHelper::GetUseHwndSpesc()
{
    return GetVal<bool>("use_hwnd_spesc");
}

void WindowSceneItemHelper::SetHwndSpesc(long long hwnd)
{
    SetVal<long long>("hwnd_spesc", hwnd);
}

long long WindowSceneItemHelper::GetHwndSpesc()
{
    return GetVal<long long>("hwnd_spesc");
}

/////////////////////////////////////////////////////////////////////
// GameSceneItemHelper
/////////////////////////////////////////////////////////////////////

bool GameSceneItemHelper::IsAntiCheat()
{
    return GetVal<bool>("anti_cheat_hook");
}

void GameSceneItemHelper::IsAntiCheat(bool val)
{
    SetVal("anti_cheat_hook", val);
}

PropertyList<std::string> GameSceneItemHelper::WindowList()
{
    auto r = GetPropertyStringList(GetItem()->GetProperties(obs_proxy::PropertiesRefreshMode::Refresh), "window");
    if (!r.empty() && std::get<0>(r.front()).empty())
        r.erase(r.begin());

    // 过滤 Windows Internal 进程
    for (auto it = r.begin(); it != r.end();) {
        if (StartsWith(std::get<0>(*it), windows_internal_prefix, false)) {
            it = r.erase(it);
        } else {
            ++it;
        }
    }

    return r;
}

std::string GameSceneItemHelper::SelectedWindow()
{
    return GetVal<std::string>("window");
}

void GameSceneItemHelper::SelectedWindow(std::string val)
{
    SetVal("capture_any_fullscreen", false);
    SetVal("window", val);
}

bool GameSceneItemHelper::IsSliCompatible()
{
    return GetVal<bool>("sli_compatibility");
}

void GameSceneItemHelper::IsSliCompatible(bool val)
{
    SetVal("sli_compatibility", val);
}

bool GameSceneItemHelper::IsAdaptedGame() {
    return GetVal<bool>("adapted_game", false);
}

void GameSceneItemHelper::SetAdaptedGame(bool adapted) {
    SetVal("adapted_game", adapted);
}

bool GameSceneItemHelper::IsTransparent()
{
    return GetVal<bool>("allow_transparency", false);
}

void GameSceneItemHelper::SetTransparent(bool transparent)
{
    SetVal("allow_transparency", transparent);
}


/////////////////////////////////////////////////////////////////////
// CameraSceneItemHelper
/////////////////////////////////////////////////////////////////////

int CameraSceneItemHelper::Volume()
{
    return std::round(GetSceneItemVolume(GetItem()) * 100.0f);
}

void CameraSceneItemHelper::Volume(int val)
{
    SetSceneItemVolume(GetItem(), val / 100.0f);
}

bool CameraSceneItemHelper::IsMuted()
{
    return SceneItemIsMuted(GetItem());
}

void CameraSceneItemHelper::SetMuted(bool muted)
{
    SceneItemSetMuted(GetItem(), muted);
}

int CameraSceneItemHelper::GetMoinitoringType()
{
    return SceneItemGetMoinitoringType(GetItem());
}

void CameraSceneItemHelper::SetMonitoringType(int status)
{
    SceneItemSetMonitoringType(GetItem(),status);
}

PropertyList<std::string> CameraSceneItemHelper::CameraList()
{
    return GetPropertyStringList(GetItem()->GetProperties(obs_proxy::PropertiesRefreshMode::Refresh), "video_device_id");
}

std::string CameraSceneItemHelper::SelectedCamera()
{
    return GetVal<std::string>("video_device_id");
}

void CameraSceneItemHelper::SelectedCamera(std::string val)
{
    SetVal("video_device_id", val);
}

std::string CameraSceneItemHelper::SelectedAudioDevice()
{
    return GetVal<std::string>("audio_device_id");
}

void CameraSceneItemHelper::SelectedAudioDevice(std::string val)
{
    SetVal("audio_device_id", val);
}

bool CameraSceneItemHelper::IsCustomAudioDevice()
{
    return GetVal<bool>("use_custom_audio_device");
}

void CameraSceneItemHelper::SetCustomAudioDevice(bool val)
{
    SetVal("use_custom_audio_device", val);
}

PropertyList<std::string> CameraSceneItemHelper::CustomAudioDeviceList()
{
    return GetPropertyStringList(GetItem()->GetProperties(obs_proxy::PropertiesRefreshMode::Refresh), "audio_device_id");
}

void CameraSceneItemHelper::LaunchConfig()
{
    auto btn = property_cast<ButtonProperty>(*GetItem()->GetProperties(PropertiesRefreshMode::NoRefresh).find("video_config"));
    btn->Click(GetItem());
}

void CameraSceneItemHelper::ShowAudioView(bool show)
{
    SetVal("show_audio_view", show);
}

bool CameraSceneItemHelper::GetShowAudioView()
{
    return GetVal<bool>("show_audio_view");
}

int CameraSceneItemHelper::ColorFilter()
{
    return GetFilterVal<int>("beauty", "color_filter").second;
}

void CameraSceneItemHelper::ColorFilter(int val)
{
    EnsureFilterExists(obs_proxy::FilterType::Beauty, "beauty");
    SetFilterVal("beauty", "color_filter", val);
    CheckFilterExists();
}

void CameraSceneItemHelper::CheckFilterExists()
{
    if (ColorFilter() == 0)
        EnsureFilterNotExists("beauty");
}


PropertyList<std::string> CameraSceneItemHelper::ResolutionList()
{
    auto r = GetPropertyStringList(GetItem()->GetProperties(obs_proxy::PropertiesRefreshMode::Refresh), "resolution");
    r.insert(r.begin(), std::make_tuple(L"", "", true));
    return r;
}

std::string CameraSceneItemHelper::SelectedResolution()
{
    if (GetVal<int>("res_type") == 1)
        return GetVal<std::string>("resolution");
    else
        return "";
}

void CameraSceneItemHelper::SelectedResolution(std::string val)
{
    if (val == "")
    {
        SetVal<int64>("res_type", 0);
    }
    else
    {
        SetVal<int64>("res_type", 1);
        SetVal("resolution", val);
    }
}

PropertyList<int64> CameraSceneItemHelper::AudioOutputList()
{
    return GetPropertyInt64List(GetItem()->GetProperties(obs_proxy::PropertiesRefreshMode::Refresh), "audio_output_mode");
}

int64 CameraSceneItemHelper::SelectedAudioOutput()
{
    return GetVal<int64>("audio_output_mode");
}

void CameraSceneItemHelper::SelectedAudioOutput(int64 val)
{
    SetVal("audio_output_mode", val);
}

bool CameraSceneItemHelper::IsUseColorKey()
{
    return GetFilterVal<bool>("chroma_key", "").first;
}

void CameraSceneItemHelper::IsUseColorKey(bool val)
{
    if (val)
        EnsureFilterExists(obs_proxy::FilterType::ChromaKey, "chroma_key");
    else
        EnsureFilterNotExists("chroma_key");
}

CameraSceneItemHelper::ColorKeyColorT CameraSceneItemHelper::ColorKeyColor()
{
    std::string key_color = GetFilterVal<std::string>("chroma_key", "key_color_type", "green").second;
    if (key_color == "green")
        return kColorGreen;
    else if (key_color == "blue")
        return kColorBlue;
    else if (key_color == "magenta")
        return kColorMagenta;
    else
        return kColorGreen;
}

void CameraSceneItemHelper::ColorKeyColor(ColorKeyColorT val)
{
    switch (val)
    {
    case kColorBlue:
        SetFilterVal("chroma_key", "key_color_type", "blue");
        break;
    case kColorMagenta:
        SetFilterVal("chroma_key", "key_color_type", "magenta");
        break;
    case kColorGreen:
    default:
        SetFilterVal("chroma_key", "key_color_type", "green");
        break;
    }
}

void CameraSceneItemHelper::ColckwiseRotate() {
    SourceItemRotateLeft();
}

float CameraSceneItemHelper::GetSourcesItemRotateVal() {
    return SourcesItemRotateVal();
}

//垂直翻转保存
void CameraSceneItemHelper::SetVerticalFlip(bool val)
{
    EnsureFilterExists(FilterType::FlipSetting, "flip");
    SetFilterVal<bool>("flip", "vertical", val);
    if (!val && !GetHorizontalFlip() && GetCameraRotate() == 0 &&
        !GetCameraYScale() && !GetCameraXScale())
    {
        EnsureFilterNotExists("flip");
    }
}
bool CameraSceneItemHelper::GetVerticalFlip()
{
    return GetFilterVal<bool>("flip", "vertical").second;
}

//水平翻转保存
void CameraSceneItemHelper::SetHorizontalFlip(bool val)
{
    EnsureFilterExists(FilterType::FlipSetting, "flip");
    SetFilterVal<bool>("flip", "horizontal", val);
    if (!val && !GetVerticalFlip() && GetCameraRotate() == 0 &&
        !GetCameraYScale() && !GetCameraXScale())
    {
        EnsureFilterNotExists("flip");
    }
}
bool CameraSceneItemHelper::GetHorizontalFlip()
{
    return GetFilterVal<bool>("flip", "horizontal").second;
}


void CameraSceneItemHelper::SetCameraRotate(int val)
{
    EnsureFilterExists(FilterType::FlipSetting, "flip");
    SetFilterVal<int>("flip", "camera_rotate", val);
    if (val == 0 && !GetHorizontalFlip() && !GetVerticalFlip() &&
        !GetCameraYScale() && !GetCameraXScale())
    {
        EnsureFilterNotExists("flip");
    }
}

int CameraSceneItemHelper::GetCameraRotate()
{
    return GetFilterVal<int>("flip", "camera_rotate").second;
}

void CameraSceneItemHelper::SetCameraXScale(bool val)
{
    EnsureFilterExists(FilterType::FlipSetting, "flip");
    SetFilterVal<bool>("flip", "camera_scale_x", val);
    if (!val && !GetVerticalFlip() && !GetHorizontalFlip() &&
        !GetCameraYScale() && GetCameraRotate() == 0)
    {
        EnsureFilterNotExists("flip");
    }
}

bool CameraSceneItemHelper::GetCameraXScale()
{
    return GetFilterVal<bool>("flip", "camera_scale_x").second;
}

void CameraSceneItemHelper::SetCameraYScale(bool val)
{
    EnsureFilterExists(FilterType::FlipSetting, "flip");
    SetFilterVal<bool>("flip", "camera_scale_y", val);
    if (!val && !GetVerticalFlip() && !GetHorizontalFlip() &&
        !GetCameraXScale() && GetCameraRotate() == 0)
    {
        EnsureFilterNotExists("flip");
    }
}
bool CameraSceneItemHelper::GetCameraYScale()
{
    return GetFilterVal<bool>("flip", "camera_scale_y").second;
}

int CameraSceneItemHelper::ColorKeySimilar()
{
    return GetFilterVal<int>("chroma_key", "similarity", 400).second;
}

void CameraSceneItemHelper::ColorKeySimilar(int val)
{
    SetFilterVal<int>("chroma_key", "similarity", val);
}

int CameraSceneItemHelper::ColorKeySmooth()
{
    return GetFilterVal<int>("chroma_key", "smoothness", 80).second;
}

void CameraSceneItemHelper::ColorKeySmooth(int val)
{
    SetFilterVal("chroma_key", "smoothness", val);
}

int CameraSceneItemHelper::ColorKeySpill()
{
    return GetFilterVal<int>("chroma_key", "spill", 100).second;
}

void CameraSceneItemHelper::ColorKeySpill(int val)
{
    SetFilterVal("chroma_key", "spill", val);
}

int CameraSceneItemHelper::ColorKeyTransparent()
{
    return GetFilterVal<int>("chroma_key", "opacity", 100).second;
}

void CameraSceneItemHelper::ColorKeyTransparent(int val)
{
    SetFilterVal("chroma_key", "opacity", val);
}

std::vector<std::string> CameraSceneItemHelper::GetFilterOrder()
{
    return{ "chroma_key", "beauty", "color_filter" };
}

//各种源通用滤镜参数设置

BaseFilterItemHelper::BaseFilterItemHelper(obs_proxy::SceneItem* sceneItem) : SceneItemHelper(sceneItem){
    if (sceneItem->type() == obs_proxy::SceneItemType::VideoCaptureDevice) {
        is_video_capture_ = true;
    }
}

bool BaseFilterItemHelper::IsUseColorKey() {
    return GetFilterVal<bool>(kFilterColorKeyID.c_str(), "").first;
}

void BaseFilterItemHelper::IsUseColorKey(bool val) {
    if (val)
        EnsureFilterExists(obs_proxy::FilterType::ChromaKey, kFilterColorKeyID.c_str());
    else
        EnsureFilterNotExists(kFilterColorKeyID.c_str());
}

BaseFilterItemHelper::ColorKeyColorT BaseFilterItemHelper::ColorKeyColor() {
    std::string key_color = GetFilterVal<std::string>(kFilterColorKeyID.c_str(), "key_color_type").second;
    if (key_color == "green")
        return kColorGreen;
    else if (key_color == "blue")
        return kColorBlue;
    else if (key_color == "magenta")
        return kColorMagenta;
    else if (key_color == "custom") {
        SkColor color = GetColorVal();
        return kColorCustom;
    }
    else
        return kColorGreen;
}

void BaseFilterItemHelper::ColorKeyColor(ColorKeyColorT val,SkColor color) {
    switch (val) {
    case kColorBlue:
        SetFilterVal(kFilterColorKeyID.c_str(), "key_color_type", "blue");
        break;
    case kColorMagenta:
        SetFilterVal(kFilterColorKeyID.c_str(), "key_color_type", "magenta");
        break;
    case kColorCustom:
        SetFilterVal(kFilterColorKeyID.c_str(), "key_color_type", "custom");
        SetFilterVal(kFilterColorKeyID.c_str(), "key_color", color);
        break;
    case kColorGreen:
    default:
        SetFilterVal(kFilterColorKeyID.c_str(), "key_color_type", "green");
        break;
    }
}

std::tuple<std::string, SkColor> BaseFilterItemHelper::GetColorKeyVal() {
    std::tuple<std::string, SkColor> val;

    std::string key_color_type = GetFilterVal<std::string>(kFilterColorKeyID.c_str(), "key_color_type").second;
    SkColor color = GetFilterVal<SkColor>(kFilterColorKeyID.c_str(), "key_color").second;

    return std::tuple<std::string, SkColor>(key_color_type, color);
}

void  BaseFilterItemHelper::SetColorKeyVal(const std::tuple<std::string, SkColor>& t) {
    std::tuple<std::string, SkColor> val = t;
    SetFilterVal(kFilterColorKeyID.c_str(), "key_color_type", std::get<0>(val));
    SetFilterVal(kFilterColorKeyID.c_str(), "key_color", std::get<1>(val));
}

int BaseFilterItemHelper::GetColorVal() {
    return GetFilterVal<SkColor>(kFilterColorKeyID.c_str(), "key_color").second;
}

void BaseFilterItemHelper::SetColorVal(SkColor val) {
    SetFilterVal(kFilterColorKeyID.c_str(), "key_color_type", "");
    SetFilterVal(kFilterColorKeyID.c_str(), "key_color", val);
}

int BaseFilterItemHelper::ColorKeySimilar() {
    int default_val = 0;
    if (is_video_capture_) {
        default_val = 400;
    }
    else {
        default_val = 1;
    }
    return GetFilterVal<int>(kFilterColorKeyID.c_str(), "similarity", default_val).second;
}

void BaseFilterItemHelper::ColorKeySimilar(int val) {
    SetFilterVal<int>(kFilterColorKeyID.c_str(), "similarity", val);
}

int BaseFilterItemHelper::ColorKeySmooth() {
    int default_val = 0;
    if (is_video_capture_) {
        default_val = 80;
    }
    else {
        default_val = 50;
    }
    return GetFilterVal<int>(kFilterColorKeyID.c_str(), "smoothness", default_val).second;
}

void BaseFilterItemHelper::ColorKeySmooth(int val) {
    SetFilterVal(kFilterColorKeyID.c_str(), "smoothness", val);
}

int BaseFilterItemHelper::ColorKeySpill() {
    return GetFilterVal<int>(kFilterColorKeyID.c_str(), "spill", 100).second;
}

void BaseFilterItemHelper::ColorKeySpill(int val) {
    SetFilterVal(kFilterColorKeyID.c_str(), "spill", val);
}

int BaseFilterItemHelper::ColorKeyTransparent() {
    return GetFilterVal<int>(kFilterColorKeyID.c_str(), "opacity", 100).second;
}

void BaseFilterItemHelper::ColorKeyTransparent(int val) {
    SetFilterVal(kFilterColorKeyID.c_str(), "opacity", val);
}

std::string CameraSceneItemHelper::to_string()
{
    return "";
}
/////////////////////////////////////////////////////////////////////
// ReceiverItemHelper
/////////////////////////////////////////////////////////////////////
int ReceiverItemHelper::GetRtmpPort()
{
    return GetVal<int64>("rtmp_port");
}

std::string ReceiverItemHelper::GetEncoderInfo()
{
    return GetVal<std::string>("encoder_info");
}

int ReceiverItemHelper::Volume()
{
    return std::round(GetSceneItemVolume(GetItem()) * 100.0f);
}

void ReceiverItemHelper::Volume(int val)
{
    SetSceneItemVolume(GetItem(), val / 100.0f);
}

bool ReceiverItemHelper::IsMuted()
{
    return SceneItemIsMuted(GetItem());
}

void ReceiverItemHelper::SetMuted(bool muted)
{
    SceneItemSetMuted(GetItem(),muted);
}

bool ReceiverItemHelper::IsComputerMic() {
    return GetVal<bool>("is_computer_mic");
}

void ReceiverItemHelper::SetComputerMic(bool muted) {
    SetVal("is_computer_mic", muted);
}

int ReceiverItemHelper::GetMoinitoringType()
{
    return SceneItemGetMoinitoringType(GetItem());
}

void ReceiverItemHelper::SetMonitoringType(int status)
{
    SceneItemSetMonitoringType(GetItem(), status);
}

void ReceiverItemHelper::SetLandscapeModel(bool is_landscape_model)
{
    SetVal("is_landscape_model", is_landscape_model);
}

ReceiverItemHelper::ReceiverType ReceiverItemHelper::GetRecvType() {
    return (ReceiverType)GetVal<int>(kRecvType);
}

void ReceiverItemHelper::SetRecvType(ReceiverType type) {
    SetVal(kRecvType, (int)type);
}

/////////////////////////////////////////////////////////////////////
// AudioDeviceItemHelper
/////////////////////////////////////////////////////////////////////
int AudioDeviceItemHelper::Volume()
{
    return std::round(GetSceneItemVolume(GetItem()) * 100.0f);
}

void AudioDeviceItemHelper::Volume(int val)
{
    SetSceneItemVolume(GetItem(), val / 100.0f);
}

bool AudioDeviceItemHelper::IsMuted()
{
    return SceneItemIsMuted(GetItem());
}

void AudioDeviceItemHelper::SetMuted(bool muted)
{
    SceneItemSetMuted(GetItem(), muted);
}

std::string AudioDeviceItemHelper::SelectedAudioDevice()
{
    return GetVal<std::string>("device_id");
}

void AudioDeviceItemHelper::SelectedAudioDevice(std::string val)
{
    SetVal("device_id", val);
}

bool AudioDeviceItemHelper::IsUseDeviceTiming()
{
    return GetVal<bool>("use_device_timing");
}

void AudioDeviceItemHelper::SetUseDeviceTiming(bool use)
{
    SetVal<bool>("use_device_timing", use);
}

int AudioDeviceItemHelper::GetMoinitoringType()
{
    return SceneItemGetMoinitoringType(GetItem());
}

void AudioDeviceItemHelper::SetMonitoringType(int status)
{
    SceneItemSetMonitoringType(GetItem(), status);
}

/////////////////////////////////////////////////////////////////////
// BrowserItemHelper
/////////////////////////////////////////////////////////////////////
int BrowserItemHelper::width()
{
    return GetVal<int>("width", 800);
}

void BrowserItemHelper::width(int val)
{
    SetVal<int>("width", val);
}

int BrowserItemHelper::height()
{
    return GetVal<int>("height", 600);
}

void BrowserItemHelper::height(int val)
{
    SetVal<int>("height", val);
}

void BrowserItemHelper::SetUrl(const std::string& url)
{
    SetVal<std::string>("url", url);
}

std::string BrowserItemHelper::GetUrl()
{
    return GetVal<std::string>("url");
}

void BrowserItemHelper::SetPluginSetUrl(const std::string& url)
{
    SetVal<std::string>("set_url", url);
}

std::string BrowserItemHelper::GetPluginSetUrl()
{
    return GetVal<std::string>("set_url");
}

void BrowserItemHelper::SetIsPlugin(bool plugin)
{
    SetVal<bool>("is_plugin", plugin);
}

bool BrowserItemHelper::GetIsPlugin()
{
    return GetVal<bool>("is_plugin");
}

void BrowserItemHelper::SetPluginId(int64_t id)
{
    SetVal<int64_t>("plugin_id", id);
}

int64_t BrowserItemHelper::GetPluginId()
{
    return GetVal<int64_t>("plugin_id");
}

void BrowserItemHelper::SetCSS(const std::string& css)
{
    SetVal<std::string>("css", css);
}

std::string BrowserItemHelper::GetCSS()
{
    return GetVal<std::string>("css");
}

void BrowserItemHelper::SetRestartWhenActive(bool use)
{
    SetVal<bool>("restart_when_active", use);
}

bool BrowserItemHelper::GetRestartWhenActive()
{
    return GetVal<bool>("restart_when_active");
}

void BrowserItemHelper::SetActivityType(int type)
{
    SetVal<int>("activity_type", type);
}

int BrowserItemHelper::activity_type()
{
    return GetVal<int>("activity_type", 0);
}

void BrowserItemHelper::SetActivityId(const std::string& id)
{
    SetVal<std::string>("activity_id", id);
}

std::string BrowserItemHelper::activity_id()
{
    return GetVal<std::string>("activity_id");
}

void BrowserItemHelper::Refresh()
{
    auto btn = property_cast<ButtonProperty>(*GetItem()->GetProperties(PropertiesRefreshMode::NoRefresh).find("refreshnocache"));
    btn->Click(GetItem());
}

void BrowserItemHelper::shutdown()
{
    SetVal<bool>("shutdown", true);
}

int BrowserItemHelper::Volume()
{
    return std::round(GetSceneItemVolume(GetItem()) * 100.0f);
}

void BrowserItemHelper::Volume(int val)
{
    SetSceneItemVolume(GetItem(), val / 100.0f);
}

bool BrowserItemHelper::IsMuted()
{
    return SceneItemIsMuted(GetItem());
}

void BrowserItemHelper::SetMuted(bool muted)
{
    SceneItemSetMuted(GetItem(), muted);
}

int BrowserItemHelper::GetMoinitoringType()
{
    return SceneItemGetMoinitoringType(GetItem());
}

void BrowserItemHelper::SetMonitoringType(int status)
{
    SceneItemSetMonitoringType(GetItem(), status);
}
/////////////////////////////////////////////////////////////////////
// MicphoneDeviceHelper
/////////////////////////////////////////////////////////////////////
MicphoneDeviceHelper::~MicphoneDeviceHelper()
{
    EndPreview();
}

std::vector<std::string> MicphoneDeviceHelper::GetFilterOrder()
{
    return{ "gain_filter", "reverb_filter","sing_identify_filter"};
}

void MicphoneDeviceHelper::StartPreview()
{
    GetItem()->StartPreview(true);
}

void MicphoneDeviceHelper::EndPreview()
{
    GetItem()->StopPreview();
}

bool MicphoneDeviceHelper::IsReverbEnabled()
{
    EnsureFilterExists(obs_proxy::FilterType::Reverb, "reverb_filter");
    return GetFilterVal<bool>("reverb_filter", "enabled").second;
}

void MicphoneDeviceHelper::IsReverbEnabled(bool val)
{
    EnsureFilterExists(obs_proxy::FilterType::Reverb, "reverb_filter");

    if (val)
        SetFilterVal<bool>("reverb_filter", "enabled", true);
    else
        SetFilterVal<bool>("reverb_filter", "enabled", false);
}

void MicphoneDeviceHelper::Rb_SetDefault()
{
    obs_proxy::Filter* rb_filter = GetItem()->GetFilter("reverb_filter");
    if (rb_filter)
    {
        auto& prop = rb_filter->GetPropertyValues();
        prop.RevertToDefault("room-size");
        prop.RevertToDefault("damping");
        prop.RevertToDefault("wet-level");
        prop.RevertToDefault("dry-level");
        prop.RevertToDefault("width");
        prop.RevertToDefault("freeze-mode");
        rb_filter->UpdatePropertyValues();
    }
}

double MicphoneDeviceHelper::Rb_RoomSize()
{
    return GetFilterVal<double>("reverb_filter", "room-size").second;
}

void MicphoneDeviceHelper::Rb_RoomSize(double val)
{
    SetFilterVal("reverb_filter", "room-size", val);
}

double MicphoneDeviceHelper::Rb_Damping()
{
    return GetFilterVal<double>("reverb_filter", "damping").second;
}

void MicphoneDeviceHelper::Rb_Damping(double val)
{
    SetFilterVal("reverb_filter", "damping", val);
}

double MicphoneDeviceHelper::Rb_WetLevel()
{
    return GetFilterVal<double>("reverb_filter", "wet-level").second;
}

void MicphoneDeviceHelper::Rb_WetLevel(double val)
{
    SetFilterVal("reverb_filter", "wet-level", val);
}

double MicphoneDeviceHelper::Rb_DryLevel()
{
    return GetFilterVal<double>("reverb_filter", "dry-level").second;
}

void MicphoneDeviceHelper::Rb_DryLevel(double val)
{
    SetFilterVal("reverb_filter", "dry-level", val);
}

double MicphoneDeviceHelper::Rb_Width()
{
    return GetFilterVal<double>("reverb_filter", "width").second;
}

void MicphoneDeviceHelper::Rb_Width(double val)
{
    SetFilterVal("reverb_filter", "width", val);
}

double MicphoneDeviceHelper::Rb_FreezeMode()
{
    return GetFilterVal<double>("reverb_filter", "freeze-mode").second;
}

void MicphoneDeviceHelper::Rb_FreezeMode(double val)
{
    SetFilterVal("reverb_filter", "freeze-mode", val);
}

void MicphoneDeviceHelper::SingIdentifyEnable(bool enable)
{
    if (enable) {
        EnsureFilterExists(obs_proxy::FilterType::SingIdentify, "sing_identify_filter");
    }
    else {
        obs_proxy::Filter* sd_filter = GetItem()->GetFilter("sing_identify_filter");
        if (sd_filter) {
            SetFilterVal<bool>("sing_identify_filter", "enabled", false);
            sd_filter->UpdatePropertyValues();
        }
        EnsureFilterNotExists("sing_identify_filter");
    }
}

void MicphoneDeviceHelper::SingIdentifyUpdate(const char* appkey,const char* token)
{
    obs_proxy::Filter* sd_filter = GetItem()->GetFilter("sing_identify_filter");
    if (sd_filter) {
        SetFilterVal("sing_identify_filter", "appkey", appkey);
        SetFilterVal("sing_identify_filter", "token", token);
        SetFilterVal<bool>("sing_identify_filter", "enabled", true);
        sd_filter->UpdatePropertyValues();
    }
}

/////////////////////////////////////////////////////////////////////
// TextureRendererItemHelper
/////////////////////////////////////////////////////////////////////

void TextureRendererItemHelper::SetType(TextureType type) {
    SetVal("texture_type", uint32_t(type));
}

TextureRendererItemHelper::TextureType TextureRendererItemHelper::GetType() {
    return TextureType(GetVal<uint32_t>("texture_type", 0));
}

void TextureRendererItemHelper::SetFrame(const SkBitmap& bmp) {
    auto source = static_cast<SceneItemImpl*>(sourceItem_)->AsSource();
    if (!source) {
        return;
    }

    auto cfg = bmp.config();
    if (cfg != SkBitmap::kARGB_8888_Config) {
        NOTREACHED();
        return;
    }

    obs_source_frame* result = obs_source_frame_create(
        VIDEO_FORMAT_BGRA, bmp.width(), bmp.height());
    if (!result) {
        return;
    }
    memcpy(result->data[0], bmp.pixelRef()->pixels(), bmp.width() * bmp.height() * 4);

    auto ts = std::chrono::steady_clock::now().time_since_epoch();
    result->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(ts).count() * 100;

    obs_source_output_video(source, result);
    obs_source_frame_destroy(result);
}

void TextureRendererItemHelper::Clear() {
    auto source = static_cast<SceneItemImpl*>(sourceItem_)->AsSource();
    if (!source) {
        return;
    }

    // 先抹掉先前的数据
    auto width = obs_source_get_base_width(source);
    auto height = obs_source_get_base_height(source);

    obs_source_frame* result = obs_source_frame_create(
        VIDEO_FORMAT_BGRA, width, height);
    if (!result) {
        obs_source_output_video(source, nullptr);
        return;
    }

    std::memset(result->data[0], 0, width*height * 4);

    auto ts = std::chrono::steady_clock::now().time_since_epoch();
    result->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(ts).count() * 100;

    obs_source_output_video(source, result);
    obs_source_frame_destroy(result);

    // 然后 deactive 纹理
    obs_source_output_video(source, nullptr);
}