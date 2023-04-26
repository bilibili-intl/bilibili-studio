#ifndef SRCPROP_UI_UTIL_H_
#define SRCPROP_UI_UTIL_H_

#include <vector>
#include <utility>

#include "base/basictypes.h"

#include "SkBitmap.h"
#include "third_party/skia/include/core/SkColor.h"

#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"
#include "obs/obs_proxy/public/proxy/obs_source_property_values.h"
#include "obs/obs_proxy/public/proxy/obs_audio_devices.h"
#include "obs/obs_proxy/public/proxy/obs_volume_controller.h"
#include "bililive/bililive/livehime/obs/source_creator.h"


int MapFloatToInt(float val, int minVal = 0, int maxVal = 100);
float MapFloatFromInt(int val, int minVal = 0, int maxVal = 100);


//range: [0, 1]
void SetSceneItemVolume(obs_proxy::SceneItem* sceneItem, float volume);
float GetSceneItemVolume(obs_proxy::SceneItem* sceneItem);

void RegisterSceneItemDeferredUpdateCallback(obs_proxy::SceneItem* sceneItem, std::function<void()>&& procedure);

template<class T>
using PropertyList = std::vector < std::tuple<std::wstring, T, bool> >;

PropertyList<std::string> GetPropertyStringList(const obs_proxy::Properties& prop, const std::string& name);
PropertyList<int64> GetPropertyInt64List(const obs_proxy::Properties& prop, const std::string& name);

template<class SourceType>
class SourceHelper {
public:
    explicit SourceHelper(SourceType* source);
    virtual ~SourceHelper();

    void Update();
    void UpdateFilters();

    std::wstring Name();

    SourceType* GetItem();

    bool EnsureFilterExists(obs_proxy::FilterType filterType, const char* filterName);
    void EnsureFilterNotExists(const char* filterName);

    virtual std::vector<std::string> GetFilterOrder() { return{}; }

protected:
    template<class T> T GetVal(const char* name, const T& def = T{});
    template<class T> void SetVal(const char* name, const T& val);
    template<class T> T GetVal2(const char* name1, const char* name2, const T& def = T{});
    template<class T> void SetVal2(const char* name1, const char* name2, const T& val);
    template<class T> std::pair<bool, T> GetFilterVal(const char* filter, const char* name, const T& def = T{});
    template<class T> bool SetFilterVal(const char* filter, const char* name, const T& val);

    SourceType* sourceItem_;
};

class SceneItemHelper : public SourceHelper<obs_proxy::SceneItem> {
public:
    typedef SourceHelper<obs_proxy::SceneItem> SuperClass;

    explicit SceneItemHelper(obs_proxy::SceneItem* sceneItem);
    ~SceneItemHelper();

    inline std::wstring Name() { return SuperClass::Name(); }
    bool Name(std::wstring name);

    void RotateLeft();
    void RotateRight();
    void SourceItemRotateLeft();
    void SourceItemRotateRight();
    float SourcesItemRotateVal();
    void VerticalFlip();
    void HorizontalFlip();
    void SetFitToScreen(bool fit_to_screen);

    obs_proxy::TransformState GetTransformState();
    void SetTransformState(obs_proxy::TransformState state);

    virtual std::string to_string() {
        return"";
    }
};

class ImageSceneItemHelper : SceneItemHelper {
public:
    explicit ImageSceneItemHelper(obs_proxy::SceneItem* sceneItem) : SceneItemHelper(sceneItem)
    {}

    using SceneItemHelper::Update;
    using SceneItemHelper::UpdateFilters;
    using SceneItemHelper::Name;
    using SceneItemHelper::RotateLeft;
    using SceneItemHelper::RotateRight;
    using SceneItemHelper::VerticalFlip;
    using SceneItemHelper::HorizontalFlip;
    using SceneItemHelper::GetTransformState;
    using SceneItemHelper::SetTransformState;

    std::wstring FilePath();
    void FilePath(std::wstring path);

    int Transparent();
    void Transparent(int val);
};

class ColorItemHelper : SceneItemHelper {
public:
    explicit ColorItemHelper(obs_proxy::SceneItem* sceneItem) : SceneItemHelper(sceneItem)
    {}

    using SceneItemHelper::Update;
    using SceneItemHelper::Name;

    int width();
    void width(int val);

    int height();
    void height(int val);

    SkColor color();
    void color(SkColor val);

    int Transparent();
    void Transparent(int val);
};

class SlideShowSceneItemHelper : SceneItemHelper {
public:
    explicit SlideShowSceneItemHelper(obs_proxy::SceneItem* sceneItem) : SceneItemHelper(sceneItem) {}

    enum TransitionT { kDefault = 0, kLeftOut, kRightIn, kFadeOut };

    using SceneItemHelper::Update;
    using SceneItemHelper::Name;

    int SlideTime();
    void SlideTime(int val);

    int TransitionTime();
    void TransitionTime(int val);

    TransitionT Transition();
    void Transition(TransitionT val);

    bool Random();
    void Random(bool val);

    bool Loop();
    void Loop(bool val);

    std::vector<std::wstring> FileList();
    void FileList(const std::vector<std::wstring>& val);
};

class MediaSceneItemHelper : SceneItemHelper {
public:
    explicit MediaSceneItemHelper(obs_proxy::SceneItem* sceneItem) : SceneItemHelper(sceneItem) {}

    using SceneItemHelper::Update;
    using SceneItemHelper::Name;
    using SceneItemHelper::GetItem;

    std::wstring FilePath();
    void FilePath(const std::wstring& path);
    void MP4MotionFilePath(const std::wstring& path);

    int Volume();
    void Volume(int val);

    bool IsLoop();
    void IsLoop(bool val);

    bool IsMuted();
    void SetMuted(bool muted);

    int GetMoinitoringType();
    void SetMonitoringType(int status);

private:
    bool InstallMP4MotionFilter(const std::wstring& path);
};

class TextSceneItemHelper : SceneItemHelper {
public:
    explicit TextSceneItemHelper(obs_proxy::SceneItem* sceneItem) : SceneItemHelper(sceneItem) {}

    using SceneItemHelper::Update;
    using SceneItemHelper::Name;

    std::wstring Text();
    void Text(const std::wstring& text);

    bool IsFromFile();
    void IsFromFile(bool val);

    std::wstring TextFile();
    void TextFile(std::wstring path);

    std::wstring FontName();
    void FontName(std::wstring font);

    int FontSize();
    void FontSize(int size);

    bool IsBold();
    void IsBold(bool val);

    bool IsItalic();
    void IsItalic(bool val);

    SkColor TextColor();
    void TextColor(SkColor clr);

    int Transparent();
    void Transparent(int val);

    int HorizontalScrollSpeed();
    void HorizontalScrollSpeed(int val);

    int VerticalScrollSpeed();
    void VerticalScrollSpeed(int val);

    bool Outline();
    void Outline(bool val);

    int OutlineSize();
    void OutlineSize(int val);

    SkColor OutlineColor();
    void OutlineColor(SkColor val);
};

class MonitorSceneItemHelper : public SceneItemHelper {
public:
    explicit MonitorSceneItemHelper(obs_proxy::SceneItem* sceneItem) : SceneItemHelper(sceneItem) {}

    using SceneItemHelper::Update;
    using SceneItemHelper::Name;

    PropertyList<int64> MonitorList();

    int64 SelectedMonitor();
    void SelectedMonitor(int64);

    bool IsCaptureCursor();
    void IsCaptureCursor(bool val);

    bool HasSliMode();

    bool IsSliMode();
    void IsSliMode(bool val);
};

class WindowSceneItemHelper : SceneItemHelper {
public:
    explicit WindowSceneItemHelper(obs_proxy::SceneItem* sceneItem) : SceneItemHelper(sceneItem) {}

    using SceneItemHelper::Update;
    using SceneItemHelper::Name;
    using SceneItemHelper::SetFitToScreen;

    PropertyList<std::string> WindowList();
    PropertyList<std::string> ModelList();
    std::string SelectedWindow();
    void SelectedWindow(std::string val);

    bool IsCaptureCursor();
    void IsCaptureCursor(bool val);

    bool IsPlayCenterGmae();
    void IsPlayCenterGmae(bool val);

    bool IsCompatibleMode();
    void IsCompatibleMode(bool val);

    bool IsSliMode();
    void IsSliMode(bool val);

    void SetCaptureModel(int model);
    int GetCaptureModel();

    void SetUseHwndSpesc(bool val);
    bool GetUseHwndSpesc();

    void SetHwndSpesc(long long hwnd);
    long long GetHwndSpesc();
};

class GameSceneItemHelper : SceneItemHelper {
public:
    explicit GameSceneItemHelper(obs_proxy::SceneItem* sceneItem) : SceneItemHelper(sceneItem) {}

    using SceneItemHelper::GetItem;
    using SceneItemHelper::Update;
    using SceneItemHelper::Name;
    using SceneItemHelper::SetFitToScreen;

    bool IsAntiCheat();
    void IsAntiCheat(bool val);

    PropertyList<std::string> WindowList();

    std::string SelectedWindow();
    void SelectedWindow(std::string val);

    bool IsSliCompatible();
    void IsSliCompatible(bool val);

    bool IsAdaptedGame();
    void SetAdaptedGame(bool adapted);

    bool IsTransparent();
    void SetTransparent(bool transparent);

};

class CameraSceneItemHelper : SceneItemHelper {
public:
    enum ColorKeyColorT { kColorGreen = 0, kColorBlue, kColorMagenta };

    explicit CameraSceneItemHelper(obs_proxy::SceneItem* sceneItem) : SceneItemHelper(sceneItem)
    {}

    using SceneItemHelper::GetItem;
    using SceneItemHelper::Update;
    using SceneItemHelper::UpdateFilters;
    using SceneItemHelper::Name;
    using SceneItemHelper::VerticalFlip;
    using SceneItemHelper::HorizontalFlip;
    using SceneItemHelper::GetTransformState;
    using SceneItemHelper::SetTransformState;

    std::string to_string() override;

    int Volume();
    void Volume(int val);

    bool IsMuted();
    void SetMuted(bool muted);

    int GetMoinitoringType();
    void SetMonitoringType(int status);

    PropertyList<std::string> CameraList();

    std::string SelectedCamera();
    void SelectedCamera(std::string val);

    std::string SelectedAudioDevice();
    void SelectedAudioDevice(std::string val);

    bool IsCustomAudioDevice();
    void SetCustomAudioDevice(bool val);

    PropertyList<std::string> CustomAudioDeviceList();

    void LaunchConfig();

    int ColorFilter();
    void ColorFilter(int val);

    PropertyList<std::string> ResolutionList();

    std::string SelectedResolution();
    void SelectedResolution(std::string val);

    PropertyList<int64> AudioOutputList();

    int64 SelectedAudioOutput();
    void SelectedAudioOutput(int64 val);

    bool IsUseColorKey();
    void IsUseColorKey(bool val);

    ColorKeyColorT ColorKeyColor();
    void ColorKeyColor(ColorKeyColorT val);

    int ColorKeySimilar();
    void ColorKeySimilar(int val);

    int ColorKeySmooth();
    void ColorKeySmooth(int val);

    int ColorKeySpill();
    void ColorKeySpill(int val);

    int ColorKeyTransparent();
    void ColorKeyTransparent(int val);

    void ShowAudioView(bool show);
    bool GetShowAudioView();  

    void ColckwiseRotate();
    float GetSourcesItemRotateVal();

    //垂直翻转保存
    void SetVerticalFlip(bool val);
    bool GetVerticalFlip();

    //水平翻转保存
    void SetHorizontalFlip(bool val);
    bool GetHorizontalFlip();

    //旋转90度保存
    void SetCameraRotate(int val);
    int GetCameraRotate();

    //scal状态
    void SetCameraXScale(bool val);
    bool GetCameraXScale();
    void SetCameraYScale(bool val);
    bool GetCameraYScale();
protected:
    void CheckFilterExists();
    std::vector<std::string> GetFilterOrder() override;
};
class BaseFilterItemHelper :public SceneItemHelper {
public:
    enum ColorKeyColorT {
        kColorGreen = 0,
        kColorBlue,
        kColorMagenta,
        kColorCustom
    };

    explicit BaseFilterItemHelper(obs_proxy::SceneItem* sceneItem);

    bool IsUseColorKey();
    void IsUseColorKey(bool val);

    ColorKeyColorT ColorKeyColor();
    void ColorKeyColor(ColorKeyColorT val,SkColor color = 0);

    std::tuple<std::string, SkColor> GetColorKeyVal();
    void  SetColorKeyVal(const std::tuple<std::string, SkColor>& t);

    int GetColorVal();
    void SetColorVal(SkColor val);

    int ColorKeySimilar();
    void ColorKeySimilar(int val);

    int ColorKeySmooth();
    void ColorKeySmooth(int val);

    int ColorKeySpill();
    void ColorKeySpill(int val);

    int ColorKeyTransparent();
    void ColorKeyTransparent(int val);

private:
    std::string  kFilterColorKeyID = "chroma_key";
    bool is_video_capture_ = false;
};

class ReceiverItemHelper : SceneItemHelper {
public:
    enum ReceiverType {
        RECV_T_WIRELESS,
        RECV_T_WIRED_IOS,
    };

    static constexpr char kRecvType[] = "recv_type";

    explicit ReceiverItemHelper(obs_proxy::SceneItem* sceneItem) : SceneItemHelper(sceneItem)
    {}

    using SceneItemHelper::Update;
    using SceneItemHelper::Name;
    using SceneItemHelper::GetItem;

    int Volume();
    void Volume(int val);

    bool IsMuted();

    void SetMuted(bool muted);

    bool IsComputerMic();
    void SetComputerMic(bool muted);

    int GetMoinitoringType();
    void SetMonitoringType(int status);

    int GetRtmpPort();

    std::string GetEncoderInfo();

    void SetLandscapeModel(bool is_landscape_model);

    ReceiverType GetRecvType();
    void SetRecvType(ReceiverType type);
};

class AudioDeviceItemHelper : SceneItemHelper {
public:
    explicit AudioDeviceItemHelper(obs_proxy::SceneItem* sceneItem) : SceneItemHelper(sceneItem)
    {}

    using SceneItemHelper::Update;
    using SceneItemHelper::Name;

    int Volume();
    void Volume(int val);

    bool IsMuted();

    void SetMuted(bool muted);

    std::string SelectedAudioDevice();
    void SelectedAudioDevice(std::string val);

    bool IsUseDeviceTiming();
    void SetUseDeviceTiming(bool use);

    int GetMoinitoringType();
    void SetMonitoringType(int status);
};

class BrowserItemHelper : SceneItemHelper {
public:
    enum ActivityType
    {
        Normal,
        MissionActivity,
    };

    explicit BrowserItemHelper(obs_proxy::SceneItem* sceneItem) : SceneItemHelper(sceneItem)
    {
    }

    using SceneItemHelper::Update;
    using SceneItemHelper::Name;

    int width();
    void width(int val);

    int height();
    void height(int val);

    void SetUrl(const std::string& url);
    std::string GetUrl();

    void SetPluginSetUrl(const std::string& url);
    std::string GetPluginSetUrl();

    void SetIsPlugin(bool plugin);
    bool GetIsPlugin();

    void SetPluginId(int64_t id);
    int64_t GetPluginId();

    void SetCSS(const std::string& css);
    std::string GetCSS();

    void SetRestartWhenActive(bool use);
    bool GetRestartWhenActive();

    // 为任务活动添加，标识是哪个任务的源
    void SetActivityType(int type);
    int activity_type();
    void SetActivityId(const std::string& id);
    std::string activity_id();

    void Refresh();

    void shutdown();

    int Volume();
    void Volume(int val);

    bool IsMuted();

    void SetMuted(bool muted);

    int GetMoinitoringType();
    void SetMonitoringType(int status);
};

class MicphoneDeviceHelper : SourceHelper < obs_proxy::AudioDevice >
{
public:
    explicit MicphoneDeviceHelper(obs_proxy::AudioDevice* audioDev)
        : SourceHelper(audioDev) {}
    ~MicphoneDeviceHelper();

    using SourceHelper<obs_proxy::AudioDevice>::GetItem;
    //using SourceHelper<obs_proxy::AudioDevice>::Update;
    using SourceHelper<obs_proxy::AudioDevice>::UpdateFilters;
    using SourceHelper<obs_proxy::AudioDevice>::Name;

    void StartPreview();
    void EndPreview();

    bool IsReverbEnabled();
    void IsReverbEnabled(bool val);

    void Rb_SetDefault();

    double Rb_RoomSize();
    void Rb_RoomSize(double val);

    double Rb_Damping();
    void Rb_Damping(double val);

    double Rb_WetLevel();
    void Rb_WetLevel(double val);

    double Rb_DryLevel();
    void Rb_DryLevel(double val);

    double Rb_Width();
    void Rb_Width(double val);

    double Rb_FreezeMode();
    void Rb_FreezeMode(double val);

    void SingIdentifyEnable(bool enable);

    void SingIdentifyUpdate(const char* appkey, const char* token);

protected:
    std::vector<std::string> GetFilterOrder() override;
};

class TextureRendererItemHelper : SceneItemHelper {
public:
    enum class TextureType : uint32_t {
        Unknown = 0,
        Vote = 1,
    };

    explicit TextureRendererItemHelper(obs_proxy::SceneItem* item)
        : SceneItemHelper(item) {}

    using SceneItemHelper::Update;
    using SceneItemHelper::Name;
    using SceneItemHelper::GetItem;

    void SetType(TextureType type);
    TextureType GetType();

    void SetFrame(const SkBitmap& bmp);
    void Clear();
};

#endif
