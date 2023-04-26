#ifndef BILILIVE_BILILIVE_UI_VIEWS_MENU_COLIVE_SCENE_ITEM_MENU_H_
#define BILILIVE_BILILIVE_UI_VIEWS_MENU_COLIVE_SCENE_ITEM_MENU_H_

#include "ui/views/context_menu_controller.h"
#include "ui/views/controls/menu/menu_delegate.h"


namespace views {
    class MenuRunner;
}

namespace obs_proxy {
    class SceneItem;
}

// 连麦场景源 item 菜单
class ColiveSceneItemMenu
    : public views::ContextMenuController
    , public views::MenuDelegate
{
public:
    explicit ColiveSceneItemMenu(views::View *target);
    ~ColiveSceneItemMenu();

    void CancelMenu();

protected:
    // ContextMenuController
    void ShowContextMenuForView(views::View* source,
        const gfx::Point& point,
        ui::MenuSourceType source_type) override;

    // MenuDelegate
    bool IsItemChecked(int command_id) const override;
    void ExecuteCommand(int command_id, int event_flags) override;

    virtual void BeforeRunMenu(views::MenuItemView* menu) {}

private:
    obs_proxy::SceneItem* GetSelectedSceneItem() const;

    views::View* target_;
    scoped_ptr<views::MenuRunner> menu_runner_;
};

#endif // BILILIVE_BILILIVE_UI_VIEWS_MENU_COLIVE_SCENE_ITEM_MENU_H_