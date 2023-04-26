#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_structs.h"


DanmakuData::DanmakuData()
    : type(DanmakuType::DT_NOSUPPORT),
      user_id(0),
      special_type(0),
      guard_type(0),
      gift_id(0),
      gift_num(0),
      gift_cost(0),
      gift_price(0),
      gift_stamp(0),
      gift_batch_num(0),
      gift_combo_num(0),
      gift_super_num(0),
      has_combo_send(false),
      gift_coin_type(GiftCoinType::GCT_SILVER),
      use_default_color(true),
      text_color(0)
{
}


namespace dmkhime {

// static
std::atomic_uint64_t DmkDataObject::id_counter = 1;


DmkDataObject::DmkDataObject()
    : identifier_(id_counter.fetch_add(1, std::memory_order_relaxed))
{
}

uint64_t DmkDataObject::GetId() const {
    return identifier_;
}

CommonDmkData::CommonDmkData()
    : guard_type(0) {}

GiftDmkData::GiftDmkData()
    : gift_id(0),
      gift_num(0),
      gift_coin_type(GiftCoinType::GCT_SILVER) {}

DetailedGiftDmkData::DetailedGiftDmkData()
    : guard_type(0),
    gift_id(0),
    gift_cost(0),
    gift_num(0),
    gift_stamp(0),
    gift_coin_type() {}

GiftComboDmkData::GiftComboDmkData()
    : gift_id(0),
      gift_combo_num(0),
      gift_coin_type(GiftCoinType::GCT_SILVER) {}

GuardBuyDmkData::GuardBuyDmkData()
    : guard_type(0),
      gift_num(0) {}

DetailedGuardBuyDmkData::DetailedGuardBuyDmkData()
    : guard_type(0),
      gift_num(0),
      gift_cost(0),
      gift_stamp(0) {}

GuardAchievementDmkData::GuardAchievementDmkData()
{}

AnnocDmkData::AnnocDmkData() {}

ComboStripDmkData::ComboStripDmkData()
    : guard_type(0),
      user_id(0),
      gift_id(0),
      combo_number(0),
      total_price(0),
      timestamp(0),
      stay_time(0),
      start_color(),
      end_color() {
}

GuardBuyBannerDmkData::GuardBuyBannerDmkData()
    : guard_type(0),
      guard_time(0),
      stay_time(0) {
}

GuardEnterBannerDmkData::GuardEnterBannerDmkData()
    : guard_type(0),
      stay_time(0) {
}

CommonNoticeDanmakuRenderData::CommonNoticeDanmakuRenderData()
{
}

LiveAsistantDanmakuRenderData::LiveAsistantDanmakuRenderData()
{

}

}


DanmakuRenderingParams::DanmakuRenderingParams()
    :opacity(255),
    base_font_size(15),
    theme(dmkhime::Theme::SORCERESS),
    show_icon(true),
    hardware_acceleration_(false) {}