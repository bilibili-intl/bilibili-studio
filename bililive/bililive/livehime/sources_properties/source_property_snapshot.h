/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_PROPERTY_SNAPSHOT_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_PROPERTY_SNAPSHOT_H_

#include <functional>
#include <vector>

#include "bilibase/basic_macros.h"

#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"

namespace livehime {

using Action = std::function<void()>;
using RestoreActions = std::vector<Action>;

// Please make sure the lifetime of a snapshot instance must be shorter than the source item
// it references.

template<typename PropertyTraits>
class SourcePropertySnapshot {
public:
    using Traits = PropertyTraits;
    using Item = typename PropertyTraits::Item;

    DEFAULT_COPY(SourcePropertySnapshot);

    static std::unique_ptr<SourcePropertySnapshot> NewTake(Item* item)
    {
        return std::unique_ptr<SourcePropertySnapshot>(
            new SourcePropertySnapshot(item, Traits::TakeSnapshot(item)));
    }

    void Restore()
    {
        for (auto&& action : restore_actions_) {
            action();
        }
    }

private:
    SourcePropertySnapshot(Item* item, RestoreActions actions)
        : ref_item_(item), restore_actions_(std::move(actions))
    {}

private:
    Item* ref_item_;
    RestoreActions restore_actions_;
};

struct CameraPropertyTraits {
    using Item = CameraSceneItemHelper;

    static RestoreActions TakeSnapshot(Item* ref_item)
    {
        RestoreActions actions;

        auto prev_camera = ref_item->SelectedCamera();
        actions.emplace_back([=] {
            ref_item->SelectedCamera(prev_camera);
        });

        auto prev_resolution = ref_item->SelectedResolution();
        actions.emplace_back([=] {
            ref_item->SelectedResolution(prev_resolution);
        });

        auto prev_audio_output = ref_item->SelectedAudioOutput();
        actions.emplace_back([=] {
            ref_item->SelectedAudioOutput(prev_audio_output);
        });

        auto prev_volume = ref_item->Volume();
        actions.emplace_back([=] {
            ref_item->Volume(prev_volume);
        });

        auto prev_mute = ref_item->IsMuted();
        actions.emplace_back([=] {
            ref_item->SetMuted(prev_mute);
        });

        auto prev_monitor = ref_item->GetMoinitoringType();
        actions.emplace_back([=] {
            ref_item->SetMonitoringType(prev_monitor);
        });

        auto prev_transforn_state = ref_item->GetTransformState();
        actions.emplace_back([=] {
            ref_item->SetTransformState(prev_transforn_state);
        });

        auto prev_is_use = ref_item->IsUseColorKey();
        actions.emplace_back([=] {
            ref_item->IsUseColorKey(prev_is_use);
        });

        auto prev_color = ref_item->ColorKeyColor();
        actions.emplace_back([=] {
            ref_item->ColorKeyColor(prev_color);
        });

        auto prev_similar = ref_item->ColorKeySimilar();
        actions.emplace_back([=] {
            ref_item->ColorKeySimilar(prev_similar);
        });

        auto prev_smooth = ref_item->ColorKeySmooth();
        actions.emplace_back([=] {
            ref_item->ColorKeySmooth(prev_smooth);
        });

        auto prev_transparent = ref_item->ColorKeyTransparent();
        actions.emplace_back([=] {
            ref_item->ColorKeyTransparent(prev_transparent);
        });

        auto prev_spill = ref_item->ColorKeySpill();
        actions.emplace_back([=] {
            ref_item->ColorKeySpill(prev_spill);
        });        

        auto vertical_flip = ref_item->GetVerticalFlip();
        actions.emplace_back([=] {
            ref_item->SetVerticalFlip(vertical_flip);
        });

        auto horizontal_filp = ref_item->GetHorizontalFlip();
        actions.emplace_back([=] {
            ref_item->SetHorizontalFlip(horizontal_filp);
        });

        auto pre_audio_device = ref_item->SelectedAudioDevice();
        actions.emplace_back([=] {
            ref_item->SelectedAudioDevice(pre_audio_device);
            });

        auto pre_use_custom_audio_device = ref_item->IsCustomAudioDevice();
        actions.emplace_back([=] {
            ref_item->SetCustomAudioDevice(pre_use_custom_audio_device);
            });


        auto pre_camera_rotate = ref_item->GetCameraRotate();
        actions.emplace_back([=] {
            ref_item->SetCameraRotate(pre_camera_rotate);
            });

        auto pre_camera_x_scale = ref_item->GetCameraXScale();
        actions.emplace_back([=] {
            ref_item->SetCameraXScale(pre_camera_x_scale);
            });

        auto pre_camera_y_scale = ref_item->GetCameraYScale();
        actions.emplace_back([=] {
            ref_item->SetCameraYScale(pre_camera_y_scale);
            });

        actions.emplace_back([=] {
            ref_item->Update();
        });


        return actions;
    }
};

struct BeautyPropertyTraits {
    using Item = CameraSceneItemHelper;

    static RestoreActions TakeSnapshot(Item* ref_item)
    {
        RestoreActions actions;

        actions.emplace_back([=] {
            ref_item->UpdateFilters();
        });

        return actions;
    }
};

struct BeautyShapePropertyTraits {
    using Item = CameraSceneItemHelper;

    static RestoreActions TakeSnapshot(Item* ref_item)
    {
        RestoreActions actions;

        actions.emplace_back([=] {
            ref_item->UpdateFilters();
        });

        return actions;
    }
};

struct WindowCapturePropertyTraits {
    using Item = WindowSceneItemHelper;

    static RestoreActions TakeSnapshot(Item* ref_item)
    {
        RestoreActions actions;

        auto select_window = ref_item->SelectedWindow();
        actions.emplace_back([=] {
            ref_item->SelectedWindow(select_window);
            });

        auto capture_cursor = ref_item->IsCaptureCursor();
        actions.emplace_back([=] {
            ref_item->IsCaptureCursor(capture_cursor);
            });

        auto compatible_mode = ref_item->IsCompatibleMode();
        actions.emplace_back([=] {
            ref_item->IsCompatibleMode(compatible_mode);
            });

        auto sli_mode = ref_item->IsSliMode();
        actions.emplace_back([=] {
            ref_item->IsSliMode(sli_mode);
            });

        auto capture_mode = ref_item->GetCaptureModel();
        actions.emplace_back([=] {
            ref_item->SetCaptureModel(capture_mode);
            });

        return actions;
    }
};

struct ColorKeyPropertyTraits {
    using Item = /*CameraSceneItemHelper*/BaseFilterItemHelper;

    static RestoreActions TakeSnapshot(Item* ref_item)
    {
        RestoreActions actions;

        auto prev_is_use = ref_item->IsUseColorKey();
        actions.emplace_back([=] {
            ref_item->IsUseColorKey(prev_is_use);
        });

        auto prev_color = ref_item->GetColorKeyVal();
        actions.emplace_back([=] {
            ref_item->SetColorKeyVal(prev_color);
        });

        auto prev_similar = ref_item->ColorKeySimilar();
        actions.emplace_back([=] {
            ref_item->ColorKeySimilar(prev_similar);
        });

        auto prev_smooth = ref_item->ColorKeySmooth();
        actions.emplace_back([=] {
            ref_item->ColorKeySmooth(prev_smooth);
        });

        auto prev_transparent = ref_item->ColorKeyTransparent();
        actions.emplace_back([=] {
            ref_item->ColorKeyTransparent(prev_transparent);
        });

        auto prev_spill = ref_item->ColorKeySpill();
        actions.emplace_back([=] {
            ref_item->ColorKeySpill(prev_spill);
        });

        actions.emplace_back([=] {
            ref_item->Update();
        });

        return actions;
    }
};

struct ImagePropertyTraits {
    using Item = ImageSceneItemHelper;

    static RestoreActions TakeSnapshot(Item* ref_item)
    {
        RestoreActions actions;

        auto prev_transparent = ref_item->Transparent();
        actions.emplace_back([=] {
            ref_item->Transparent(prev_transparent);
        });

        auto prev_transforn_state = ref_item->GetTransformState();
        actions.emplace_back([=] {
            ref_item->SetTransformState(prev_transforn_state);
        });

        actions.emplace_back([=] {
            ref_item->UpdateFilters();
        });

        return actions;
    }
};

struct ColorPropertyTraits {
    using Item = ColorItemHelper;
    static RestoreActions TakeSnapshot(Item* ref_item)
    {
        RestoreActions actions;

        auto prev_color = ref_item->color();
        actions.emplace_back([=] {
            ref_item->color(prev_color);
        });

        auto prev_width = ref_item->width();
        actions.emplace_back([=] {
            ref_item->width(prev_width);
        });

        auto prev_height = ref_item->height();
        actions.emplace_back([=] {
            ref_item->height(prev_height);
        });

        auto prev_transparent = ref_item->Transparent();
        actions.emplace_back([=] {
            ref_item->Transparent(prev_transparent);
        });

        actions.emplace_back([=] {
            ref_item->Update();
        });
        return actions;
    }
};


struct MediaPropertyTraits {
    using Item = MediaSceneItemHelper;

    static RestoreActions TakeSnapshot(Item* ref_item)
    {
        RestoreActions actions;

        auto prev_volume = ref_item->Volume();
        actions.emplace_back([=] {
            ref_item->Volume(prev_volume);
        });

        auto prev_mute = ref_item->IsMuted();
        actions.emplace_back([=] {
            ref_item->SetMuted(prev_mute);
        });

        auto prev_monitor = ref_item->GetMoinitoringType();
        actions.emplace_back([=] {
            ref_item->SetMonitoringType(prev_monitor);
        });


        return actions;
    }
};

struct TextPropertyTraits {
    using Item = TextSceneItemHelper;

    static RestoreActions TakeSnapshot(Item* ref_item)
    {
        RestoreActions actions;

        auto prev_font_name = ref_item->FontName();
        actions.emplace_back([=] {
            ref_item->FontName(prev_font_name);
        });

        auto prev_font_size = ref_item->FontSize();
        actions.emplace_back([=] {
            ref_item->FontSize(prev_font_size);
        });

        auto prev_font_style_bold = ref_item->IsBold();
        actions.emplace_back([=] {
            ref_item->IsBold(prev_font_style_bold);
        });

        auto prev_font_style_italic = ref_item->IsItalic();
        actions.emplace_back([=] {
            ref_item->IsItalic(prev_font_style_italic);
        });

        auto prev_font_color = ref_item->TextColor();
        actions.emplace_back([=] {
            ref_item->TextColor(prev_font_color);
        });

        auto prev_outline = ref_item->Outline();
        actions.emplace_back([=] {
            ref_item->Outline(prev_outline);
        });

        auto prev_outline_color = ref_item->OutlineColor();
        actions.emplace_back([=] {
            ref_item->OutlineColor(prev_outline_color);
        });

        auto prev_outline_size = ref_item->OutlineSize();
        actions.emplace_back([=] {
            ref_item->OutlineSize(prev_outline_size);
        });

        auto prev_transparent = ref_item->Transparent();
        actions.emplace_back([=] {
            ref_item->Transparent(prev_transparent);
        });

        auto prev_horizontal_scroll_speed = ref_item->HorizontalScrollSpeed();
        actions.emplace_back([=] {
            ref_item->HorizontalScrollSpeed(prev_horizontal_scroll_speed);
        });

        auto prev_vertical_scroll_speed = ref_item->VerticalScrollSpeed();
        actions.emplace_back([=] {
            ref_item->VerticalScrollSpeed(prev_vertical_scroll_speed);
        });

        actions.emplace_back([=] {
            ref_item->Update();
        });

        return actions;
    }
};

struct AlbumPropertyTraits {
    using Item = SlideShowSceneItemHelper;

    static RestoreActions TakeSnapshot(Item* ref_item)
    {
        RestoreActions actions;

        auto prev_transition = ref_item->Transition();
        actions.emplace_back([=] {
            ref_item->Transition(prev_transition);
        });

        auto prev_slide_time = ref_item->SlideTime();
        actions.emplace_back([=] {
            ref_item->SlideTime(prev_slide_time);
        });

        auto prev_transition_time = ref_item->TransitionTime();
        actions.emplace_back([=] {
            ref_item->TransitionTime(prev_transition_time);
        });

        auto prev_loop = ref_item->Loop();
        actions.emplace_back([=] {
            ref_item->Loop(prev_loop);
        });

        auto prev_random = ref_item->Random();
        actions.emplace_back([=] {
            ref_item->Random(prev_random);
        });

        actions.emplace_back([=] {
            ref_item->Update();
        });

        return actions;
    }
};

struct ReceiverPropertyTraits {
    using Item = ReceiverItemHelper;

    static RestoreActions TakeSnapshot(Item* ref_item)
    {
        RestoreActions actions;

        auto prev_volume = ref_item->Volume();
        actions.emplace_back([=] {
            ref_item->Volume(prev_volume);
        });

        auto prev_mute = ref_item->IsMuted();
        actions.emplace_back([=] {
            ref_item->SetMuted(prev_mute);
        });

        auto prev_monitor = ref_item->GetMoinitoringType();
        actions.emplace_back([=] {
            ref_item->SetMonitoringType(prev_monitor);
        });

        return actions;
    }
};


struct BrowserPropertyTraits {
    using Item = BrowserItemHelper;

    static RestoreActions TakeSnapshot(Item* ref_item)
    {
        RestoreActions actions;
        auto prev_width = ref_item->width();
        actions.emplace_back([=] {
            ref_item->width(prev_width);
        });

        auto prev_height = ref_item->height();
        actions.emplace_back([=] {
            ref_item->height(prev_height);
        });

        auto prev_url = ref_item->GetUrl();
        actions.emplace_back([=] {
            ref_item->SetUrl(prev_url);
        });

        auto prev_css = ref_item->GetCSS();
        actions.emplace_back([=] {
            ref_item->SetCSS(prev_css);
        });

        auto prev_restart = ref_item->GetRestartWhenActive();
        actions.emplace_back([=] {
            ref_item->SetRestartWhenActive(prev_restart);
        });

        auto prev_volume = ref_item->Volume();
        actions.emplace_back([=] {
            ref_item->Volume(prev_volume);
        });

        auto prev_mute = ref_item->IsMuted();
        actions.emplace_back([=] {
            ref_item->SetMuted(prev_mute);
        });

        actions.emplace_back([=] {
            ref_item->Update();
        });

        return actions;
    }
};

struct AudioDevicePropertyTraits {
    using Item = AudioDeviceItemHelper;

    static RestoreActions TakeSnapshot(Item* ref_item)
    {
        RestoreActions actions;

        auto prev_volume = ref_item->Volume();
        actions.emplace_back([=] {
            ref_item->Volume(prev_volume);
        });

        auto prev_mute = ref_item->IsMuted();
        actions.emplace_back([=] {
            ref_item->SetMuted(prev_mute);
        });

        auto prev_monitor = ref_item->GetMoinitoringType();
        actions.emplace_back([=] {
            ref_item->SetMonitoringType(prev_monitor);
        });

        auto pre_selected_device = ref_item->SelectedAudioDevice();
        actions.emplace_back([=] {
            ref_item->SelectedAudioDevice(pre_selected_device);
        });

        auto pre_is_use_device_timing = ref_item->IsUseDeviceTiming();
        actions.emplace_back([=] {
            ref_item->SetUseDeviceTiming(pre_is_use_device_timing);
        });

        actions.emplace_back([=] {
            ref_item->Update();
        });
        return actions;
    }
};

//struct VtuberPropertyTraits {
//    using Item = VtuberItemHelper;
//
//    static RestoreActions TakeSnapshot(Item* ref_item)
//    {
//        RestoreActions actions;
//
//        auto prev_auto_blink = ref_item->IsAutoBlink();
//        actions.emplace_back([ref_item, prev_auto_blink] {
//            ref_item->SetAutoBlink(prev_auto_blink);
//        });
//
//        auto prev_auto_breath = ref_item->IsAutoBreath();
//        actions.emplace_back([ref_item, prev_auto_breath] {
//            ref_item->SetAutoBreath(prev_auto_breath);
//        });
//
//        double prev_eye_range_v1;
//        double prev_eye_range_v2;
//        ref_item->GetEyeRange(&prev_eye_range_v1, &prev_eye_range_v2);
//        actions.emplace_back([ref_item, prev_eye_range_v1, prev_eye_range_v2] {
//            ref_item->SetEyeRange(prev_eye_range_v1, prev_eye_range_v2);
//        });
//
//        auto prev_eye_sens = ref_item->GetEyeSensitivity();
//        actions.emplace_back([ref_item, prev_eye_sens] {
//            ref_item->SetEyeSensitivity(prev_eye_sens);
//        });
//
//        auto prev_mouth_sens = ref_item->GetMouthSensitivity();
//        actions.emplace_back([ref_item, prev_mouth_sens] {
//            ref_item->SetMouthSensitivity(prev_mouth_sens);
//        });
//
//        auto prev_head_sens = ref_item->GetHeadSensitivity();
//        actions.emplace_back([ref_item, prev_head_sens] {
//            ref_item->SetHeadSensitivity(prev_head_sens);
//        });
//
//        auto horizontal_filp = ref_item->GetHorizontalFlip();
//        actions.emplace_back([=] {
//            ref_item->SetHorizontalFlip(horizontal_filp);
//        });
//
//        return actions;
//    }
//};

using CameraPropertySnapshot = SourcePropertySnapshot<CameraPropertyTraits>;
using BeautyPropertySnapshot = SourcePropertySnapshot<BeautyPropertyTraits>;
using WindowCaptureSnapshot = SourcePropertySnapshot<WindowCapturePropertyTraits>;
using ColorKeyPropertySnapshot = SourcePropertySnapshot<ColorKeyPropertyTraits>;
using BeautyShapePropertySnapshot = SourcePropertySnapshot<BeautyShapePropertyTraits>;
using ImagePropertySnapshot = SourcePropertySnapshot<ImagePropertyTraits>;
using MediaPropertySnapshot = SourcePropertySnapshot<MediaPropertyTraits>;
using TextPropertySnapshot = SourcePropertySnapshot<TextPropertyTraits>;
using AlbumPropertySnapshot = SourcePropertySnapshot<AlbumPropertyTraits>;
using ColorPropertySnapshot = SourcePropertySnapshot<ColorPropertyTraits>;
using ReceiverPropertySnapshot = SourcePropertySnapshot<ReceiverPropertyTraits>;
using BrowserPropertySnapshot = SourcePropertySnapshot<BrowserPropertyTraits>;
using AudioDevicePropertySnapshot = SourcePropertySnapshot<AudioDevicePropertyTraits>;
//using VtuberPropertySnapshot = SourcePropertySnapshot<VtuberPropertyTraits>;
}   // namespace livehime

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_PROPERTY_SNAPSHOT_H_
