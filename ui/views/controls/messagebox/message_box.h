#ifndef UI_VIEWS_CONTROLS_MESSAGEBOX_MESSAGE_BOX_H
#define UI_VIEWS_CONTROLS_MESSAGEBOX_MESSAGE_BOX_H

#include "base\bind.h"
#include "base\callback.h"
#include "base\strings\string16.h"
#include "ui\gfx\image\image_skia.h"
#include "ui\views\views_export.h"
#include "ui\gfx\native_widget_types.h"
#include "ui\gfx\text_constants.h"


// eg:
//void MessageReturn(int button_id) {
//
//}
//ui::ResourceBundle &rb = ui::ResourceBundle::GetSharedInstance();
//views::ShowMessageBox(NULL, L"测试", L"测试测试\n\n测试测\n\n\n\n测试测试",
//  rb.GetImageSkiaNamed(IDR_APP),
//  L"1,2,3,5,6",
//  base::Bind(&MessageReturn, 1),
//  base::Bind(&MessageReturn, 2),
//  base::Bind(&MessageReturn, 3),
//  base::Bind(&MessageReturn, 5),
//  base::Bind(&MessageReturn, 6));

namespace views {
  extern base::Closure NullClosure;
  class Widget;

  VIEWS_EXPORT const base::string16 ShowMessageBox(
    gfx::NativeView parent_widget,
    const base::string16& title,
    const base::string16& message,
    const gfx::ImageSkia* image_skia,
    const base::string16& buttons,
    gfx::HorizontalAlignment align = gfx::ALIGN_CENTER);

  VIEWS_EXPORT views::Widget* ShowMessageBox(
    gfx::NativeView parent_widget,
    const base::string16& title,
    const base::string16& message,
    const gfx::ImageSkia* image_skia,
    const base::Closure close_closure,
    const base::string16& buttons,
    const base::Closure button_closure,
    ...);

  // 当前是否存在未决断的modal提示框
  VIEWS_EXPORT bool IsInModalNow();

  // 系统分辨率变更之后确保哪边可见
  enum InsureShowCorner
  {
      ISC_NONE,
      ISC_CENTER,   // 居中，居中显示不全就保证左上角显示
      ISC_NEARBY,   // 部分被隐藏之后根据当前位置就近贴屏显示
      //ISC_TOPLEFT,  // 保证左上角显示
      //ISC_TOPRIGHT, // 保证右上角显示
      //ISC_BOTTOMLEFT,   // 保证左下角显示
      ISC_BOTTOMRIGHT,  // 保证右下角显示
  };

  VIEWS_EXPORT void InsureWidgetVisible(views::Widget *widget, InsureShowCorner isc);
}
#endif