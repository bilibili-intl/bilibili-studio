#ifndef BILILIVE_BILILIVE_UI_VIEWS_MENU_SCENEITEM_MENU_H
#define BILILIVE_BILILIVE_UI_VIEWS_MENU_SCENEITEM_MENU_H

#include "ui/views/context_menu_controller.h"
#include "ui/views/controls/menu/menu_delegate.h"

namespace views{
    class MenuRunner;
}

// 场景源item项菜单，主界面预览窗和侧边栏场景页中源列表项继承使用
class SceneItemMenu
    : public views::ContextMenuController
    , public views::MenuDelegate
{
public:
    explicit SceneItemMenu(views::View *target);
    virtual ~SceneItemMenu();

    void CancelMenu();

protected:
    //override from ContextMenuController
    void ShowContextMenuForView(views::View* source,
        const gfx::Point& point,
        ui::MenuSourceType source_type) override;

    // MenuDelegate
    bool IsItemChecked(int command_id) const override;
    void ExecuteCommand(int command_id, int event_flags) override;

    virtual void BeforeRunMenu(views::MenuItemView* menu) {}

private:
	scoped_ptr<views::MenuRunner> menu_runner_;

    views::View *target_;
};

#endif // BILILIVE_BILILIVE_UI_VIEWS_MENU_SCENEITEM_MENU_H