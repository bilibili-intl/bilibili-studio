#ifndef BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TABAREA_SCENE_CONTENT_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TABAREA_SCENE_CONTENT_VIEW_H

#include "base/notification/notification_observer.h"
#include "base/notification/notification_registrar.h"
#include "base/timer/timer.h"

#include "bililive/bililive/ui/views/controls/listview.h"
#include "bililive/bililive/ui/views/menu/sceneitem_menu.h"
#include "bililive/bililive/ui/views/menu/sceneitem_menu_cmd.h"

#include "ui/views/controls/button/button.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/controls/textfield/textfield_controller.h"

#include "obs/obs_proxy/public/proxy/obs_proxy.h"
#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"


class ChatRoomMainView;

namespace
{
    class SceneItemRenameTextfield;

    class SceneItemRenameDelegate : public views::TextfieldController
    {
    public:
        virtual base::string16 GetSceneName() const = 0;
        virtual void SetFocus() = 0;
        virtual void OnFinishEdit() = 0;
    };
}

class BililiveImageButton;
class BililiveLabel;
class MediaPropDetailView;

// There are n instances corresponding to scenes 1, 2, 3...
class TabAreaSceneContentViddupView
    : public views::View
    , views::ButtonListener
    , ListViewController
    , obs_proxy_ui::OBSUIProxyObserver
    , base::NotificationObserver
{
public:
    TabAreaSceneContentViddupView(int index, const base::string16 &name);
    virtual ~TabAreaSceneContentViddupView();

    int scene_index() { return scene_index_; }
    base::string16 scene_name() const { return scene_name_; }
    bool IsCurrentSceneSelected() const;

    void RemoveAllSceneItem();

    void AddVtuberStromSceneItem(const std::string& mp4_path, int type);
    void SortStormItem(int vtuber_floor);

    gfx::Rect GetCameraItemEditBtnBounds(const std::string& name) const;

    void ShowOperateView(const std::string& name, bool show);

    void ShowOrHideCanvas(bool enter);

    void ShowChatRoomView(bool show_canvas);

	  void ShowEmptyView();

protected:
    //override from View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // ListViewController
    void OnItemSelectedChanged(ListItemView::ListItemViewChangeReason reason) override;
    void OnItemsOrderChanged(ListItemView::ListItemViewChangeReason reason) override;
    bool FindListItem(const LVFINDITEMINFO* pFindInfo, ListItemView* item_view) override;

    // OBSUIProxyObserver
    void OnSourceRename(const base::string16& prev_item_name, const base::string16& new_item_name) override;
    void OnSceneItemAdd(const base::string16& scene_name, const obs_proxy_ui::TableItem& table_item, obs_source_t* source, int source_type) override;
    void OnSceneItemRemove(const base::string16& scene_name, const base::string16& item_name) override;
    void OnSceneItemSelected(const base::string16& scene_name, const base::string16& item_name) override;
    void OnSceneItemDeselect(const base::string16& scene_name, const base::string16& item_name) override;
    void OnSceneItemReordered(const base::string16& scene_name, const std::vector<string16> &item_names) override;
    void OnSceneItemVisible(const base::string16& scene_name, const base::string16& item_name, bool visible) override;
    void OnSceneItemLocked(const base::string16& scene_name, const base::string16& item_name, bool locked) override;

    // NotificationObserver
    void Observe(int type,
                 const base::NotificationSource& source,
                 const base::NotificationDetails& details) override;

private:
    void InitViews();
    void ExecuteSceneItemRelatedCommand(int cmd_id, const string16 &item_name);
    void EnsureSelectedItemVisible();

    void OnMediaStateChangeCallBack(bool play_end);
    void StartShowStormMp4(obs_proxy::SceneItem* scene);

private:
    views::ImageButton *move_up_button_;
    views::ImageButton *move_down_button_;
    views::ImageButton *top_button_;
    views::ImageButton *bottom_button_;
    views::View* empty_view_ = nullptr;

    ListView *list_view_;
    int add_materials_view_seq_id_ = -1;
    int chat_room_view_seq_id_ = -1;

    MediaPropDetailView* media_view_ = nullptr;
    //std::unique_ptr<ChatRoomMainView>   chat_room_main_view_;

    int scene_index_;
    base::string16 scene_name_;
    base::NotificationRegistrar notifation_register_;
    base::WeakPtrFactory<TabAreaSceneContentViddupView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(TabAreaSceneContentViddupView);
};

// Scene source list entry View
class TabAreaSceneItemViddupView
    : public ListItemView
    , public views::ButtonListener
    , public SceneItemMenu
    , public SceneItemRenameDelegate
{
public:
    TabAreaSceneItemViddupView(ListView *list_view, const base::string16& scene_name, const obs_proxy_ui::TableItem& table_item, int source_type);
    virtual ~TabAreaSceneItemViddupView();

    void SetItemName(const base::string16& item_name);
    base::string16 item_name() const { return item_info_.contents_; }
    void RemoveSceneItem();
    void SetItemVisible(bool visible);
    void SetItemLocked(bool lock);
    bool item_visible() const { return item_info_.visible_; }

    // Camera edit control screen coordinates
    gfx::Rect GetCameraItemEditBtnBounds() const;

    void ShowOperateView(bool show);

    void LimitItemEdit();
    void SceneItemDelete();

protected:
    //override from View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    void OnMouseEntered(const ui::MouseEvent& event) override;
    void OnMouseExited(const ui::MouseEvent& event) override;
    bool OnMousePressed(const ui::MouseEvent& event) override;
    bool OnMouseDragged(const ui::MouseEvent& event) override;
    void OnMouseReleased(const ui::MouseEvent& event) override;
    void OnKeyEvent(ui::KeyEvent* event) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // ListItemView
    void OnSelectedChanged(ListItemViewChangeReason reason) override;

    // SceneItemRenameDelegate
    base::string16 GetSceneName() const override;
    void SetFocus() override;
    void OnFinishEdit() override;

    // TextfieldController
    void ContentsChanged(views::Textfield* sender, const string16& new_contents) override;

private:
    void InitViews();
    void ExecuteSceneItemRelatedCommand(int cmd_id);
    void ShowRenameTextfield(bool show);
    gfx::ImageSkia* GetImageSkia(int type);
    bool IsBrowserPlugin();
    void EventPolaris(int type,int switch_type);

private:
    base::string16 scene_name_;
    obs_proxy_ui::TableItem item_info_;
    int64_t plugin_id_ = 0;
    int source_type_;
    BililiveImageButton* icon_button_;
    views::View* name_view_;
    BililiveLabel* item_name_label_;
    SceneItemRenameTextfield* rename_textfield_;
    views::View* operate_view_;
    BililiveImageButton* look_button_;
    BililiveImageButton* edit_button_;
    BililiveImageButton* del_button_;
    BililiveImageButton* lock_button_;

    bool should_start_rename_timer_;
    base::OneShotTimer<TabAreaSceneItemViddupView> rename_click_timer_;
    base::WeakPtrFactory<TabAreaSceneItemViddupView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(TabAreaSceneItemViddupView);
};

#endif  //BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TABAREA_SCENE_CONTENT_VIEW_H