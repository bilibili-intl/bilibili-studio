// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "streaming_qmcs_window.h"

#include <string>

#include "base/file_util.h"
#include "base/observer_list.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/base/win/dpi.h"
#include "ui/gfx/canvas.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/scroll_view.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"

#include "bililive/bililive/livehime/streaming_report/streaming_qmcs_types.h"
#include "bililive/bililive/utils/time_span.h"
#include "bililive/public/protobuf/streaming_details.pb.h"

namespace
{
    class MouseOnView;

    int GetLengthByDPIScale(int len)
    {
        return std::round(len * ui::GetDPIScale());
    }

    const int kCellWidth = 20;
    const int kPixelPerSecond = 1;  // 横向时间轴，一个格子20秒，每秒1个像素；
    const int kCellCountPerRow = 5; // 纵向分值/占比/率，5个格子，每个格子20分，高度不固定
    const gfx::Size kMinDiagramSize(kCellWidth * 15, kCellWidth * 3);
    const size_t kMaxSampleCache = kMinDiagramSize.width() / kPixelPerSecond; // 各个图表缓存的采样数据的上限
    const SkColor kClrGridLine = SkColorSetRGB(217, 234, 244);
    const SkColor kClrDiagramLine = SkColorSetRGB(17, 125, 187);
    const SkColor kClrDiagramBk = SkColorSetA(SK_ColorLTGRAY, 100);
    const int64 kProcessMaxMemInMB = 4 * 1024;
    MouseOnView* g_mouse_on_view_single_instance = nullptr;
    enum CtrlID
    {
        LoadFile_Button = 1,
        LoadUrl_Button,
    };

    void MessageBox(HWND hwnd, const std::wstring& str)
    {
        ::MessageBox(hwnd, str.c_str(), L"提示", MB_OK);
    }

    class StreamingQMCSWindow;
    StreamingQMCSWindow* g_single_instance = nullptr;

    class StreamingSamplingNotifier;
    StreamingSamplingNotifier* g_notifier_instance = nullptr;

    gfx::Point g_mouse_pt_in_screen;
    bool g_mouse_entered = false;
    int64 g_last_ts = 0;

    class StreamingSamplingObserver
    {
    public:
        virtual void OnMouseMoved() {}
        virtual void OnMouseExited() {}

    protected:
        virtual ~StreamingSamplingObserver() = default;
    };

    class StreamingSamplingNotifier
    {
    public:
        StreamingSamplingNotifier()
        {
            g_notifier_instance = this;
        }

        ~StreamingSamplingNotifier()
        {
            g_notifier_instance = nullptr;
        }

        void AddObserver(StreamingSamplingObserver* observer) { observer_list_.AddObserver(observer); }
        void RemoveObserver(StreamingSamplingObserver* observer) { observer_list_.RemoveObserver(observer); }

        void OnMouseMoved()
        {
            FOR_EACH_OBSERVER(StreamingSamplingObserver, observer_list_, OnMouseMoved());
        }

        void OnMouseExited()
        {
            FOR_EACH_OBSERVER(StreamingSamplingObserver, observer_list_, OnMouseExited());
        }

    private:
        ObserverList<StreamingSamplingObserver> observer_list_;
    };

    // 图表视图
    class DiagramView : public views::View
    {
    public:
        DiagramView(StreamingSampleType dt)
            : dt_(dt)
        {
            set_border(views::Border::CreateSolidBorder(1, SkColorSetA(kClrDiagramLine, 128)));
        }

        void ChangeReport(std::vector<streaming_performance::SampleData>&& samples)
        {
            samples_ = std::move(samples);
        }

        void ChangeReportStartTimestamp(int64 ts)
        {
            start_show_ts_ = ts;
        }

        streaming_performance::SampleData GetHoverPointSampleData() const
        {
            if (!samples_.empty())
            {
                gfx::Point hover_pt = g_mouse_pt_in_screen;
                views::View::ConvertPointFromScreen(this, &hover_pt);

                // 鼠标hover点的采样时间戳
                int64 hover_ts = start_show_ts_ + hover_pt.x() / kPixelPerSecond;
                auto iter = std::find_if(samples_.begin(), samples_.end(),
                    [hover_ts](const streaming_performance::SampleData& sd)
                    {
                        if (sd.timestamp() == hover_ts)
                        {
                            return true;
                        }
                        return false;
                     }
                );
                // 计算鼠标点时间戳对应的采样数据
                if (iter != samples_.end())
                {
                    return (*iter);
                }
            }
            return {};
        }

    protected:
        // View
        gfx::Size GetPreferredSize() override
        {
            int64 duration = 0;
            if (!samples_.empty())
            {
                duration = std::max(0ll, samples_.back().timestamp() - start_show_ts_);
            }
            gfx::Size size(std::max(kMinDiagramSize.width(), (int)duration * kPixelPerSecond), kMinDiagramSize.height());
            return size;
        }

        void OnPaint(gfx::Canvas* canvas) override
        {
            OnPaintBackground(canvas);

            // 图表按照系统任务管理器性能图表的显示方式，从右往左移动
            SkPath path;
            std::vector<SkPoint> pts;
            if (!samples_.empty())
            {
                gfx::Rect vb = GetVisibleBounds();
                int x_oft = 0;
                for (auto iter = samples_.begin(); iter != samples_.end(); ++iter)
                {

                    // 底层采样点不可能做到严格按照采样时间间隔来采样，上层的绘制以两个采样点的实际时间间隔来进行
                    int64 duration = std::max(0ll, iter->timestamp() - start_show_ts_);
                    x_oft = duration * kPixelPerSecond;
                    double y_rate = 0;

                    if (x_oft < vb.x())
                    {
                        continue;
                    }
                    // 根据数据的类型换算成0~1.0
                    switch (dt_)
                    {
                    case StreamingSampleType::NDF:
                    case StreamingSampleType::EDF:
                    case StreamingSampleType::RDF:
                        {
                            // 底层传的是丢帧率/使用率
                            y_rate = iter->data();
                        }
                        break;
                    case StreamingSampleType::GPU:
                    case StreamingSampleType::CPU:
                    case StreamingSampleType::MEM:
                    case StreamingSampleType::P_CPU:
                        {
                            // 底层传的是比例整数值
                            y_rate = iter->data() / 100;
                        }
                        break;
                    case StreamingSampleType::P_MEM:
                        {
                            // 底层传来的是MB，以4GB为分母
                            y_rate = iter->data() / kProcessMaxMemInMB;
                        }
                        break;
                    default:
                        break;
                    }

                    pts.push_back(SkPoint::Make(x_oft, std::max<int>(1, height() * (1.0f - y_rate))));

                    // 如果已经画出边缘就不要再画了
                    if (x_oft > vb.right())
                    {
                        break;
                    }
                }

                // 为图表两端添加一个到底部X轴的封闭点
                //pts.push_back(SkPoint::Make(0, height()));
                pts.push_back(SkPoint::Make(x_oft, height()));

                path.addPoly(&pts[0], pts.size(), false);
            }

            // 竖线
            for (int x = 0; x < width(); x += kCellWidth)
            {
                canvas->DrawLine(gfx::Point(x, 0), gfx::Point(x, height()), kClrGridLine);
            }
            // 横线
            int y_spacing = height() / kCellCountPerRow;
            for (int y = 0; y < height(); y += y_spacing)
            {
                canvas->DrawLine(gfx::Point(0, y), gfx::Point(width(), y), kClrGridLine);
            }

            // 折线图
            if (!path.isEmpty())
            {
                // 折线区底色
                SkPaint paint;
                paint.setColor(kClrDiagramBk);
                //canvas->DrawPath(path, paint);

                // 折线
                paint.setColor(kClrDiagramLine);
                paint.setStrokeWidth(1);
                paint.setStyle(SkPaint::kStroke_Style);
                canvas->DrawPath(path, paint);
            }

            // 画鼠标hover时的统一时间纵轴线
            if (g_mouse_entered)
            {
                gfx::Point hover_pt = g_mouse_pt_in_screen;
                views::View::ConvertPointFromScreen(this, &hover_pt);

                canvas->DrawLine(gfx::Point(hover_pt.x(), 0), gfx::Point(hover_pt.x(), height()), SK_ColorBLUE);
            }

            OnPaintFocusBorder(canvas);
            OnPaintBorder(canvas);
        }

    private:
        StreamingSampleType dt_;
        int64 start_show_ts_ = 0;
        std::vector<streaming_performance::SampleData> samples_;
    };

    // 单个数据项视图
    class StatisticsView :
        public views::View,
        public StreamingSamplingObserver
    {
    public:
        StatisticsView(StreamingSampleType dt)
            : dt_(dt)
        {
            set_notify_enter_exit_on_child(true);

            views::GridLayout* layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            auto columnset = layout->AddColumnSet(0);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

            columnset = layout->AddColumnSet(1);
            columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

            static const std::map<StreamingSampleType, std::wstring> kTypeStr{
                {StreamingSampleType::CPU, L"CPU使用率："},
                {StreamingSampleType::GPU, L"GPU使用率："},
                {StreamingSampleType::MEM, L"内存使用率："},
                {StreamingSampleType::P_CPU, L"直播姬CPU使用率："},
                {StreamingSampleType::P_MEM, L"直播姬内存使用："},
                {StreamingSampleType::NDF, L"网络丢帧："},
                {StreamingSampleType::EDF, L"编码丢帧："},
                {StreamingSampleType::RDF, L"渲染丢帧："},
            };
            views::Label* label = new views::Label(kTypeStr.at(dt));

            instantaneous_label_ = new views::Label();
            instantaneous_label_->SetHorizontalAlignment(gfx::ALIGN_RIGHT);

            layout->StartRowWithPadding(0, 0, 0, 0);
            layout->AddView(label);
            layout->AddView(instantaneous_label_);

            layout->StartRowWithPadding(0, 1, 0, 6);
            diagram_view_ = new DiagramView(dt);
            layout->AddView(diagram_view_);
        }

        std::wstring GetLabText()
        {
            if (instantaneous_label_)
                return instantaneous_label_->text();
            return L"";
        }

        void ChangeReport(std::vector<streaming_performance::SampleData>&& samples)
        {
            diagram_view_->ChangeReport(std::move(samples));
        }

        void ChangeReportStartTimestamp(int64 ts)
        {
            diagram_view_->ChangeReportStartTimestamp(ts);
        }

    protected:
        // View
        void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override
        {
            if (details.child == this)
            {
                if (details.is_add)
                {
                    g_notifier_instance->AddObserver(this);
                }
                else
                {
                    //g_notifier_instance->RemoveObserver(this);
                }
            }
        }

        const char* GetClassName() const override
        {
            return "StatisticsView";
        }

        // StreamingSamplingObserver
        void OnMouseMoved() override
        {
            OnSampling();
        }

        void OnMouseExited() override
        {
            OnSampling();
        }

    private:
        void OnSampling()
        {
            // 鼠标hover时显示鼠标hover点的数据，否则显示最新的采样数据
            streaming_performance::SampleData sample_data;
            if (g_mouse_entered)
            {
                sample_data = diagram_view_->GetHoverPointSampleData();
                UpdateHoverInfo(sample_data, true);
            }
            else
            {
                UpdateHoverInfo({}, false);
            }
        }

        void UpdateHoverInfo(const streaming_performance::SampleData& sample_data, bool instantaneous)
        {
            if (!instantaneous)
            {
                instantaneous_label_->SetText(L"");
                return;
            }

            base::string16 str;
            switch (dt_)
            {
            case StreamingSampleType::NDF:
            case StreamingSampleType::EDF:
            case StreamingSampleType::RDF:
                {
                    // 底层传的是丢帧率/使用率
                    str = base::StringPrintf(L"%.0f%%", sample_data.data() * 100);
                }
                break;
            case StreamingSampleType::GPU:
            case StreamingSampleType::CPU:
            case StreamingSampleType::MEM:
            case StreamingSampleType::P_CPU:
                {
                    // 底层传的是比例整数值
                    str = base::StringPrintf(L"%d%%", (int)sample_data.data());
                }
                break;
            case StreamingSampleType::P_MEM:
                {
                    // 底层传来的是MB，以4GB为分母
                    str = base::StringPrintf(L"%dMB", (int)sample_data.data());
                }
                break;
            default:
                break;
            }

            str += (sample_data.timestamp() == 0) ? L"(采样缺失)" : L"";

            instantaneous_label_->SetText(str);
        }

    private:
        StreamingSampleType dt_;

        views::Label* instantaneous_label_ = nullptr;
        DiagramView* diagram_view_ = nullptr;
    };


    //鼠标移上去弹窗
    class MouseOnView : public views::WidgetDelegateView
    {
    public:
        static void ShowWindow()
        {
            if (!g_mouse_on_view_single_instance)
            {
                views::Widget::CreateWindowWithBounds(new MouseOnView(),
                    gfx::Rect(0, 0, 300, 300))->Show();
            }
        }

        void SetText(const std::wstring& time, std::wstring text[StreamingSampleType::COUNT])
        {
            time_label_->SetText(time);
            for (int i = 0; i < StreamingSampleType::COUNT; i++)
            {
                item_views_[i]->SetText(text[i]);
            }

            Layout();
        }
    protected:
        // Overridden from View:
        void ViewHierarchyChanged(
            const views::View::ViewHierarchyChangedDetails& details) OVERRIDE
        {
            if (details.is_add && details.child == this)
            {
                InitViews();
            }
        }

        View* GetContentsView() OVERRIDE { return this; }

    private:
        MouseOnView()
        {
            g_mouse_on_view_single_instance = this;
        }
        ~MouseOnView()
        {
            g_mouse_on_view_single_instance = nullptr;
        }

        void InitViews()
        {
            set_background(views::Background::CreateSolidBackground(SK_ColorWHITE));
            views::GridLayout* layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            auto columnset = layout->AddColumnSet(0);
            columnset->AddPaddingColumn(0, 6);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

            time_label_ = new views::Label();

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(time_label_);

            for (int i = 0; i < StreamingSampleType::COUNT; i++)
            {
                item_views_[i] = new views::Label();

                layout->StartRowWithPadding(0, 0, 0, 12);
                layout->AddView(item_views_[i]);
            }
        }

        views::Label* item_views_[StreamingSampleType::COUNT] = {};
        views::Label* time_label_ = nullptr;
    };

    // 右侧图表区
    class DiagramAreaView : public views::View
    {
    public:
        DiagramAreaView()
        {
            set_notify_enter_exit_on_child(true);
        }

        void ChangeReport(streaming_performance::PerformanceReport* pb_reader)
        {
            // 把各种类型的数据解析出来
            if (!pb_reader)
            {
                for (int i = 0; i < StreamingSampleType::COUNT; i++)
                {
                    sub_views_[i]->ChangeReport({});
                }

                return;
            }

            // 所有类型的采样数据中最早产生数据的时间点
            start_show_ts_ = std::numeric_limits<int64>::max();
            std::vector<streaming_performance::SampleData> samples;

            samples.assign(pb_reader->statistics().ndf().data().samples().begin(),
                pb_reader->statistics().ndf().data().samples().end());
            if (!samples.empty()) start_show_ts_ = std::min(start_show_ts_, samples.front().timestamp());
            sub_views_[StreamingSampleType::NDF]->ChangeReport(std::move(samples));

            samples.assign(pb_reader->statistics().rdf().data().samples().begin(),
                pb_reader->statistics().rdf().data().samples().end());
            if (!samples.empty()) start_show_ts_ = std::min(start_show_ts_, samples.front().timestamp());
            sub_views_[StreamingSampleType::RDF]->ChangeReport(std::move(samples));

            samples.assign(pb_reader->statistics().edf().data().samples().begin(),
                pb_reader->statistics().edf().data().samples().end());
            if (!samples.empty()) start_show_ts_ = std::min(start_show_ts_, samples.front().timestamp());
            sub_views_[StreamingSampleType::EDF]->ChangeReport(std::move(samples));

            samples.assign(pb_reader->statistics().gpu().data().samples().begin(),
                pb_reader->statistics().gpu().data().samples().end());
            if (!samples.empty()) start_show_ts_ = std::min(start_show_ts_, samples.front().timestamp());
            sub_views_[StreamingSampleType::GPU]->ChangeReport(std::move(samples));

            samples.assign(pb_reader->statistics().cpu().data().samples().begin(),
                pb_reader->statistics().cpu().data().samples().end());
            if (!samples.empty()) start_show_ts_ = std::min(start_show_ts_, samples.front().timestamp());
            sub_views_[StreamingSampleType::CPU]->ChangeReport(std::move(samples));

            samples.assign(pb_reader->statistics().mem().data().samples().begin(),
                pb_reader->statistics().mem().data().samples().end());
            if (!samples.empty()) start_show_ts_ = std::min(start_show_ts_, samples.front().timestamp());
            sub_views_[StreamingSampleType::MEM]->ChangeReport(std::move(samples));

            samples.assign(pb_reader->statistics().p_cpu().data().samples().begin(),
                pb_reader->statistics().p_cpu().data().samples().end());
            if (!samples.empty()) start_show_ts_ = std::min(start_show_ts_, samples.front().timestamp());
            sub_views_[StreamingSampleType::P_CPU]->ChangeReport(std::move(samples));

            samples.assign(pb_reader->statistics().p_mem().data().samples().begin(),
                pb_reader->statistics().p_mem().data().samples().end());
            if (!samples.empty()) start_show_ts_ = std::min(start_show_ts_, samples.front().timestamp());
            sub_views_[StreamingSampleType::P_MEM]->ChangeReport(std::move(samples));

            // 向所有图表设置初始时间戳，以便于全部图表的时间轴对齐
            for (int i = 0; i < StreamingSampleType::COUNT; i++)
            {
                sub_views_[i]->ChangeReportStartTimestamp(start_show_ts_);
            }

            start_time_label_->SetText(bililive::TimeStampToString(start_show_ts_));
            OnBoundsChanged(bounds());
        }

    protected:
        // View
        void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override
        {
            if (details.child == this)
            {
                if (details.is_add)
                {
                    InitViews();
                }
            }
        }

        void OnMouseEntered(const ui::MouseEvent& event)
        {
            g_mouse_entered = true;
        }

        void OnMouseMoved(const ui::MouseEvent& event)
        {
            g_mouse_pt_in_screen = event.location();
            views::View::ConvertPointToScreen(this, &g_mouse_pt_in_screen);

            UpdateHoverInfo(event.location());

            // 发通知给其他需要展示鼠标采样点数据的地方
            g_notifier_instance->OnMouseMoved();

            SchedulePaint();

            UpdateMouseOnViewInfo(event.location());
        }

        void OnMouseExited(const ui::MouseEvent& event)
        {
            g_mouse_entered = false;
            time_label_->SetText(L"");

            // 发通知给其他需要展示鼠标采样点数据的地方
            g_notifier_instance->OnMouseMoved();

            SchedulePaint();
        }

        void OnBoundsChanged(const gfx::Rect& previous_bounds) override
        {
            if (start_show_ts_ != std::numeric_limits<int64>::max())
            {
                int duration = width() / kPixelPerSecond;
                total_time_label_->SetText(bililive::TimeStampToString(start_show_ts_ + duration));
            }
        }

        views::View* GetEventHandlerForPoint(const gfx::Point& point) override
        {
            return this;
        }

    private:
        void InitViews()
        {
            views::GridLayout* layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            auto columnset = layout->AddColumnSet(0);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            columnset->AddPaddingColumn(0, 6);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

            columnset = layout->AddColumnSet(1);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
            columnset->AddColumn(views::GridLayout::TRAILING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

            columnset = layout->AddColumnSet(2);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

            views::Label* tl = new views::Label(L"各项性能指标");
            //tl->SetFont(ftPrimaryBold);

            views::Label* dl = new views::Label(L"(全时段各项性能指标变化情况)");

            layout->StartRowWithPadding(0, 0, 0, 0);
            layout->AddView(tl);
            layout->AddView(dl);

            start_time_label_ = new views::Label(L"--:--");
            time_label_ = new views::Label();
            total_time_label_ = new views::Label();

            layout->StartRowWithPadding(0, 1, 0, 6);
            layout->AddView(start_time_label_);
            layout->AddView(time_label_);
            layout->AddView(total_time_label_);

            for (int i = 0; i < StreamingSampleType::COUNT; i++)
            {
                sub_views_[i] = new StatisticsView((StreamingSampleType)i);

                layout->StartRowWithPadding(0, 2, 0, 12);
                layout->AddView(sub_views_[i]);
            }
        }

        void UpdateHoverInfo(const gfx::Point& pt)
        {
            // 计算鼠标点相较右侧的距离应该对应哪个采样点，计算出当前鼠标hover的时间点
            int duration = pt.x() / kPixelPerSecond;
            base::Time::Exploded exploded = { 0 };
            base::Time::FromDoubleT(start_show_ts_ + duration).LocalExplode(&exploded);
            time_label_->SetText(base::StringPrintf(L"- %02d:%02d:%02d -", exploded.hour, exploded.minute, exploded.second));
        }

        void UpdateMouseOnViewInfo(const gfx::Point& pt)
        {
            MouseOnView::ShowWindow();
            if (g_mouse_on_view_single_instance)
            {
                int duration = pt.x() / kPixelPerSecond;
                base::Time::Exploded exploded = { 0 };
                base::Time::FromDoubleT(start_show_ts_ + duration).LocalExplode(&exploded);
                std::wstring time = base::StringPrintf(L"时间：%02d:%02d:%02d", exploded.hour, exploded.minute, exploded.second);

                std::wstring text[StreamingSampleType::COUNT]{ L"网络丢帧：",L"渲染丢帧：",L"编码丢帧：",
                L"GPU使用率：",
                L"CPU使用率：",
                L"内存使用率：",
                L"直播姬CPU使用率：",
                L"直播姬内存使用率：" };

                for (int i = 0; i < StreamingSampleType::COUNT; i++)
                {
                    text[i] += sub_views_[i]->GetLabText();
                }
                g_mouse_on_view_single_instance->SetText(time, text);
            }
        }
    private:
        views::Label* start_time_label_ = nullptr;
        views::Label* time_label_ = nullptr;
        views::Label* total_time_label_ = nullptr;
        StatisticsView* sub_views_[StreamingSampleType::COUNT] = {};
        int64 start_show_ts_ = std::numeric_limits<int64>::max();
    };

    // 左侧信息区
    class DetailsInfoAreaView : public views::View
    {
        enum LabelType
        {
            SYSVER,
            CPU,
            GRAPHICS,
            CUR_GRAPHICS,
            MEM,

            NETWORK,
            IP,
            REGION,
            CO,

            VERSION,
            ROOMID,
            UID,

            BITRATE,
            BITRATETYPE,
            FPS,
            RESOLUTION,
            ENCODER,
            QUALITY,

            LIVE_STATUS,
            LIVE_START_TIME,
            STREAM_NAME,

            LOGVER,
            SAMPLERATE,
            ACCOUNT,
            LIVE_END_TIME,

            COUNT,
        };

    public:
        DetailsInfoAreaView()
            : weakptr_factory_(this)
        {
            set_border(views::Border::CreateSolidSidedBorder(0, 0, 0, 1, SkColorSetRGB(0xd7, 0xd7, 0xd7)));
        }

        void ChangeReport(streaming_performance::PerformanceReport* pb_reader)
        {
            if (!pb_reader)
            {
                for (int i = 0; i < LabelType::COUNT; i++)
                {
                    labels_[i]->SetText(L"");
                }

                return;
            }

            // 报告信息
            labels_[LabelType::LOGVER]->SetText(std::to_wstring(pb_reader->logver()));
            labels_[LabelType::SAMPLERATE]->SetText(std::to_wstring(pb_reader->sampling()));

            // 系统配置
            labels_[LabelType::SYSVER]->SetText(base::UTF8ToUTF16(pb_reader->app_info().sys_info().system_ver()));
            labels_[LabelType::CPU]->SetText(base::UTF8ToUTF16(pb_reader->app_info().sys_info().cpu()));
            // gpu
            std::vector<base::string16> gpus;
            for (int i = 0; i < pb_reader->app_info().sys_info().graphic().size(); i++)
            {
                gpus.push_back(base::UTF8ToUTF16(pb_reader->app_info().sys_info().graphic(i)));
            }
            labels_[LabelType::GRAPHICS]->SetText(JoinString(gpus, '\n'));
            labels_[LabelType::CUR_GRAPHICS]->SetText(base::UTF8ToUTF16(pb_reader->app_info().sys_info().load_graphic()));
            // 内存
            labels_[LabelType::MEM]->SetText(base::UTF8ToUTF16(pb_reader->app_info().sys_info().memory()));
            // 地址
            switch (pb_reader->app_info().sys_info().network())
            {
            case streaming_performance::RuntimeNetWork::ETHERNET:
                labels_[LabelType::NETWORK]->SetText(L"网线连接");
                break;
            case streaming_performance::RuntimeNetWork::WIFI:
                labels_[LabelType::NETWORK]->SetText(L"WIFI连接");
                break;
            default:
                NOTREACHED();
                labels_[LabelType::NETWORK]->SetText(L"网线连接");
                break;
            }
            labels_[LabelType::IP]->SetText(base::UTF8ToUTF16(pb_reader->app_info().sys_info().ip()));
            labels_[LabelType::REGION]->SetText(base::UTF8ToUTF16(pb_reader->app_info().sys_info().region()));
            labels_[LabelType::CO]->SetText(base::UTF8ToUTF16(pb_reader->app_info().sys_info().co()));

            // 程序信息
            labels_[LabelType::VERSION]->SetText(base::UTF8ToUTF16(pb_reader->app_info().version()));
            labels_[LabelType::ROOMID]->SetText(std::to_wstring(pb_reader->app_info().roomid()));
            labels_[LabelType::UID]->SetText(std::to_wstring(pb_reader->app_info().uid()));
            labels_[LabelType::ACCOUNT]->SetText(base::UTF8ToUTF16(pb_reader->app_info().account()));

            // 推流信息
            labels_[LabelType::LIVE_STATUS]->SetText((pb_reader->runtime_info().live_status() == 0) ?
                L"闲置" : L"直播中");
            labels_[LabelType::LIVE_START_TIME]->SetText(
                bililive::TimeFormatFriendlyDate(base::Time::FromDoubleT(pb_reader->runtime_info().start_time())));
            labels_[LabelType::LIVE_END_TIME]->SetText(
                bililive::TimeFormatFriendlyDate(base::Time::FromDoubleT(pb_reader->runtime_info().end_time())));
            labels_[LabelType::STREAM_NAME]->SetText(base::UTF8ToUTF16(pb_reader->runtime_info().code().substr(1)));

            // 视频设置
            labels_[LabelType::BITRATE]->SetText(std::to_wstring(pb_reader->runtime_info().video_settings(0).bit_rate()));
            labels_[LabelType::BITRATETYPE]->SetText(base::UTF8ToUTF16(pb_reader->runtime_info().video_settings(0).rate_type()));
            labels_[LabelType::FPS]->SetText(base::UTF8ToUTF16(pb_reader->runtime_info().video_settings(0).fps()));
            labels_[LabelType::RESOLUTION]->SetText(base::UTF8ToUTF16(pb_reader->runtime_info().video_settings(0).resolution()));
            labels_[LabelType::ENCODER]->SetText(base::UTF8ToUTF16(pb_reader->runtime_info().video_settings(0).encoder()));
            labels_[LabelType::QUALITY]->SetText(base::UTF8ToUTF16(pb_reader->runtime_info().video_settings(0).quality()));
        }

    protected:
        // View
        void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override
        {
            if (details.child == this)
            {
                if (details.is_add)
                {
                    InitViews();
                }
            }
        }

        gfx::Size GetPreferredSize() override
        {
            gfx::Size size = __super::GetPreferredSize();
            size.set_width(std::max(size.width(), GetLengthByDPIScale(350)));
            return size;
        }

    private:
        void InitViews()
        {
            for (int i = 0; i < LabelType::COUNT; i++)
            {
                labels_[i] = new views::Label();
                labels_[i]->SetHorizontalAlignment(gfx::ALIGN_LEFT);
            }
            labels_[LabelType::SYSVER]->SetMultiLine(true);
            labels_[LabelType::CPU]->SetMultiLine(true);
            labels_[LabelType::GRAPHICS]->SetMultiLine(true);
            labels_[LabelType::STREAM_NAME]->SetMultiLine(true);

            views::GridLayout* layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            auto columnset = layout->AddColumnSet(0);
            columnset->AddPaddingColumn(0, 12);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            columnset->AddPaddingColumn(0, 6);
            columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

            columnset = layout->AddColumnSet(1);
            columnset->AddPaddingColumn(0, 12);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

            // 报告信息
            views::Label* gl = new views::Label(L"报告信息");
            //gl->SetFont(ftPrimaryBold);
            layout->StartRowWithPadding(0, 1, 0, 0);
            layout->AddView(gl);

            layout->StartRowWithPadding(0, 0, 0, 12);
            layout->AddView(new views::Label(L"报告版本："));
            layout->AddView(labels_[LabelType::LOGVER]);

            layout->StartRowWithPadding(0, 0, 0, 12);
            layout->AddView(new views::Label(L"采样率："));
            layout->AddView(labels_[LabelType::SAMPLERATE]);

            // 系统信息
            gl = new views::Label(L"系统信息");
            //gl->SetFont(ftPrimaryBold);
            layout->StartRowWithPadding(0, 1, 0, 12);
            layout->AddView(gl);

            layout->StartRowWithPadding(0, 0, 0, 12);
            layout->AddView(new views::Label(L"系统版本："), 1, 1, views::GridLayout::LEADING, views::GridLayout::LEADING, 0, 0);
            layout->AddView(labels_[LabelType::SYSVER]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"CPU："), 1, 1, views::GridLayout::LEADING, views::GridLayout::LEADING, 0, 0);
            layout->AddView(labels_[LabelType::CPU]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"显卡："), 1, 1, views::GridLayout::LEADING, views::GridLayout::LEADING, 0, 0);
            layout->AddView(labels_[LabelType::GRAPHICS]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"程序使用显卡："));
            layout->AddView(labels_[LabelType::CUR_GRAPHICS]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"内存："));
            layout->AddView(labels_[LabelType::MEM]);

            // 网络信息
            gl = new views::Label(L"网络信息");
            //gl->SetFont(ftPrimaryBold);
            layout->StartRowWithPadding(0, 1, 0, 12);
            layout->AddView(gl);

            layout->StartRowWithPadding(0, 0, 0, 12);
            layout->AddView(new views::Label(L"连接方式："));
            layout->AddView(labels_[LabelType::NETWORK]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"以太网IP："));
            layout->AddView(labels_[LabelType::IP]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"地区："));
            layout->AddView(labels_[LabelType::REGION]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"网络运营商："));
            layout->AddView(labels_[LabelType::CO]);

            // 程序信息
            gl = new views::Label(L"程序信息");
            //gl->SetFont(ftPrimaryBold);
            layout->StartRowWithPadding(0, 1, 0, 12);
            layout->AddView(gl);

            layout->StartRowWithPadding(0, 0, 0, 12);
            layout->AddView(new views::Label(L"程序版本："));
            layout->AddView(labels_[LabelType::VERSION]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"房间号："));
            layout->AddView(labels_[LabelType::ROOMID]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"用户UID："));
            layout->AddView(labels_[LabelType::UID]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"用户账号："));
            layout->AddView(labels_[LabelType::ACCOUNT]);

            // 视频设置
            gl = new views::Label(L"编码参数");
            //gl->SetFont(ftPrimaryBold);
            layout->StartRowWithPadding(0, 1, 0, 12);
            layout->AddView(gl);

            layout->StartRowWithPadding(0, 0, 0, 12);
            layout->AddView(new views::Label(L"码率："));
            layout->AddView(labels_[LabelType::BITRATE]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"码率控制："));
            layout->AddView(labels_[LabelType::BITRATETYPE]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"帧率："));
            layout->AddView(labels_[LabelType::FPS]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"分辨率："));
            layout->AddView(labels_[LabelType::RESOLUTION]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"编码器："));
            layout->AddView(labels_[LabelType::ENCODER]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"性能平衡："));
            layout->AddView(labels_[LabelType::QUALITY]);

            // 开播相关参数
            gl = new views::Label(L"推流状态");
            //gl->SetFont(ftPrimaryBold);
            layout->StartRowWithPadding(0, 1, 0, 12);
            layout->AddView(gl);

            layout->StartRowWithPadding(0, 0, 0, 12);
            layout->AddView(new views::Label(L"开播状态："));
            layout->AddView(labels_[LabelType::LIVE_STATUS]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"推流开始时间："));
            layout->AddView(labels_[LabelType::LIVE_START_TIME]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"推流结束时间："));
            layout->AddView(labels_[LabelType::LIVE_END_TIME]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"直播码："), 1, 1, views::GridLayout::LEADING, views::GridLayout::LEADING, 0, 0);
            layout->AddView(labels_[LabelType::STREAM_NAME]);
        }

    private:
        views::Label* labels_[LabelType::COUNT] = {};

        base::WeakPtrFactory<DetailsInfoAreaView> weakptr_factory_;
    };

    // 报告全内容视图
    class QMCSContentView : public views::View
    {
    public:
        QMCSContentView()
        {
        }

    protected:
        // View
        void Layout() override
        {
            gfx::Size pref_size = GetPreferredSize();
            SetSize(pref_size);

            __super::Layout();
        }

    private:
    };

    // 主窗口
    class StreamingQMCSWindow :
        public views::WidgetDelegateView,
        public views::ButtonListener
    {
    public:
        StreamingQMCSWindow()
        {
            g_single_instance = this;

            notifier_.reset(new StreamingSamplingNotifier());
        }

        virtual ~StreamingQMCSWindow()
        {
            g_single_instance = nullptr;
        }

    protected:
        // Overridden from WidgetDelegateView:
        bool CanResize() const OVERRIDE { return true; }
        bool CanMaximize() const OVERRIDE { return true; }
        string16 GetWindowTitle() const OVERRIDE
        {
            return window_title_;
        }
        View* GetContentsView() OVERRIDE { return this; }
        void WindowClosing() OVERRIDE
        {
            base::MessageLoopForUI::current()->Quit();
        }

        // Overridden from View:
        void ViewHierarchyChanged(
            const views::View::ViewHierarchyChangedDetails& details) OVERRIDE
        {
            if (details.is_add && details.child == this)
            {
                InitViews();
            }
        }

        // ButtonListener
        void ButtonPressed(views::Button* sender, const ui::Event& event) override
        {
            base::string16 text = source_textfield_->text();
            if (!text.empty())
            {
                switch (sender->id())
                {
                case LoadFile_Button:
                    {
                        std::string data;
                        base::FilePath path(text);
                        if (base::PathExists(path) && !base::DirectoryExists(path) &&
                            file_util::ReadFileToString(path, &data))
                        {
                            pb_reader_.reset(new streaming_performance::PerformanceReport);
                            if (pb_reader_->ParseFromString(data))
                            {
                                window_title_ = L"推流质量监控 - " + path.BaseName().value();

                                // 图表区和信息区更新数据展示
                                info_view_->ChangeReport(pb_reader_.get());
                                diagram_view_->ChangeReport(pb_reader_.get());
                                Layout();
                            }
                            else
                            {
                                MessageBox(GetWidget()->GetNativeWindow(), L"无效的protobuf报告文件");
                            }
                        }
                        else
                        {
                            MessageBox(GetWidget()->GetNativeWindow(), L"请提供有效的文件路径");
                        }
                    }
                    break;
                case LoadUrl_Button:
                    break;
                default:
                    break;
                }
            }
            else
            {
                window_title_ = L"推流质量监控";
            }
            GetWidget()->UpdateWindowTitle();
        }

    private:
        void InitViews()
        {
            set_background(views::Background::CreateSolidBackground(SK_ColorWHITE));

            views::GridLayout* layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            auto columnset = layout->AddColumnSet(0);
            columnset->AddPaddingColumn(0, 12);
            columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

            columnset = layout->AddColumnSet(1);
            columnset->AddPaddingColumn(0, 12);
            columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            columnset->AddPaddingColumn(0, 6);
            columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

            source_textfield_ = new views::Textfield();
            source_textfield_->set_placeholder_text(L"粘贴文件全路径或URL（当前只支持本地文件，你先自行从BFS下载下来吧）");

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(source_textfield_);

            views::LabelButton* file_button = new views::LabelButton(this, L"加载文件");
            file_button->set_id(LoadFile_Button);
            file_button->SetIsDefault(true);
            views::LabelButton* url_button = new views::LabelButton(this, L"加载URL");
            url_button->set_id(LoadUrl_Button);
            url_button->SetEnabled(false);

            layout->StartRowWithPadding(0, 1, 0, 6);
            layout->AddView(file_button);
            layout->AddView(url_button);

            views::ScrollView* scroll_view = new views::ScrollView();
            {
                QMCSContentView* scroll_content_view = new QMCSContentView();
                scroll_view->SetContents(scroll_content_view);

                auto layout = new views::BoxLayout(views::BoxLayout::kHorizontal,
                    12, 12, 12);
                layout->set_spread_blank_space(true);
                scroll_content_view->SetLayoutManager(layout);

                info_view_ = new DetailsInfoAreaView();
                diagram_view_ = new DiagramAreaView();

                scroll_content_view->AddChildView(info_view_);
                scroll_content_view->AddChildView(diagram_view_);

                //scroll_view->set_border(views::Border::CreateSolidBorder(1, SK_ColorRED));
                //scroll_content_view->set_border(views::Border::CreateSolidBorder(1, SK_ColorBLACK));
            }

            layout->StartRowWithPadding(1.0f, 0, 0, 6);
            layout->AddView(scroll_view);
        }

    private:
        base::string16 window_title_ = L"推流质量监控";
        views::Textfield* source_textfield_ = nullptr;

        DetailsInfoAreaView* info_view_ = nullptr;
        DiagramAreaView* diagram_view_ = nullptr;

        scoped_ptr<StreamingSamplingNotifier> notifier_;

        std::unique_ptr<streaming_performance::PerformanceReport> pb_reader_;

        DISALLOW_COPY_AND_ASSIGN(StreamingQMCSWindow);
    };

}

namespace livehime
{
    namespace streaming_qmcs
    {
        void ShowStreamingQMCSWindow()
        {
            if (g_single_instance)
            {
                g_single_instance->GetWidget()->Activate();
            }
            else
            {
                views::Widget::CreateWindowWithBounds(new StreamingQMCSWindow(),
                    gfx::Rect(0, 0, 850, 950))->Show();
            }
        }
    }
}
