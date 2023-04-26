/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_BILILIVE_LIVEHIME_VOLUME_SOUND_EFFECT_PROPERTY_SNAPSHOT_H_
#define BILILIVE_BILILIVE_LIVEHIME_VOLUME_SOUND_EFFECT_PROPERTY_SNAPSHOT_H_

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

struct MicphonePropertyTraits {
    using Item = MicphoneDeviceHelper;

    static RestoreActions TakeSnapshot(Item* ref_item)
    {
        RestoreActions actions;

        auto prev_is_reverb_enabled = ref_item->IsReverbEnabled();
        actions.emplace_back([=] {
            ref_item->IsReverbEnabled(prev_is_reverb_enabled);
        });

        auto prev_room_size = ref_item->Rb_RoomSize();
        actions.emplace_back([=] {
            ref_item->Rb_RoomSize(prev_room_size);
        });

        auto prev_damping = ref_item->Rb_Damping();
        actions.emplace_back([=] {
            ref_item->Rb_Damping(prev_damping);
        });

        auto prev_wet_level = ref_item->Rb_WetLevel();
        actions.emplace_back([=] {
            ref_item->Rb_WetLevel(prev_wet_level);
        });

        auto prev_dry_level = ref_item->Rb_DryLevel();
        actions.emplace_back([=] {
            ref_item->Rb_DryLevel(prev_dry_level);
        });

        auto prev_width = ref_item->Rb_Width();
        actions.emplace_back([=] {
            ref_item->Rb_Width(prev_width);
        });

        auto prev_freeze_mode = ref_item->Rb_FreezeMode();
        actions.emplace_back([=] {
            ref_item->Rb_FreezeMode(prev_freeze_mode);
        });

        actions.emplace_back([=] {
            ref_item->UpdateFilters();
        });

        return actions;
    }
};

using MicphonePropertySnapshot = SourcePropertySnapshot<MicphonePropertyTraits>;

}   // namespace livehime

#endif  // BILILIVE_BILILIVE_LIVEHIME_VOLUME_SOUND_EFFECT_PROPERTY_SNAPSHOT_H_
