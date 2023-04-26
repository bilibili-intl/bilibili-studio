#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_BASE_SETTINGS_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_BASE_SETTINGS_VIEW_H_

#include "ui/base/resource/resource_bundle.h"

#include "bililive/bililive/ui/views/controls/bililive_floating_scroll_view.h"


namespace base {
    class DictionaryValue;
}

// 各设置子页公共基类
class BaseSettingsView
    : public BililiveViewWithFloatingScrollbar
{
public:
    BaseSettingsView() = default;

    enum ChangeType {
        TYPE_ERR = -1,
        // 推流设置改变的标识。该标识指示推流设置未改变。
        TYPE_NONE = 0,
        // 推流设置改变的标识。该标识指示改变的推流设置不需重推流。
        TYPE_SAVE = 1 << 0,
        // 推流设置改变的标识。该标识指示改变的推流设置只影响本地。
        TYPE_LOCAL = 1 << 1,
        // 推流设置改变的标识。该标识指示推流目标改变。
        TYPE_SERVER = 1 << 2,

        // 推流设置从自定义变更为自动
        TYPE_CHANGE_TO_AUTO = 1 << 3,

        TYPE_SAVE_LOCAL = TYPE_SAVE | TYPE_LOCAL,
        TYPE_SAVE_SERVER = TYPE_SAVE | TYPE_SERVER,
        TYPE_LOCAL_SERVER = TYPE_LOCAL | TYPE_SERVER,
        TYPE_SAVE_LOCAL_SERVER = TYPE_SAVE | TYPE_LOCAL | TYPE_SERVER,
    };

    // 检查设置是否有效。该方法会在用户点击确认按钮时被调用。
    // 若某一子页面想要在设置保存之前先检查用户指定的设置数据是否合法，
    // 则它应在实现中进行数据的检查；否则直接返回 true 即可。
    // @return: 若设置数据有效，返回 true，否则返回 false。
    virtual bool CheckSettingsValid() = 0;

    // 保存普通设置。普通设置是指与推流无关的设置。
    // @return: 若保存成功，返回 true，否则返回 false。
    virtual bool SaveNormalSettingsChange() = 0;

    // 保存推流相关设置，或只检查推流相关设置是否改变。
    // {check}: 为 true 时只检查推流设置是否改变；为 false 时保存推流设置。
    // @return: 当 {check} 为 true 时，返回推流设置改变的标识；为 false 时
    //          返回保存成功或失败的标识：成功为 1，失败为 0。
    virtual void SaveOrCheckStreamingSettingsChange(bool check, ChangeType &result) {}

    virtual void OnCancel() {}

public:
    // View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) override;

    virtual void InitViews() = 0;
    virtual void InitData() = 0;
    virtual void UninitViews() {}

protected:
    static int radio_button_groupid_;

private:
    DISALLOW_COPY_AND_ASSIGN(BaseSettingsView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_BASE_SETTINGS_VIEW_H_