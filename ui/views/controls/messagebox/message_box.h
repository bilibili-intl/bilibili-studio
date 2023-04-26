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
//views::ShowMessageBox(NULL, L"����", L"���Բ���\n\n���Բ�\n\n\n\n���Բ���",
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

  // ��ǰ�Ƿ����δ���ϵ�modal��ʾ��
  VIEWS_EXPORT bool IsInModalNow();

  // ϵͳ�ֱ��ʱ��֮��ȷ���ı߿ɼ�
  enum InsureShowCorner
  {
      ISC_NONE,
      ISC_CENTER,   // ���У�������ʾ��ȫ�ͱ�֤���Ͻ���ʾ
      ISC_NEARBY,   // ���ֱ�����֮����ݵ�ǰλ�þͽ�������ʾ
      //ISC_TOPLEFT,  // ��֤���Ͻ���ʾ
      //ISC_TOPRIGHT, // ��֤���Ͻ���ʾ
      //ISC_BOTTOMLEFT,   // ��֤���½���ʾ
      ISC_BOTTOMRIGHT,  // ��֤���½���ʾ
  };

  VIEWS_EXPORT void InsureWidgetVisible(views::Widget *widget, InsureShowCorner isc);
}
#endif