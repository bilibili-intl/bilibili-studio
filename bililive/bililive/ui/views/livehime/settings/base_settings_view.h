#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_BASE_SETTINGS_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_BASE_SETTINGS_VIEW_H_

#include "ui/base/resource/resource_bundle.h"

#include "bililive/bililive/ui/views/controls/bililive_floating_scroll_view.h"


namespace base {
    class DictionaryValue;
}

// ��������ҳ��������
class BaseSettingsView
    : public BililiveViewWithFloatingScrollbar
{
public:
    BaseSettingsView() = default;

    enum ChangeType {
        TYPE_ERR = -1,
        // �������øı�ı�ʶ���ñ�ʶָʾ��������δ�ı䡣
        TYPE_NONE = 0,
        // �������øı�ı�ʶ���ñ�ʶָʾ�ı���������ò�����������
        TYPE_SAVE = 1 << 0,
        // �������øı�ı�ʶ���ñ�ʶָʾ�ı����������ֻӰ�챾�ء�
        TYPE_LOCAL = 1 << 1,
        // �������øı�ı�ʶ���ñ�ʶָʾ����Ŀ��ı䡣
        TYPE_SERVER = 1 << 2,

        // �������ô��Զ�����Ϊ�Զ�
        TYPE_CHANGE_TO_AUTO = 1 << 3,

        TYPE_SAVE_LOCAL = TYPE_SAVE | TYPE_LOCAL,
        TYPE_SAVE_SERVER = TYPE_SAVE | TYPE_SERVER,
        TYPE_LOCAL_SERVER = TYPE_LOCAL | TYPE_SERVER,
        TYPE_SAVE_LOCAL_SERVER = TYPE_SAVE | TYPE_LOCAL | TYPE_SERVER,
    };

    // ��������Ƿ���Ч���÷��������û����ȷ�ϰ�ťʱ�����á�
    // ��ĳһ��ҳ����Ҫ�����ñ���֮ǰ�ȼ���û�ָ�������������Ƿ�Ϸ���
    // ����Ӧ��ʵ���н������ݵļ�飻����ֱ�ӷ��� true ���ɡ�
    // @return: ������������Ч������ true�����򷵻� false��
    virtual bool CheckSettingsValid() = 0;

    // ������ͨ���á���ͨ������ָ�������޹ص����á�
    // @return: ������ɹ������� true�����򷵻� false��
    virtual bool SaveNormalSettingsChange() = 0;

    // ��������������ã���ֻ���������������Ƿ�ı䡣
    // {check}: Ϊ true ʱֻ������������Ƿ�ı䣻Ϊ false ʱ�����������á�
    // @return: �� {check} Ϊ true ʱ�������������øı�ı�ʶ��Ϊ false ʱ
    //          ���ر���ɹ���ʧ�ܵı�ʶ���ɹ�Ϊ 1��ʧ��Ϊ 0��
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