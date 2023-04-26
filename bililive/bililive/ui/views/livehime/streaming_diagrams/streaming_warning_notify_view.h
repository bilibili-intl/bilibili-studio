#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_STREAMING_DIAGRAMS_STREAMING_WARNING_NOTIFY_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_STREAMING_DIAGRAMS_STREAMING_WARNING_NOTIFY_VIEW_H
#include "ui/views/view.h"
#include "ui/views/controls/button/button.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"

class StreamingWarningNotifyView
    : public BililiveWidgetDelegate
{
public:
    static void ShowWindow(const std::wstring& info);
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) override;
    gfx::Size GetPreferredSize() override;

    string16 GetWindowTitle() const override;
private:
    StreamingWarningNotifyView();
    ~StreamingWarningNotifyView();

    void InitViews();

    static void MoveDialog(views::Widget *widget);

    void AddInfo(const std::wstring& info);

    LivehimeContentLabel* label_ = nullptr;

    std::wstring info_;

    base::WeakPtrFactory<StreamingWarningNotifyView> weak_ptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(StreamingWarningNotifyView);
};

#endif //BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_STREAMING_DIAGRAMS_STREAMING_WARNING_NOTIFY_VIEW_H
