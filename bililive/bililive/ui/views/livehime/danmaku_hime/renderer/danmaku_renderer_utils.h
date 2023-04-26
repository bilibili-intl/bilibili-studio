#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_RENDERER_DANMAKU_RENDERER_UTILS_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_RENDERER_DANMAKU_RENDERER_UTILS_H_

#include "SkColor.h"

#include "base/strings/string16.h"

#include <d2d1.h>


namespace dmkhime {

// µ¯Ä»ÑÕÉ«
extern const D2D1_COLOR_F cost_gold_color;
extern const D2D1_COLOR_F cost_silver_color;
extern const D2D1_COLOR_F action_accent_color;

extern const D2D1_COLOR_F viddup_dmk_system;
extern const D2D1_COLOR_F viddup_dmk_user;
extern const D2D1_COLOR_F viddup_dmk_gift;
extern const D2D1_COLOR_F viddup_dmk_warning;

D2D1_COLOR_F ConvertColor(SkColor color, float alpha = 1.f);
D2D1_COLOR_F ConvertColorARGB(SkColor color);
string16 TimeStampToString(int64_t time_stamp);

}

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_RENDERER_DANMAKU_RENDERER_UTILS_H_