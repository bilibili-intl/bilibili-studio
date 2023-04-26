#include "bililive/bililive/ui/views/livehime/danmaku_hime/renderer/danmaku_renderer_utils.h"

#include <algorithm>

#include "base/strings/stringprintf.h"
#include "base/time/time.h"


namespace dmkhime {

// 弹幕颜色
const D2D1_COLOR_F cost_gold_color = D2D1::ColorF(0xfca622);
const D2D1_COLOR_F cost_silver_color = D2D1::ColorF(0xadd2f6);
const D2D1_COLOR_F action_accent_color = D2D1::ColorF(0xf7b500);

const D2D1_COLOR_F viddup_dmk_system = D2D1::ColorF(0x999999);     //海外版弹幕系统消息颜色
const D2D1_COLOR_F viddup_dmk_user = D2D1::ColorF(0xFFFFFF);       //海外版弹幕用户发言消息颜色
const D2D1_COLOR_F viddup_dmk_gift = D2D1::ColorF(0xFF9E03);       //海外版弹幕礼物消息颜色

const D2D1_COLOR_F viddup_dmk_warning = D2D1::ColorF(0xff4747);    //海外版超管警告消息颜色

D2D1_COLOR_F ConvertColor(SkColor color, float alpha) {
    D2D1_COLOR_F d2d_color;

    d2d_color.a = std::min(alpha, 1.f);
    d2d_color.a = std::max(d2d_color.a, 0.f);

    d2d_color.r = SkColorGetR(color) / 255.f;
    d2d_color.g = SkColorGetG(color) / 255.f;
    d2d_color.b = SkColorGetB(color) / 255.f;

    return d2d_color;
}

D2D1_COLOR_F ConvertColorARGB(SkColor color) {
    D2D1_COLOR_F d2d_color;

    d2d_color.a = SkColorGetA(color) / 255.f;
    d2d_color.r = SkColorGetR(color) / 255.f;
    d2d_color.g = SkColorGetG(color) / 255.f;
    d2d_color.b = SkColorGetB(color) / 255.f;

    return d2d_color;
}

string16 TimeStampToString(int64_t time_stamp) {
    if (time_stamp <= 0) {
        return L"00:00:00";
    }

    base::Time::Exploded exploded;
    base::Time::FromTimeT(time_stamp).LocalExplode(&exploded);

    auto time = base::StringPrintf(L"%02d:%02d:%02d",
        exploded.hour,
        exploded.minute,
        exploded.second);

    return time;
}

}
