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
    const int kPixelPerSecond = 1;  // ����ʱ���ᣬһ������20�룬ÿ��1�����أ�
    const int kCellCountPerRow = 5; // �����ֵ/ռ��/�ʣ�5�����ӣ�ÿ������20�֣��߶Ȳ��̶�
    const gfx::Size kMinDiagramSize(kCellWidth * 15, kCellWidth * 3);
    const size_t kMaxSampleCache = kMinDiagramSize.width() / kPixelPerSecond; // ����ͼ����Ĳ������ݵ�����
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
        ::MessageBox(hwnd, str.c_str(), L"��ʾ", MB_OK);
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

    // ͼ����ͼ
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

                // ���hover��Ĳ���ʱ���
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
                // ��������ʱ�����Ӧ�Ĳ�������
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

            // ͼ����ϵͳ�������������ͼ�����ʾ��ʽ�����������ƶ�
            SkPath path;
            std::vector<SkPoint> pts;
            if (!samples_.empty())
            {
                gfx::Rect vb = GetVisibleBounds();
                int x_oft = 0;
                for (auto iter = samples_.begin(); iter != samples_.end(); ++iter)
                {

                    // �ײ�����㲻���������ϸ��ղ���ʱ�������������ϲ�Ļ����������������ʵ��ʱ����������
                    int64 duration = std::max(0ll, iter->timestamp() - start_show_ts_);
                    x_oft = duration * kPixelPerSecond;
                    double y_rate = 0;

                    if (x_oft < vb.x())
                    {
                        continue;
                    }
                    // �������ݵ����ͻ����0~1.0
                    switch (dt_)
                    {
                    case StreamingSampleType::NDF:
                    case StreamingSampleType::EDF:
                    case StreamingSampleType::RDF:
                        {
                            // �ײ㴫���Ƕ�֡��/ʹ����
                            y_rate = iter->data();
                        }
                        break;
                    case StreamingSampleType::GPU:
                    case StreamingSampleType::CPU:
                    case StreamingSampleType::MEM:
                    case StreamingSampleType::P_CPU:
                        {
                            // �ײ㴫���Ǳ�������ֵ
                            y_rate = iter->data() / 100;
                        }
                        break;
                    case StreamingSampleType::P_MEM:
                        {
                            // �ײ㴫������MB����4GBΪ��ĸ
                            y_rate = iter->data() / kProcessMaxMemInMB;
                        }
                        break;
                    default:
                        break;
                    }

                    pts.push_back(SkPoint::Make(x_oft, std::max<int>(1, height() * (1.0f - y_rate))));

                    // ����Ѿ�������Ե�Ͳ�Ҫ�ٻ���
                    if (x_oft > vb.right())
                    {
                        break;
                    }
                }

                // Ϊͼ���������һ�����ײ�X��ķ�յ�
                //pts.push_back(SkPoint::Make(0, height()));
                pts.push_back(SkPoint::Make(x_oft, height()));

                path.addPoly(&pts[0], pts.size(), false);
            }

            // ����
            for (int x = 0; x < width(); x += kCellWidth)
            {
                canvas->DrawLine(gfx::Point(x, 0), gfx::Point(x, height()), kClrGridLine);
            }
            // ����
            int y_spacing = height() / kCellCountPerRow;
            for (int y = 0; y < height(); y += y_spacing)
            {
                canvas->DrawLine(gfx::Point(0, y), gfx::Point(width(), y), kClrGridLine);
            }

            // ����ͼ
            if (!path.isEmpty())
            {
                // ��������ɫ
                SkPaint paint;
                paint.setColor(kClrDiagramBk);
                //canvas->DrawPath(path, paint);

                // ����
                paint.setColor(kClrDiagramLine);
                paint.setStrokeWidth(1);
                paint.setStyle(SkPaint::kStroke_Style);
                canvas->DrawPath(path, paint);
            }

            // �����hoverʱ��ͳһʱ��������
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

    // ������������ͼ
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
                {StreamingSampleType::CPU, L"CPUʹ���ʣ�"},
                {StreamingSampleType::GPU, L"GPUʹ���ʣ�"},
                {StreamingSampleType::MEM, L"�ڴ�ʹ���ʣ�"},
                {StreamingSampleType::P_CPU, L"ֱ����CPUʹ���ʣ�"},
                {StreamingSampleType::P_MEM, L"ֱ�����ڴ�ʹ�ã�"},
                {StreamingSampleType::NDF, L"���綪֡��"},
                {StreamingSampleType::EDF, L"���붪֡��"},
                {StreamingSampleType::RDF, L"��Ⱦ��֡��"},
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
            // ���hoverʱ��ʾ���hover������ݣ�������ʾ���µĲ�������
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
                    // �ײ㴫���Ƕ�֡��/ʹ����
                    str = base::StringPrintf(L"%.0f%%", sample_data.data() * 100);
                }
                break;
            case StreamingSampleType::GPU:
            case StreamingSampleType::CPU:
            case StreamingSampleType::MEM:
            case StreamingSampleType::P_CPU:
                {
                    // �ײ㴫���Ǳ�������ֵ
                    str = base::StringPrintf(L"%d%%", (int)sample_data.data());
                }
                break;
            case StreamingSampleType::P_MEM:
                {
                    // �ײ㴫������MB����4GBΪ��ĸ
                    str = base::StringPrintf(L"%dMB", (int)sample_data.data());
                }
                break;
            default:
                break;
            }

            str += (sample_data.timestamp() == 0) ? L"(����ȱʧ)" : L"";

            instantaneous_label_->SetText(str);
        }

    private:
        StreamingSampleType dt_;

        views::Label* instantaneous_label_ = nullptr;
        DiagramView* diagram_view_ = nullptr;
    };


    //�������ȥ����
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

    // �Ҳ�ͼ����
    class DiagramAreaView : public views::View
    {
    public:
        DiagramAreaView()
        {
            set_notify_enter_exit_on_child(true);
        }

        void ChangeReport(streaming_performance::PerformanceReport* pb_reader)
        {
            // �Ѹ������͵����ݽ�������
            if (!pb_reader)
            {
                for (int i = 0; i < StreamingSampleType::COUNT; i++)
                {
                    sub_views_[i]->ChangeReport({});
                }

                return;
            }

            // �������͵Ĳ�������������������ݵ�ʱ���
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

            // ������ͼ�����ó�ʼʱ������Ա���ȫ��ͼ���ʱ�������
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

            // ��֪ͨ��������Ҫչʾ�����������ݵĵط�
            g_notifier_instance->OnMouseMoved();

            SchedulePaint();

            UpdateMouseOnViewInfo(event.location());
        }

        void OnMouseExited(const ui::MouseEvent& event)
        {
            g_mouse_entered = false;
            time_label_->SetText(L"");

            // ��֪ͨ��������Ҫչʾ�����������ݵĵط�
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

            views::Label* tl = new views::Label(L"��������ָ��");
            //tl->SetFont(ftPrimaryBold);

            views::Label* dl = new views::Label(L"(ȫʱ�θ�������ָ��仯���)");

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
            // ������������Ҳ�ľ���Ӧ�ö�Ӧ�ĸ������㣬�������ǰ���hover��ʱ���
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
                std::wstring time = base::StringPrintf(L"ʱ�䣺%02d:%02d:%02d", exploded.hour, exploded.minute, exploded.second);

                std::wstring text[StreamingSampleType::COUNT]{ L"���綪֡��",L"��Ⱦ��֡��",L"���붪֡��",
                L"GPUʹ���ʣ�",
                L"CPUʹ���ʣ�",
                L"�ڴ�ʹ���ʣ�",
                L"ֱ����CPUʹ���ʣ�",
                L"ֱ�����ڴ�ʹ���ʣ�" };

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

    // �����Ϣ��
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

            // ������Ϣ
            labels_[LabelType::LOGVER]->SetText(std::to_wstring(pb_reader->logver()));
            labels_[LabelType::SAMPLERATE]->SetText(std::to_wstring(pb_reader->sampling()));

            // ϵͳ����
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
            // �ڴ�
            labels_[LabelType::MEM]->SetText(base::UTF8ToUTF16(pb_reader->app_info().sys_info().memory()));
            // ��ַ
            switch (pb_reader->app_info().sys_info().network())
            {
            case streaming_performance::RuntimeNetWork::ETHERNET:
                labels_[LabelType::NETWORK]->SetText(L"��������");
                break;
            case streaming_performance::RuntimeNetWork::WIFI:
                labels_[LabelType::NETWORK]->SetText(L"WIFI����");
                break;
            default:
                NOTREACHED();
                labels_[LabelType::NETWORK]->SetText(L"��������");
                break;
            }
            labels_[LabelType::IP]->SetText(base::UTF8ToUTF16(pb_reader->app_info().sys_info().ip()));
            labels_[LabelType::REGION]->SetText(base::UTF8ToUTF16(pb_reader->app_info().sys_info().region()));
            labels_[LabelType::CO]->SetText(base::UTF8ToUTF16(pb_reader->app_info().sys_info().co()));

            // ������Ϣ
            labels_[LabelType::VERSION]->SetText(base::UTF8ToUTF16(pb_reader->app_info().version()));
            labels_[LabelType::ROOMID]->SetText(std::to_wstring(pb_reader->app_info().roomid()));
            labels_[LabelType::UID]->SetText(std::to_wstring(pb_reader->app_info().uid()));
            labels_[LabelType::ACCOUNT]->SetText(base::UTF8ToUTF16(pb_reader->app_info().account()));

            // ������Ϣ
            labels_[LabelType::LIVE_STATUS]->SetText((pb_reader->runtime_info().live_status() == 0) ?
                L"����" : L"ֱ����");
            labels_[LabelType::LIVE_START_TIME]->SetText(
                bililive::TimeFormatFriendlyDate(base::Time::FromDoubleT(pb_reader->runtime_info().start_time())));
            labels_[LabelType::LIVE_END_TIME]->SetText(
                bililive::TimeFormatFriendlyDate(base::Time::FromDoubleT(pb_reader->runtime_info().end_time())));
            labels_[LabelType::STREAM_NAME]->SetText(base::UTF8ToUTF16(pb_reader->runtime_info().code().substr(1)));

            // ��Ƶ����
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

            // ������Ϣ
            views::Label* gl = new views::Label(L"������Ϣ");
            //gl->SetFont(ftPrimaryBold);
            layout->StartRowWithPadding(0, 1, 0, 0);
            layout->AddView(gl);

            layout->StartRowWithPadding(0, 0, 0, 12);
            layout->AddView(new views::Label(L"����汾��"));
            layout->AddView(labels_[LabelType::LOGVER]);

            layout->StartRowWithPadding(0, 0, 0, 12);
            layout->AddView(new views::Label(L"�����ʣ�"));
            layout->AddView(labels_[LabelType::SAMPLERATE]);

            // ϵͳ��Ϣ
            gl = new views::Label(L"ϵͳ��Ϣ");
            //gl->SetFont(ftPrimaryBold);
            layout->StartRowWithPadding(0, 1, 0, 12);
            layout->AddView(gl);

            layout->StartRowWithPadding(0, 0, 0, 12);
            layout->AddView(new views::Label(L"ϵͳ�汾��"), 1, 1, views::GridLayout::LEADING, views::GridLayout::LEADING, 0, 0);
            layout->AddView(labels_[LabelType::SYSVER]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"CPU��"), 1, 1, views::GridLayout::LEADING, views::GridLayout::LEADING, 0, 0);
            layout->AddView(labels_[LabelType::CPU]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"�Կ���"), 1, 1, views::GridLayout::LEADING, views::GridLayout::LEADING, 0, 0);
            layout->AddView(labels_[LabelType::GRAPHICS]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"����ʹ���Կ���"));
            layout->AddView(labels_[LabelType::CUR_GRAPHICS]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"�ڴ棺"));
            layout->AddView(labels_[LabelType::MEM]);

            // ������Ϣ
            gl = new views::Label(L"������Ϣ");
            //gl->SetFont(ftPrimaryBold);
            layout->StartRowWithPadding(0, 1, 0, 12);
            layout->AddView(gl);

            layout->StartRowWithPadding(0, 0, 0, 12);
            layout->AddView(new views::Label(L"���ӷ�ʽ��"));
            layout->AddView(labels_[LabelType::NETWORK]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"��̫��IP��"));
            layout->AddView(labels_[LabelType::IP]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"������"));
            layout->AddView(labels_[LabelType::REGION]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"������Ӫ�̣�"));
            layout->AddView(labels_[LabelType::CO]);

            // ������Ϣ
            gl = new views::Label(L"������Ϣ");
            //gl->SetFont(ftPrimaryBold);
            layout->StartRowWithPadding(0, 1, 0, 12);
            layout->AddView(gl);

            layout->StartRowWithPadding(0, 0, 0, 12);
            layout->AddView(new views::Label(L"����汾��"));
            layout->AddView(labels_[LabelType::VERSION]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"����ţ�"));
            layout->AddView(labels_[LabelType::ROOMID]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"�û�UID��"));
            layout->AddView(labels_[LabelType::UID]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"�û��˺ţ�"));
            layout->AddView(labels_[LabelType::ACCOUNT]);

            // ��Ƶ����
            gl = new views::Label(L"�������");
            //gl->SetFont(ftPrimaryBold);
            layout->StartRowWithPadding(0, 1, 0, 12);
            layout->AddView(gl);

            layout->StartRowWithPadding(0, 0, 0, 12);
            layout->AddView(new views::Label(L"���ʣ�"));
            layout->AddView(labels_[LabelType::BITRATE]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"���ʿ��ƣ�"));
            layout->AddView(labels_[LabelType::BITRATETYPE]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"֡�ʣ�"));
            layout->AddView(labels_[LabelType::FPS]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"�ֱ��ʣ�"));
            layout->AddView(labels_[LabelType::RESOLUTION]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"��������"));
            layout->AddView(labels_[LabelType::ENCODER]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"����ƽ�⣺"));
            layout->AddView(labels_[LabelType::QUALITY]);

            // ������ز���
            gl = new views::Label(L"����״̬");
            //gl->SetFont(ftPrimaryBold);
            layout->StartRowWithPadding(0, 1, 0, 12);
            layout->AddView(gl);

            layout->StartRowWithPadding(0, 0, 0, 12);
            layout->AddView(new views::Label(L"����״̬��"));
            layout->AddView(labels_[LabelType::LIVE_STATUS]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"������ʼʱ�䣺"));
            layout->AddView(labels_[LabelType::LIVE_START_TIME]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"��������ʱ�䣺"));
            layout->AddView(labels_[LabelType::LIVE_END_TIME]);

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(new views::Label(L"ֱ���룺"), 1, 1, views::GridLayout::LEADING, views::GridLayout::LEADING, 0, 0);
            layout->AddView(labels_[LabelType::STREAM_NAME]);
        }

    private:
        views::Label* labels_[LabelType::COUNT] = {};

        base::WeakPtrFactory<DetailsInfoAreaView> weakptr_factory_;
    };

    // ����ȫ������ͼ
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

    // ������
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
                                window_title_ = L"����������� - " + path.BaseName().value();

                                // ͼ��������Ϣ����������չʾ
                                info_view_->ChangeReport(pb_reader_.get());
                                diagram_view_->ChangeReport(pb_reader_.get());
                                Layout();
                            }
                            else
                            {
                                MessageBox(GetWidget()->GetNativeWindow(), L"��Ч��protobuf�����ļ�");
                            }
                        }
                        else
                        {
                            MessageBox(GetWidget()->GetNativeWindow(), L"���ṩ��Ч���ļ�·��");
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
                window_title_ = L"�����������";
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
            source_textfield_->set_placeholder_text(L"ճ���ļ�ȫ·����URL����ǰֻ֧�ֱ����ļ����������д�BFS���������ɣ�");

            layout->StartRowWithPadding(0, 0, 0, 6);
            layout->AddView(source_textfield_);

            views::LabelButton* file_button = new views::LabelButton(this, L"�����ļ�");
            file_button->set_id(LoadFile_Button);
            file_button->SetIsDefault(true);
            views::LabelButton* url_button = new views::LabelButton(this, L"����URL");
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
        base::string16 window_title_ = L"�����������";
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
