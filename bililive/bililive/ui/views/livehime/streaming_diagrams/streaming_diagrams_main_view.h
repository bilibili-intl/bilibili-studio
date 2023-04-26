#pragma once

#include "base/observer_list.h"

#include "bililive/bililive/livehime/streaming_report/streaming_report_service.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"


namespace
{
    class StatisticsView;
    class DiagramAreaView;
    class StreamingSamplingNotifier;
}

class StreamingDiagramsMainView :
    public BililiveWidgetDelegate,
    StreamingReportObserver
{
public:
    static void ShowWindow();

protected:
    // View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;
    gfx::Size GetPreferredSize() override;
    void OnPaintBackground(gfx::Canvas* canvas) override;

    // WidgetDelegate
    void WindowClosing() override;

    // StreamingReportObserver
    void OnStreamingReportSampling(const StreamingReportInfo& info) override;

private:
    StreamingDiagramsMainView();
    ~StreamingDiagramsMainView();

    void InitViews();
    void UninitViews();

private:
    StatisticsView* sub_views_[StreamingSampleType::COUNT] = {};
    DiagramAreaView* left_view_ = nullptr;

    int show_live_status_ = 0;
    int64 start_show_ts_ = 0;
    scoped_ptr<StreamingSamplingNotifier> notifier_;

    base::WeakPtrFactory<StreamingDiagramsMainView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(StreamingDiagramsMainView);
};