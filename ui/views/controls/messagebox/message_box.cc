#include "message_box.h"

#include "ui\base\win\dpi.h"

#include "ui\views\bubble\bubble_frame_view.h"
#include "ui\views\widget\widget_delegate.h"
#include "ui\views\widget\widget.h"
#include "ui\views\layout\box_layout.h"
#include "ui\views\controls\button\label_button.h"
#include "ui\views\layout\grid_layout.h"
#include "ui\views\layout\fill_layout.h"
#include "ui\views\bubble\bubble_border.h"
#include "ui\views\widget\widget_observer.h"
#include "ui\views\focus\accelerator_handler.h"

#include "base\message_loop\message_loop.h"
#include "base\run_loop.h"

namespace
{
    bool is_in_modal = false;

  void NullFunc(void)
  {
  }

  class MessageBoxResult
  {
  public:
    MessageBoxResult(base::MessageLoop *looper)
      : looper_(looper)
    {
    }

    base::string16 GetResultString()
    {
      return result_string_;
    }

    void SetResultString(base::string16 result_string)
    {
      result_string_ = result_string;
      looper_->Quit();
    }

  private:
    base::string16 result_string_;
    base::MessageLoop *looper_;
  };

  class MessageBoxLabel : public views::Label
  {
  public:
      explicit MessageBoxLabel(const string16& text)
          : views::Label(text)
          , contaner_for_multimode_(nullptr)
      {
          SetHorizontalAlignment(gfx::ALIGN_CENTER);
      }

      ~MessageBoxLabel(){}

      void SetMultiLineEx(bool multi_line)
      {
          __super::SetMultiLine(multi_line);

          if (multi_line)
          {
              DCHECK(!contaner_for_multimode_);
              if (!contaner_for_multimode_)
              {
                  contaner_for_multimode_ = new views::View();
                  views::FillLayout *layout = new views::FillLayout();
                  contaner_for_multimode_->SetLayoutManager(layout);
                  contaner_for_multimode_->AddChildView(this);
              }
          }
      }

      views::View* ContanerForMultiMode()
      {
          return contaner_for_multimode_;
      }

      gfx::Size GetPreferredSize()
      {
          gfx::Size pref_size;
          if (!contaner_for_multimode_)
          {
              pref_size = __super::GetPreferredSize();
          }
          else
          {
              int h = GetHeightForWidth(parent()->width());
              pref_size.SetSize(parent()->width(), h);
          }
          return pref_size;
      }

  private:
      views::View *contaner_for_multimode_;
  };

  const int messasge_box_border_ = 10;

  class CommonMessageBoxFrameView
    : public views::BubbleFrameView
  {
  public:
    CommonMessageBoxFrameView()
      : views::BubbleFrameView(gfx::Insets())
    {
    }

    virtual ~CommonMessageBoxFrameView()
    {
    }

    virtual int NonClientHitTest(const gfx::Point& point) OVERRIDE
    {
      int ht_result = __super::NonClientHitTest(point);
      if (ht_result != HTNOWHERE)
      {
        return ht_result;
      }

      gfx::Point client_point(point);
      if (!GetWidget()->client_view()->bounds().Contains(client_point))
        return HTCAPTION;

      return HTNOWHERE;
    }
  };

  class CommonMessageBoxView
    : public views::WidgetDelegateView
    , public views::ButtonListener
    , public views::WidgetObserver
  {
  public:
    typedef std::pair<const base::string16, const base::Closure> TitleAndClosurePair;
    typedef std::map<views::Button *, base::Closure> ButtonToClosureMap;

    explicit CommonMessageBoxView(
      const base::string16 &title,
      const base::string16& message,
      const gfx::ImageSkia* image_skia,
      const base::Closure &close_closure,
      const std::vector<TitleAndClosurePair> &button_list,
      gfx::HorizontalAlignment align = gfx::ALIGN_CENTER)
      : title_(title)
      , close_closure_(close_closure)
      , button_list_(button_list)
      , sender_(NULL)
      , focus_view_(NULL)
    {
      set_background(views::Background::CreateSolidBackground(0xff, 0xff, 0xff, 0xff));

      View *operator_panel = new views::View();
      operator_panel->SetLayoutManager(new views::BoxLayout(views::BoxLayout::kHorizontal, 0, 0, messasge_box_border_));
      for (std::vector<TitleAndClosurePair>::const_iterator it = button_list.begin();
        it != button_list.end(); ++it)
      {
        views::LabelButton *button = new views::LabelButton(this, it->first);
        button->SetStyle(views::Button::STYLE_BUTTON);
        operator_panel->AddChildView(button);
        button_map_[button] = it->second;
        if (focus_view_ == NULL)
        {
          focus_view_ = button;
        }
      }

      views::ImageView *image_view = new views::ImageView();
      if (image_skia)
      {
        image_view->SetImage(*image_skia);
      }

      /*views::Label *lable = new views::Label(message);
      lable->SetMultiLine(true);
      lable->SetHorizontalAlignment(gfx::ALIGN_LEFT);*/
      MessageBoxLabel *lable = new MessageBoxLabel(message);
      lable->SetMultiLineEx(true);
      lable->SetHorizontalAlignment(align);

      views::GridLayout *layout = new views::GridLayout(this);

      views::ColumnSet *content_set = layout->AddColumnSet(0);
      content_set->AddPaddingColumn(0, messasge_box_border_);
      content_set->AddColumn(views::GridLayout::Alignment::FILL,
        views::GridLayout::Alignment::LEADING, 0, views::GridLayout::SizeType::USE_PREF, 0, 0);
      content_set->AddPaddingColumn(0, messasge_box_border_);
      /*content_set->AddColumn(views::GridLayout::Alignment::FILL,
          views::GridLayout::Alignment::LEADING, 0, views::GridLayout::SizeType::USE_PREF, 0, 0);*/
      content_set->AddColumn(views::GridLayout::Alignment::FILL,
          views::GridLayout::Alignment::FILL, 0, views::GridLayout::SizeType::FIXED, 350, 0);
      content_set->AddPaddingColumn(0, messasge_box_border_);

      content_set = layout->AddColumnSet(1);
      /*content_set->AddPaddingColumn(1.0, messasge_box_border_);
      content_set->AddColumn(views::GridLayout::Alignment::FILL,
        views::GridLayout::Alignment::LEADING, 0, views::GridLayout::SizeType::USE_PREF, 0, 0);
      content_set->AddPaddingColumn(0, messasge_box_border_);*/
      content_set->AddPaddingColumn(1.0, messasge_box_border_);
      content_set->AddColumn(views::GridLayout::Alignment::FILL,
          views::GridLayout::Alignment::FILL, 0, views::GridLayout::SizeType::USE_PREF, 0, 0);
      content_set->AddPaddingColumn(1.0, messasge_box_border_);

      SetLayoutManager(layout);

      layout->StartRowWithPadding(0, 0, 0, messasge_box_border_);
      layout->AddView(image_view);
      layout->AddView(lable->ContanerForMultiMode());

      layout->StartRowWithPadding(0, 1, 0, messasge_box_border_);
      layout->AddView(operator_panel);
      layout->AddPaddingRow(0, messasge_box_border_);
    }

    virtual ~CommonMessageBoxView()
    {

    }

    virtual bool ShouldShowCloseButton() const OVERRIDE
    {
      return !close_closure_.Equals(views::NullClosure);
    }

    virtual views::NonClientFrameView* CreateNonClientFrameView(views::Widget* widget) OVERRIDE
    {
      views::BubbleFrameView* frame = new CommonMessageBoxFrameView();
      const SkColor color = widget->GetNativeTheme()->GetSystemColor(
        ui::NativeTheme::kColorId_DialogBackground);
      frame->SetBubbleBorder(new views::BubbleBorder(views::BubbleBorder::FLOAT,
        views::BubbleBorder::BIG_SHADOW,
        color));

      return frame;
    }

    virtual views::View* GetContentsView() OVERRIDE
    {
      return this;
    }

    virtual string16 GetWindowTitle() const OVERRIDE
    {
      return title_;
    }

    virtual void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) OVERRIDE
    {
      if (details.is_add && details.child == this)
      {
        GetWidget()->AddObserver(this);
        if (GetFocusManager() != NULL)
        {
          GetFocusManager()->SetFocusedView(focus_view_);
        }
      }
    }

    virtual void OnWidgetClosing(views::Widget* widget) OVERRIDE
    {
      GetWidget()->RemoveObserver(this);

      if (sender_)
      {
        button_map_[sender_].Run();
      }
      else
      {
        close_closure_.Run();
      }
    }

    virtual void ButtonPressed(views::Button* sender, const ui::Event& event) OVERRIDE
    {
      sender_ = sender;
      GetWidget()->Close();
    }

    virtual ui::ModalType GetModalType() const OVERRIDE
    {
      return ui::ModalType::MODAL_TYPE_WINDOW;
    };

  private:
    base::string16 title_;
    std::vector<TitleAndClosurePair> button_list_;
    ButtonToClosureMap button_map_;
    base::Closure close_closure_;

    View *focus_view_;

    views::Button *sender_;
  };

  void SplitStringIntoParagraphs(const string16& text,
    std::vector<string16>* paragraphs) {
    paragraphs->clear();

    size_t start = 0;
    for (size_t i = 0; i < text.length(); ++i) {
      if (text[i] == ',') {
        paragraphs->push_back(text.substr(start, i - start));
        start = i + 1;
      }
    }
    paragraphs->push_back(text.substr(start, text.length() - start));
  }

  void MovieWindowToWindowCenter(gfx::NativeView window, gfx::NativeView parent_window)
  {
    views::Widget *parent_widget = views::Widget::GetWidgetForNativeView(parent_window);
    views::Widget *child_widget = views::Widget::GetWidgetForNativeView(window);

    gfx::Rect target_bounds = parent_widget->GetWindowBoundsInScreen();
    target_bounds.ClampToCenteredSize(child_widget->GetWindowBoundsInScreen().size());

    child_widget->SetBounds(target_bounds);
  }
}

namespace views
{
  base::Closure NullClosure = base::Bind(&NullFunc);

  views::Widget* ShowMessageBox(
    gfx::NativeView parent_widget,
    const base::string16& title,
    const base::string16& message,
    const gfx::ImageSkia* image_skia,
    const base::Closure close_closure,
    const base::string16& buttons,
    const base::Closure button_closure,
    ...)
  {
    std::vector<string16> paragraphs;
    SplitStringIntoParagraphs(buttons, &paragraphs);
    DCHECK((int)paragraphs.size() >= 1);

    std::vector<std::pair<const base::string16, const base::Closure>> button_list;
    if (!buttons.empty())
    {
      button_list.push_back(std::make_pair(paragraphs[0], button_closure));
    }

    va_list arg_ptr;
    va_start(arg_ptr, button_closure);
    for (size_t i = 1; i < paragraphs.size(); i++)
    {
      const base::Closure closure = va_arg(arg_ptr, const base::Closure);
      button_list.push_back(std::make_pair(paragraphs[i], closure));
    }
    va_end(arg_ptr);

    CommonMessageBoxView* delegate_view =
      new CommonMessageBoxView(title, message, image_skia, close_closure, button_list);
    views::Widget *widget = new views::Widget();
    views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW);
    params.parent = parent_widget;
    params.delegate = delegate_view;
    params.opacity = Widget::InitParams::TRANSLUCENT_WINDOW;
    params.remove_standard_frame = true;
    widget->Init(params);
    if (IsWindow(parent_widget))
    {
      MovieWindowToWindowCenter(widget->GetNativeWindow(), parent_widget);
    }

    widget->Show();

    // 贴屏幕就近弹框
    InsureWidgetVisible(widget, InsureShowCorner::ISC_NEARBY);

    return widget;
  }

  const base::string16 ShowMessageBox(
      gfx::NativeView parent_widget,
      const base::string16& title,
      const base::string16& message,
      const gfx::ImageSkia* image_skia,
      const base::string16& buttons,
      gfx::HorizontalAlignment align/* = gfx::ALIGN_CENTER*/)
  {
    DCHECK(base::MessageLoopForUI::current());

    is_in_modal = true;

    MessageBoxResult result(base::MessageLoopForUI::current());

    std::vector<string16> paragraphs;
    SplitStringIntoParagraphs(buttons, &paragraphs);
    DCHECK((int)paragraphs.size() >= 1);

    std::vector<std::pair<const base::string16, const base::Closure>> button_list;
    for (size_t i = 0; i < paragraphs.size(); i++)
    {
      const base::Closure closure = base::Bind(&MessageBoxResult::SetResultString,
        base::Unretained(&result), paragraphs[i]);
      button_list.push_back(std::make_pair(paragraphs[i], closure));
    }

    const base::Closure closure = base::Bind(&MessageBoxResult::SetResultString,
       base::Unretained(&result), base::string16());

    // 若当前父窗体已有浮于其上的窗口，则以此窗口为父窗口
    if (parent_widget)
    {
        HWND hPopup = ::GetLastActivePopup(parent_widget);
        if (hPopup)
        {
            views::Widget *top_widget = views::Widget::GetWidgetForNativeWindow(hPopup);
            if (top_widget)
            {
                parent_widget = hPopup;
            }
        }
    }

    CommonMessageBoxView* delegate_view =
        new CommonMessageBoxView(title, message, image_skia, closure, button_list, align);
    views::Widget *widget = new views::Widget();
    views::Widget::InitParams params;
    params.parent = parent_widget;
    params.delegate = delegate_view;
    params.opacity = Widget::InitParams::TRANSLUCENT_WINDOW;
    params.remove_standard_frame = true;
    widget->set_frame_type(views::Widget::FRAME_TYPE_FORCE_CUSTOM);
    widget->Init(params);
    if (IsWindow(parent_widget))
    {
      MovieWindowToWindowCenter(widget->GetNativeWindow(), parent_widget);
    }

    widget->Show();

    // 贴屏幕就近弹框
    InsureWidgetVisible(widget, InsureShowCorner::ISC_NEARBY);

    base::MessageLoopForUI* loop = base::MessageLoopForUI::current();
    base::MessageLoopForUI::ScopedNestableTaskAllower allow_nested(loop);
    views::AcceleratorHandler accelerator_handler;
    base::RunLoop run_loop(&accelerator_handler);
    auto quit_inner_loop = run_loop.QuitClosure();
    run_loop.Run();
    quit_inner_loop.Run();

    if (params.parent)
    {
        ::SetActiveWindow(params.parent);
    }

    is_in_modal = false;

    return result.GetResultString();
  }

  bool IsInModalNow()
  {
      return is_in_modal;
  }

  void InsureWidgetVisible(views::Widget *widget, InsureShowCorner isc)
  {
      if (!widget || isc == ISC_NONE)
          return;

      // 确保弹出框可见，并优先保证其右上角可见
      gfx::Rect rcWnd = widget->GetWindowBoundsInScreen();
      HMONITOR monitor = MonitorFromWindow(widget->GetNativeView(), MONITOR_DEFAULTTONEAREST);
      if (monitor)
      {
          MONITORINFO mi = { 0 };
          mi.cbSize = sizeof(mi);
          GetMonitorInfo(monitor, &mi);
          gfx::Rect rcMonitorWork(mi.rcWork);
          switch (isc)
          {
          case ISC_CENTER:
          {
              int x = 0, y = 0;
              if (rcMonitorWork.width() >= rcWnd.width())
              {
                  x = rcMonitorWork.x() + (rcMonitorWork.width() - rcWnd.width()) / 2;
              }
              else
              {
                  x = rcMonitorWork.x();
              }
              if (rcMonitorWork.height() >= rcWnd.height())
              {
                  y = rcMonitorWork.y() + (rcMonitorWork.height() - rcWnd.height()) / 2;
              }
              else
              {
                  y = rcMonitorWork.y();
              }
              ::SetWindowPos(widget->GetNativeView(), NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
          }
              break;
          case ISC_NEARBY:
          {
              int x = std::max(rcMonitorWork.x(), rcWnd.x());
              x = std::min(rcMonitorWork.right() - rcWnd.width(), x);
              int y = std::max(rcMonitorWork.y(), rcWnd.y());
              y = std::min(rcMonitorWork.bottom() - rcWnd.height(), y);

              ::SetWindowPos(widget->GetNativeView(), NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
          }
          	break;
          case ISC_BOTTOMRIGHT:
          {
              int x = 0, y = 0;
              int visible_x = rcMonitorWork.right() - rcWnd.width();
              int visible_y = rcMonitorWork.bottom() - rcWnd.height();
              if (rcMonitorWork.width() >= rcWnd.width())
              {
                  x = visible_x;
              }
              else
              {
                  x = rcWnd.x();
              }
              if (rcMonitorWork.height() >= rcWnd.height())
              {
                  y = visible_y;
              }
              else
              {
                  y = rcWnd.y();
              }
              ::SetWindowPos(widget->GetNativeView(), NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
          }
              break;
          default:
              break;
          }
      }
  }

}