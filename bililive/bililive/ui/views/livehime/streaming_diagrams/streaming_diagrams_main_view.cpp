#include "bililive/bililive/ui/views/livehime/streaming_diagrams/streaming_diagrams_main_view.h"

#include <list>
#include <shellapi.h>

#include "base/bind.h"
#include "base/notification/notification_service.h"
#include "base/prefs/pref_change_registrar.h"
#include "base/prefs/pref_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/gfx/canvas.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/profiles/profile.h"
#include "bililive/bililive/ui/views/controls/bililive_floating_scroll_view.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "bililive/bililive/utils/setting_util.h"
#include "bililive/bililive/utils/time_span.h"
#include "bililive/common/bililive_context.h"
#include "bililive/public/protobuf/streaming_details.pb.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/secret/public/user_info.h"
#include "bililive/secret/public/network_info.h"

#include "obs/obs-studio/libobs/obs.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"


namespace
{
    const int kCellWidth = GetLengthByDPIScale(20);
    const int kPixelPerSecond = 2;  // ����ʱ���ᣬһ������10�룬ÿ��2�����أ�
    const int kCellCountPerRow = 5; // �����ֵ/ռ��/�ʣ�5�����ӣ�ÿ������20�֣��߶Ȳ��̶�
    const gfx::Size kMinDiagramSize(kCellWidth * 15, kCellWidth * 3);
    const size_t kMaxSampleCache = kMinDiagramSize.width() / kPixelPerSecond; // ����ͼ����Ĳ������ݵ�����
    const SkColor kClrGridLine = SkColorSetRGB(217, 234, 244);
    const SkColor kClrDiagramLine = SkColorSetRGB(17, 125, 187);
    const SkColor kClrDiagramBk = SkColorSetA(SK_ColorLTGRAY, 100);
    const int64 kProcessMaxMemInMB = 4 * 1024;

    StreamingDiagramsMainView* g_single_instance = nullptr;
    StreamingSamplingNotifier* g_notifier_instance = nullptr;

    class StreamingSettingView;
    StreamingSettingView* g_setting_single_instance = nullptr;

    gfx::Point g_mouse_pt_in_screen;
    bool g_mouse_entered = false;
    int64 g_last_ts = 0;

    class StreamingSamplingObserver
    {
    public:
        virtual void OnSampling() = 0;
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

        void OnSampling()
        {
            FOR_EACH_OBSERVER(StreamingSamplingObserver, observer_list_, OnSampling());
        }

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
        DiagramView(StreamingSampleType dt, int64 start_show_ts)
            : dt_(dt)
        {
            set_border(views::Border::CreateSolidBorder(1, kClrDiagramLine));

            // �����ߵ��ƶ�����View��ʼ��ʾʱ��ʱ��Ϊ��׼�ģ�����ʱ�������һ��һ��Ĵ��������ƶ�
            start_show_ts_ = start_show_ts;
        }

        void AppendSample(const StreamingReportSampleData& sample)
        {
            // Ϊʹ�Ӿ�Ч����һ�㣬�����ߵı�Ե�͵�һ���������غ������
            if (samples_.empty())
            {
                start_show_ts_ = sample.ts;
            }
            samples_.push_back(sample);
            if (samples_.size() > kMaxSampleCache + 2)
            {
                samples_.pop_front();
            }
        }

        StreamingReportSampleData GetHoverPointSampleData() const
        {
            gfx::Point hover_pt = g_mouse_pt_in_screen;
            views::View::ConvertPointFromScreen(this, &hover_pt);

            // ������������Ҳ�ľ���Ӧ�ö�Ӧ�ĸ�������
            int reversed_index = (width() - hover_pt.x()) / kPixelPerSecond;
            if (reversed_index < (int)samples_.size())
            {
                auto riter = samples_.rbegin();
                std::advance(riter, reversed_index);
                if (riter != samples_.rend())
                {
                    return (*riter);
                }
            }
            return {};
        }

        StreamingReportSampleData GetLastSampleData() const
        {
            if (!samples_.empty())
            {
                return samples_.back();
            }
            return {};
        }

    protected:
        // View
        gfx::Size GetPreferredSize() override
        {
            return kMinDiagramSize;
        }

        void OnPaint(gfx::Canvas* canvas) override
        {
            OnPaintBackground(canvas);

            // ͼ����ϵͳ�������������ͼ�����ʾ��ʽ�����������ƶ�
            int64 last_sample_ts = 0;
            SkPath path;
            std::vector<SkPoint> pts;
            if (!samples_.empty())
            {
                int x_oft = 0;
                StreamingReportSampleData& sample = samples_.back();
                last_sample_ts = sample.ts;
                for (auto iter = samples_.rbegin(); iter != samples_.rend(); ++iter)
                {
                    // �ײ�����㲻���������ϸ��ղ���ʱ�������������ϲ�Ļ����������������ʵ��ʱ����������
                    int64 duration = std::max(0ll, last_sample_ts - iter->ts);
                    x_oft = width() - duration * kPixelPerSecond;
                    double y_rate = 0;

                    // �������ݵ����ͻ����0~1.0
                    switch (dt_)
                    {
                    case StreamingSampleType::NDF:
                    case StreamingSampleType::EDF:
                    case StreamingSampleType::RDF:
                        {
                            // �ײ㴫���Ƕ�֡��/ʹ����
                            y_rate = iter->data;
                        }
                        break;
                    case StreamingSampleType::GPU:
                    case StreamingSampleType::CPU:
                    case StreamingSampleType::MEM:
                    case StreamingSampleType::P_CPU:
                        {
                            // �ײ㴫���Ǳ�������ֵ
                            y_rate = iter->data / 100;
                        }
                        break;
                    case StreamingSampleType::P_MEM:
                        {
                            // �ײ㴫������MB����4GBΪ��ĸ
                            y_rate = iter->data / kProcessMaxMemInMB;
                        }
                        break;
                    default:
                        break;
                    }

                    pts.push_back(SkPoint::Make(x_oft, height() * (1.0f - y_rate)));

                    // ����Ѿ��������Ե�Ͳ�Ҫ�ٻ���
                    if (x_oft < 0)
                    {
                        break;
                    }
                }

                // Ϊͼ���������һ�����ײ�X��ķ�յ�
                pts.push_back(SkPoint::Make(x_oft, height()));
                pts.push_back(SkPoint::Make(width(), height()));

                path.addPoly(&pts[0], pts.size(), false);
            }
            else
            {
                last_sample_ts = base::Time::Now().ToDoubleT();
            }

            // ��UIչʾ��ʱ���Ϊԭ�㻭����ͼ
            int64 duration = std::max(0ll, last_sample_ts - start_show_ts_);
            int x_origin = width() - duration * kPixelPerSecond;
            if (x_origin > 0)
            {
                x_origin -= (x_origin / kCellWidth + 1) * kCellWidth;
            }
            else if (x_origin < 0)
            {
                x_origin = kCellWidth + x_origin % kCellWidth;
            }
            // ����
            for (int x = x_origin; x < width(); x += kCellWidth)
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
                canvas->DrawPath(path, paint);

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
        std::list<StreamingReportSampleData> samples_;
    };

    // ������������ͼ
    class StatisticsView : public views::View,
        public StreamingSamplingObserver
    {
    public:
        StatisticsView(StreamingSampleType dt, int64 start_show_ts)
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
                {StreamingSampleType::CPU, GetLocalizedString(IDS_STREAMING_SETTING_CPU_LABEL)},
                {StreamingSampleType::GPU, GetLocalizedString(IDS_STREAMING_SETTING_GPU_LABEL)},
                {StreamingSampleType::MEM, GetLocalizedString(IDS_STREAMING_SETTING_MEM_LABEL)},
                {StreamingSampleType::P_CPU, GetLocalizedString(IDS_STREAMING_SETTING_P_CPU_LABEL)},
                {StreamingSampleType::P_MEM, GetLocalizedString(IDS_STREAMING_SETTING_P_MEM_LABEL)},
                {StreamingSampleType::NDF, GetLocalizedString(IDS_STREAMING_SETTING_NDF_LABEL)},
                {StreamingSampleType::EDF, GetLocalizedString(IDS_STREAMING_SETTING_EDF_LABEL)},
                {StreamingSampleType::RDF, GetLocalizedString(IDS_STREAMING_SETTING_RDF_LABEL)},
            };
            LivehimeTitleLabel* label = new LivehimeTitleLabel(kTypeStr.at(dt));

            instantaneous_label_ = new LivehimeContentLabel();
            instantaneous_label_->SetHorizontalAlignment(gfx::ALIGN_RIGHT);

            layout->StartRowWithPadding(0, 0, 0, 0);
            layout->AddView(label);
            layout->AddView(instantaneous_label_);

            layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForCtrlTips);
            diagram_view_ = new DiagramView(dt, start_show_ts);
            layout->AddView(diagram_view_);
        }

        void AppendSample(const StreamingReportSampleData& sample)
        {
            diagram_view_->AppendSample(sample);
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

        // StreamingSampleObserver
        void OnSampling() override
        {
            // ���hoverʱ��ʾ���hover������ݣ�������ʾ���µĲ�������
            StreamingReportSampleData sample_data = { 0 };
            if (g_mouse_entered)
            {
                sample_data = diagram_view_->GetHoverPointSampleData();
            }
            else
            {
                sample_data = diagram_view_->GetLastSampleData();
            }
            UpdateHoverInfo(sample_data, g_mouse_entered);
        }

        void OnMouseMoved() override
        {
            OnSampling();
        }

        void OnMouseExited() override
        {
            OnSampling();
        }

    private:
        void UpdateHoverInfo(const StreamingReportSampleData& sample_data, bool instantaneous)
        {
            base::string16 str;
            switch (dt_)
            {
            case StreamingSampleType::NDF:
            case StreamingSampleType::EDF:
            case StreamingSampleType::RDF:
                {
                    // �ײ㴫���Ƕ�֡��/ʹ����
                    str = base::StringPrintf(L"%.0f%%", sample_data.data*100);
                }
                break;
            case StreamingSampleType::GPU:
            case StreamingSampleType::CPU:
            case StreamingSampleType::MEM:
            case StreamingSampleType::P_CPU:
                {
                    // �ײ㴫���Ǳ�������ֵ
                    str = base::StringPrintf(L"%d%%", (int)sample_data.data);
                }
                break;
            case StreamingSampleType::P_MEM:
                {
                    // �ײ㴫������MB����4GBΪ��ĸ
                    str = base::StringPrintf(L"%dMB", (int)sample_data.data);
                }
                break;
            default:
                break;
            }

            str += instantaneous ? GetLocalizedString(IDS_STREAMING_SETTING_GRAB_SAMPLING_LABEL) : L"";

            instantaneous_label_->SetText(str);
        }

    private:
        StreamingSampleType dt_;

        LivehimeContentLabel* instantaneous_label_ = nullptr;
        DiagramView* diagram_view_ = nullptr;
    };

    //���ý���
    class StreamingSettingView
        : public BililiveWidgetDelegate,
          public views::ButtonListener
    {
    public:
        static void ShowWindow(views::Widget *parent)
        {
            if (!g_setting_single_instance) {
                views::Widget *widget = new views::Widget();

                views::Widget::InitParams params;
                params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
                params.native_widget = new BililiveNativeWidgetWin(widget);
                params.parent = parent->GetNativeView();

                g_setting_single_instance = new StreamingSettingView();
                DoModalWidget(g_setting_single_instance, widget, params);
            }
        }
        // BililiveWidgetDelegate
        void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) override
        {
            if (details.child == this) {
                if (details.is_add) {
                    InitViews();
                }
            }
        }

        ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }

        gfx::Size GetPreferredSize() override
        {
            return gfx::Size(GetLengthByDPIScale(300), GetLengthByDPIScale(100));
        }

        string16 GetWindowTitle() const override
        {
            return GetLocalizedString(IDS_SET);
        }

        // ButtonListener
        void ButtonPressed(views::Button* sender, const ui::Event& event) override
        {
            if (sender == warning_checkbox_)
            {
                StreamingReportService::GetInstance()->SetAnalysisLiveQuality(warning_checkbox_->checked());
                PrefService* prefs = GetBililiveProcess()->profile()->GetPrefs();
                if (prefs)
                {
                    prefs->SetBoolean(prefs::kAnalysisLiveQuality, warning_checkbox_->checked());
                }
            }
        }

    private:
        StreamingSettingView()
        {
        }

        ~StreamingSettingView()
        {
            g_setting_single_instance = nullptr;
        }

        void InitViews()
        {
            views::GridLayout* layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            auto columnset = layout->AddColumnSet(0);
            columnset->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            columnset->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);

            warning_checkbox_ = new LivehimeCheckbox(GetLocalizedString(IDS_STREAMING_CHECKBOX_LAGGING));
            warning_checkbox_->set_listener(this);
            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightEnds);
            layout->AddView(warning_checkbox_);

            BililiveLabel* label = new LivehimeTipLabel(GetLocalizedString(IDS_STREAMING_CHECKBOX_LAGGING_LABEL));
            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(label);

            PrefService* prefs = GetBililiveProcess()->profile()->GetPrefs();
            if (prefs)
            {
                warning_checkbox_->SetChecked(prefs->GetBoolean(prefs::kAnalysisLiveQuality));
            }
        }

        LivehimeCheckbox* warning_checkbox_ = nullptr;
    };

    // ���ͼ����
    class DiagramAreaView : public views::View,
        public StreamingSamplingObserver,
        public views::ButtonListener
    {
    public:
        DiagramAreaView()
        {
            set_notify_enter_exit_on_child(true);
        }

        void InitViews(StatisticsView* sub_views[StreamingSampleType::COUNT])
        {
            views::GridLayout* layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            auto columnset = layout->AddColumnSet(0);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            columnset->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

            columnset = layout->AddColumnSet(1);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
            columnset->AddColumn(views::GridLayout::TRAILING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

            columnset = layout->AddColumnSet(2);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

            columnset = layout->AddColumnSet(3);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            columnset->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            columnset->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

            BililiveLabel* tl = new LivehimeTitleLabel(GetLocalizedString(IDS_STREAMING_SETTING_STATISTICS_LABEL));
            tl->SetFont(ftPrimaryBold);

            BililiveLabel* dl = new LivehimeTipLabel(std::to_wstring(kMaxSampleCache) +
                GetLocalizedString(IDS_STREAMING_SETTING_STATISTICS_SEC_LABEL));

            setting_button_ = new BililiveLabelButton(this, GetLocalizedString(IDS_SET));

            layout->StartRowWithPadding(0, 3, 0, 0);
            layout->AddView(tl);
            layout->AddView(dl);
            layout->AddView(setting_button_);

            time_label_ = new LivehimeSmallContentLabel();

            layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(new LivehimeTipLabel(std::to_wstring(kMaxSampleCache) + GetLocalizedString(IDS_SEC_LABEL)));
            layout->AddView(time_label_);
            layout->AddView(new LivehimeTipLabel(L"0" + GetLocalizedString(IDS_SEC_LABEL)));

            for (int i = 0; i < StreamingSampleType::COUNT; i++)
            {
                if (sub_views[i])
                {
                    layout->StartRowWithPadding(0, 2, 0, kPaddingRowHeightForGroupCtrls);
                    layout->AddView(sub_views[i]);
                }
            }
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
        }

        void OnMouseExited(const ui::MouseEvent& event)
        {
            g_mouse_entered = false;
            time_label_->SetText(L"");

            // ��֪ͨ��������Ҫչʾ�����������ݵĵط�
            g_notifier_instance->OnMouseMoved();

            SchedulePaint();
        }

        views::View* GetEventHandlerForPoint(const gfx::Point& point) override
        {
            if (!setting_button_->bounds().Contains(point))
            {
                return this;
            }
            else
            {
                return __super::GetEventHandlerForPoint(point);
            }
        }

        // ButtonListener
        void ButtonPressed(views::Button* sender, const ui::Event& event) override
        {
            if (sender == setting_button_)
            {
                StreamingSettingView::ShowWindow(this->GetWidget());
            }
        }

        // StreamingSampleObserver
        void OnSampling() override
        {
            if (g_mouse_entered)
            {
                gfx::Point hover_pt = g_mouse_pt_in_screen;
                views::View::ConvertPointFromScreen(this, &hover_pt);

                UpdateHoverInfo(hover_pt);
            }
        }

    private:
        void UpdateHoverInfo(const gfx::Point& pt)
        {
            // ������������Ҳ�ľ���Ӧ�ö�Ӧ�ĸ������㣬�������ǰ���hover��ʱ���
            int reversed_index = (width() - pt.x()) / kPixelPerSecond;
            base::Time::Exploded exploded = { 0 };
            base::Time::FromDoubleT(g_last_ts - reversed_index).LocalExplode(&exploded);
            time_label_->SetText(base::StringPrintf(L"- %02d:%02d:%02d -", exploded.hour, exploded.minute, exploded.second));
        }

    private:
        BililiveLabel* time_label_ = nullptr;
        BililiveLabelButton* setting_button_ = nullptr;
    };

    // �Ҳ���Ϣ��
    class DetailsInfoAreaView : public BililiveViewWithFloatingScrollbar,
        LivehimeLiveRoomObserver
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

            COUNT,
        };

    public:
        DetailsInfoAreaView()
            : weakptr_factory_(this)
        {
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

                    profile_pref_registrar_.Init(GetBililiveProcess()->profile()->GetPrefs());
                    profile_pref_registrar_.Add(prefs::kVideoBitRate,
                        base::Bind(&DetailsInfoAreaView::OnVideoBitRateChanged, weakptr_factory_.GetWeakPtr()));
                    profile_pref_registrar_.Add(prefs::kVideoBitRateControl,
                        base::Bind(&DetailsInfoAreaView::OnVideoBitRateControlChanged, weakptr_factory_.GetWeakPtr()));
                    profile_pref_registrar_.Add(prefs::kVideoFPSCommon,
                        base::Bind(&DetailsInfoAreaView::OnVideoFPSChanged, weakptr_factory_.GetWeakPtr()));
                    profile_pref_registrar_.Add(prefs::kVideoOutputCY,
                        base::Bind(&DetailsInfoAreaView::OnVideoOutputChanged, weakptr_factory_.GetWeakPtr()));
                    profile_pref_registrar_.Add(prefs::kOutputStreamVideoEncoder,
                        base::Bind(&DetailsInfoAreaView::OnVideoEncoderChanged, weakptr_factory_.GetWeakPtr()));
                    profile_pref_registrar_.Add(prefs::kOutputStreamVideoQuality,
                        base::Bind(&DetailsInfoAreaView::OnVideoQualityChanged, weakptr_factory_.GetWeakPtr()));

                    LivehimeLiveRoomController::GetInstance()->AddObserver(this);
                }
                else
                {
                    profile_pref_registrar_.RemoveAll();
                    LivehimeLiveRoomController::GetInstance()->RemoveObserver(this);
                }
            }
        }

        gfx::Size GetPreferredSize() override
        {
            gfx::Size size;
            size.set_width(GetLengthByDPIScale(370));
            size.set_height(GetHeightForWidth(size.width()));
            return size;
        }

        // LivehimeLiveRoomObserver
        void OnOpenLiveRoomSuccessed(const secret::LiveStreamingService::StartLiveInfo& start_live_info) override
        {
            UpdateLiveStatusInfo(false);
        }

        void OnLiveRoomStreamingStarted(const bililive::StartStreamingDetails& details) override
        {
            UpdateLiveStatusInfo(false);
        }

        void OnLiveRoomClosed(bool is_restreaming, const secret::LiveStreamingService::StartLiveInfo& start_live_info) override
        {
            UpdateLiveStatusInfo(true);
        }

    private:
        void InitViews()
        {
            for (int i = 0; i < LabelType::COUNT; i++)
            {
                labels_[i] = new LivehimeContentLabel();
                labels_[i]->SetHorizontalAlignment(gfx::ALIGN_LEFT);
            }
            labels_[LabelType::SYSVER]->SetMultiLine(true);
            labels_[LabelType::CPU]->SetMultiLine(true);
            labels_[LabelType::GRAPHICS]->SetMultiLine(true);
            labels_[LabelType::STREAM_NAME]->SetMultiLine(true);

            // ϵͳ����
            cpu_and_os_version si = obs_get_cpu_and_os_version();
            labels_[LabelType::SYSVER]->SetText(base::UTF8ToUTF16(si.os_version));
            labels_[LabelType::CPU]->SetText(base::UTF8ToUTF16(si.cpu_name));
            // gpu
            std::vector<base::string16> gpus;
            gpus_info gi = obs_get_gpus_info();
            for (int i = 0; i < gi.gpu_count; i++)
            {
                gpus.push_back(base::StringPrintf(L"%ls (�����汾 %ls)",
                    base::UTF8ToUTF16(gi.gpus[i].gpu_name).c_str(),
                    base::UTF8ToUTF16(gi.gpus[i].driver_version).c_str()));

                if (i == gi.cur_load_gpu_index)
                {
                    labels_[LabelType::CUR_GRAPHICS]->SetText(base::UTF8ToUTF16(gi.gpus[i].gpu_name));
                }
            }
            labels_[LabelType::GRAPHICS]->SetText(JoinString(gpus, '\n'));
            // �ڴ�
            MEMORYSTATUSEX memory;
            memory.dwLength = sizeof(memory);
            if (GlobalMemoryStatusEx(&memory))
            {
                float gb = memory.ullTotalPhys * 1.0f / 1024 / 1024 / 1024;
                labels_[LabelType::MEM]->SetText(base::StringPrintf(L"%.1fGB", gb));
            }

            // ������Ϣ
            labels_[LabelType::VERSION]->SetText(BililiveContext::Current()->GetExecutableVersion());
            labels_[LabelType::ROOMID]->SetText(std::to_wstring(GetBililiveProcess()->secret_core()->user_info().room_id()));
            labels_[LabelType::UID]->SetText(std::to_wstring(GetBililiveProcess()->secret_core()->account_info().mid()));
            switch ((secret::BehaviorEventNetwork)GetBililiveProcess()->secret_core()->network_info().network_type())
            {
            case secret::BehaviorEventNetwork::Ethernet:
                labels_[LabelType::NETWORK]->SetText(L"��������");
                break;
            case secret::BehaviorEventNetwork::Wifi:
                labels_[LabelType::NETWORK]->SetText(L"WIFI����");
                break;
            default:
                NOTREACHED();
                labels_[LabelType::NETWORK]->SetText(L"��������");
                break;
            }
            labels_[LabelType::IP]->SetText(base::UTF8ToUTF16(GetBililiveProcess()->secret_core()->network_info().ip()));
            labels_[LabelType::REGION]->SetText(GetBililiveProcess()->secret_core()->network_info().region());
            labels_[LabelType::CO]->SetText(GetBililiveProcess()->secret_core()->network_info().communications_operator());

            // ��Ƶ����
            auto pf = GetBililiveProcess()->profile()->GetPrefs();
            labels_[LabelType::BITRATE]->SetText(std::to_wstring(pf->GetInteger(prefs::kVideoBitRate)));
            labels_[LabelType::BITRATETYPE]->SetText(base::UTF8ToUTF16(pf->GetString(prefs::kVideoBitRateControl)));
            labels_[LabelType::FPS]->SetText(base::UTF8ToUTF16(pf->GetString(prefs::kVideoFPSCommon)));
            labels_[LabelType::RESOLUTION]->SetText(base::StringPrintf(L"%dx%d",
                pf->GetInteger(prefs::kVideoOutputCX), pf->GetInteger(prefs::kVideoOutputCY)));
            labels_[LabelType::ENCODER]->SetText(livehime::GetVideoEncoderFriendlyName(
                pf->GetString(prefs::kOutputStreamVideoEncoder)));
            labels_[LabelType::QUALITY]->SetText(livehime::GetVideoQualityFriendlyName(
                pf->GetInteger(prefs::kOutputStreamVideoQuality)));

            // ������Ϣ
            UpdateLiveStatusInfo(LivehimeLiveRoomController::GetInstance()->IsLiveReady());

            views::GridLayout* layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            auto columnset = layout->AddColumnSet(0);
            columnset->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            columnset->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
            columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
            columnset->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);

            columnset = layout->AddColumnSet(1);
            columnset->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
            columnset->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);

            // ϵͳ��Ϣ
            LivehimeTitleLabel* gl = new LivehimeTitleLabel(L"ϵͳ��Ϣ");
            gl->SetFont(ftPrimaryBold);
            layout->StartRowWithPadding(0, 1, 0, 0);
            layout->AddView(gl);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
            layout->AddView(new LivehimeTitleLabel(L"ϵͳ�汾��"), 1, 1, views::GridLayout::LEADING, views::GridLayout::LEADING, 0, 0);
            layout->AddView(labels_[LabelType::SYSVER]);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(new LivehimeTitleLabel(L"CPU��"), 1, 1, views::GridLayout::LEADING, views::GridLayout::LEADING, 0, 0);
            layout->AddView(labels_[LabelType::CPU]);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(new LivehimeTitleLabel(L"�Կ���"), 1, 1, views::GridLayout::LEADING, views::GridLayout::LEADING, 0, 0);
            layout->AddView(labels_[LabelType::GRAPHICS]);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(new LivehimeTitleLabel(L"����ʹ���Կ���"));
            layout->AddView(labels_[LabelType::CUR_GRAPHICS]);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(new LivehimeTitleLabel(L"�ڴ棺"));
            layout->AddView(labels_[LabelType::MEM]);

            // ������Ϣ
            gl = new LivehimeTitleLabel(L"������Ϣ");
            gl->SetFont(ftPrimaryBold);
            layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
            layout->AddView(gl);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
            layout->AddView(new LivehimeTitleLabel(L"���ӷ�ʽ��"));
            layout->AddView(labels_[LabelType::NETWORK]);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(new LivehimeTitleLabel(L"��̫��IP��"));
            layout->AddView(labels_[LabelType::IP]);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(new LivehimeTitleLabel(L"������"));
            layout->AddView(labels_[LabelType::REGION]);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(new LivehimeTitleLabel(L"������Ӫ�̣�"));
            layout->AddView(labels_[LabelType::CO]);

            // ������Ϣ
            gl = new LivehimeTitleLabel(L"������Ϣ");
            gl->SetFont(ftPrimaryBold);
            layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
            layout->AddView(gl);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
            layout->AddView(new LivehimeTitleLabel(L"����汾��"));
            layout->AddView(labels_[LabelType::VERSION]);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(new LivehimeTitleLabel(L"����ţ�"));
            layout->AddView(labels_[LabelType::ROOMID]);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(new LivehimeTitleLabel(L"�û�UID��"));
            layout->AddView(labels_[LabelType::UID]);

            // ��Ƶ����
            gl = new LivehimeTitleLabel(L"�������");
            gl->SetFont(ftPrimaryBold);
            layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
            layout->AddView(gl);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
            layout->AddView(new LivehimeTitleLabel(L"���ʣ�"));
            layout->AddView(labels_[LabelType::BITRATE]);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(new LivehimeTitleLabel(L"���ʿ��ƣ�"));
            layout->AddView(labels_[LabelType::BITRATETYPE]);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(new LivehimeTitleLabel(L"֡�ʣ�"));
            layout->AddView(labels_[LabelType::FPS]);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(new LivehimeTitleLabel(L"�ֱ��ʣ�"));
            layout->AddView(labels_[LabelType::RESOLUTION]);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(new LivehimeTitleLabel(L"��������"));
            layout->AddView(labels_[LabelType::ENCODER]);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(new LivehimeTitleLabel(L"����ƽ�⣺"));
            layout->AddView(labels_[LabelType::QUALITY]);

            // ������ز���
            gl = new LivehimeTitleLabel(L"����״̬");
            gl->SetFont(ftPrimaryBold);
            layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
            layout->AddView(gl);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
            layout->AddView(new LivehimeTitleLabel(L"����״̬��"));
            layout->AddView(labels_[LabelType::LIVE_STATUS]);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(new LivehimeTitleLabel(L"������ʼʱ�䣺"));
            layout->AddView(labels_[LabelType::LIVE_START_TIME]);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
            layout->AddView(new LivehimeTitleLabel(L"ֱ���룺"), 1, 1, views::GridLayout::LEADING, views::GridLayout::LEADING, 0, 0);
            layout->AddView(labels_[LabelType::STREAM_NAME]);
        }

        // ���ñ��
        void OnVideoBitRateChanged()
        {
            auto pf = GetBililiveProcess()->profile()->GetPrefs();
            labels_[LabelType::BITRATE]->SetText(std::to_wstring(pf->GetInteger(prefs::kVideoBitRate)));
        }

        void OnVideoBitRateControlChanged()
        {
            auto pf = GetBililiveProcess()->profile()->GetPrefs();
            labels_[LabelType::BITRATETYPE]->SetText(base::UTF8ToUTF16(pf->GetString(prefs::kVideoBitRateControl)));
        }

        void OnVideoFPSChanged()
        {
            auto pf = GetBililiveProcess()->profile()->GetPrefs();
            labels_[LabelType::FPS]->SetText(base::UTF8ToUTF16(pf->GetString(prefs::kVideoFPSCommon)));
        }

        void OnVideoOutputChanged()
        {
            auto pf = GetBililiveProcess()->profile()->GetPrefs();
            labels_[LabelType::RESOLUTION]->SetText(base::StringPrintf(L"%dx%d",
                pf->GetInteger(prefs::kVideoOutputCX), pf->GetInteger(prefs::kVideoOutputCY)));
        }

        void OnVideoEncoderChanged()
        {
            auto pf = GetBililiveProcess()->profile()->GetPrefs();
            labels_[LabelType::ENCODER]->SetText(livehime::GetVideoEncoderFriendlyName(
                pf->GetString(prefs::kOutputStreamVideoEncoder)));
        }

        void OnVideoQualityChanged()
        {
            auto pf = GetBililiveProcess()->profile()->GetPrefs();
            labels_[LabelType::QUALITY]->SetText(livehime::GetVideoQualityFriendlyName(
                pf->GetInteger(prefs::kOutputStreamVideoQuality)));
        }

        void UpdateLiveStatusInfo(bool is_live_ready)
        {
            labels_[LabelType::LIVE_STATUS]->SetText(is_live_ready ?
                L"����" : L"ֱ����");
            labels_[LabelType::LIVE_START_TIME]->SetText(is_live_ready ?
                L"" : bililive::TimeFormatFriendlyDate(LivehimeLiveRoomController::GetInstance()->start_live_time()));

            base::string16 key;
            if (!is_live_ready)
            {
                //key = key.substr(key.find('?') + 1);
                // ��ǰ�ַ���Ⱦ�����⣬�����ӿڷ��ص������롰?streamname=live_28007935_2042254&key=e3a631fcc19f7ba1d86fe324c3d1face��
                // ��label����û�а취��ȷ�Ļ�����ʾ�ģ�������չʾ
                base::StringPairs kvs;
                base::SplitStringIntoKeyValuePairs(LivehimeLiveRoomController::GetInstance()->start_live_info().rtmp_info.key,
                    '=', '&', &kvs);
                std::vector<std::string> vs;
                for (auto& iter : kvs)
                {
                    vs.push_back(iter.second);
                }
                key = base::UTF8ToUTF16(JoinString(vs, '\n'));
            }
            labels_[LabelType::STREAM_NAME]->SetText(key);

            PreferredSizeChanged();
        }

    private:
        PrefChangeRegistrar profile_pref_registrar_;

        BililiveLabel* labels_[LabelType::COUNT] = {};

        base::WeakPtrFactory<DetailsInfoAreaView> weakptr_factory_;
    };
}

void StreamingDiagramsMainView::ShowWindow()
{
    if (!g_single_instance)
    {
        views::Widget *widget = new views::Widget();
        views::Widget::InitParams params;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.native_widget = new BililiveNativeWidgetWin(widget);

        g_single_instance = new StreamingDiagramsMainView();
        ShowWidget(g_single_instance, widget, params);
    }
    else
    {
        if (g_single_instance->GetWidget())
        {
            g_single_instance->GetWidget()->Activate();
        }
    }
}

StreamingDiagramsMainView::StreamingDiagramsMainView()
    : BililiveWidgetDelegate(gfx::ImageSkia(), GetLocalizedString(IDS_STREAMING_SETTING_TITLE))
    , weakptr_factory_(this)
{
    notifier_.reset(new StreamingSamplingNotifier());
}

StreamingDiagramsMainView::~StreamingDiagramsMainView()
{
    g_single_instance = nullptr;
}

void StreamingDiagramsMainView::WindowClosing()
{
    UninitViews();

    // UI�ع����
    int close_live_status = (LivehimeLiveRoomController::GetInstance() &&
            LivehimeLiveRoomController::GetInstance()->IsLiveReady()) ? 0 : 1;
    std::vector<std::string> ps;
    ps.push_back(base::StringPrintf("show_live_status:%d", show_live_status_)); // UI��ʾʱ�Ŀ���״̬
    ps.push_back(base::StringPrintf("close_live_status:%d", close_live_status));// UI�ر�ʱ�Ŀ���״̬
    ps.push_back(base::StringPrintf("duration:%d", (int)((int64)base::Time::Now().ToDoubleT() - start_show_ts_)));// UI���ع�ʱ������
    auto mid = GetBililiveProcess()->secret_core()->account_info().mid();
    GetBililiveProcess()->secret_core()->event_tracking_service()->ReportLivehimeBehaviorEvent(
        secret::LivehimeBehaviorEvent::LivehimeStreamingQMCSShow,
        mid, JoinString(ps, ';')).Call();
}

void StreamingDiagramsMainView::OnStreamingReportSampling(const StreamingReportInfo& info)
{
    for (int i = 0; i < StreamingSampleType::COUNT; i++)
    {
        if (sub_views_[i])
        {
            sub_views_[i]->AppendSample(info.data[i]);
        }
    }
    // ÿ�β����ص��������ַ�һ��֪ͨ����չʾʵʱ���ݵĵط��ܸ���ʱ����ı仯�����Լ�����Ϣ
    g_last_ts = info.data[StreamingSampleType::CPU].ts;
    notifier_->OnSampling();

    SchedulePaint();
}

void StreamingDiagramsMainView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            show_live_status_ = LivehimeLiveRoomController::GetInstance()->IsLiveReady() ? 0 : 1;
            start_show_ts_ = base::Time::Now().ToDoubleT();
            InitViews();

            StreamingReportService::GetInstance()->AddObserver(this);
        }
    }
}

gfx::Size StreamingDiagramsMainView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.set_height(left_view_->GetPreferredSize().height() + kPaddingRowHeightForGroupCtrls * 2);
    return size;
}

void StreamingDiagramsMainView::OnPaintBackground(gfx::Canvas* canvas)
{
    __super::OnPaintBackground(canvas);

    gfx::Point pt = left_view_->bounds().bottom_right();
    ConvertPointToTarget(left_view_, this, &pt);
    canvas->DrawLine(gfx::Point(pt.x(), 0),
        gfx::Point(pt.x(), height()), GetColor(CtrlBorderNor));
}

void StreamingDiagramsMainView::InitViews()
{
    BililiveViewWithFloatingScrollbar* scroll_view = new BililiveViewWithFloatingScrollbar();
    {
        views::GridLayout* layout = new views::GridLayout(scroll_view);
        scroll_view->SetLayoutManager(layout);

        auto columnset = layout->AddColumnSet(0);
        columnset->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
        columnset->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
        columnset->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);

        left_view_ = new DiagramAreaView();
        {
            for (int i = 0; i < StreamingSampleType::COUNT; i++)
            {
                //SRT��ص�û��UI���棬ֻ���ϱ���
                if ( i == StreamingSampleType::SRT_SEND_PKTS ||
                     i == StreamingSampleType::SRT_LOST_PKTS ||
                     i == StreamingSampleType::SRT_DROP_PKTS ||
                     i == StreamingSampleType::SRT_RETTRANS_PKTS ||
                     i == StreamingSampleType::SRT_RTT_MS ||
                     i == StreamingSampleType::SRT_SEND_RATE_KBPS ||
                     i == StreamingSampleType::NET_NEED_TOTAL_FRAMES ||
                     (i == StreamingSampleType::GPU && !StreamingReportService::GetInstance()->IsGpuValid()))
                {
                    continue;
                }

                sub_views_[i] = new StatisticsView((StreamingSampleType)i, start_show_ts_);
            }

            left_view_->InitViews(sub_views_);
        }

        DetailsInfoAreaView* right_view = new DetailsInfoAreaView();

        layout->StartRowWithPadding(1.0f, 0, 0, kPaddingRowHeightForGroupCtrls);
        layout->AddView(left_view_);
        layout->AddView(right_view->Container(), 1, 1, views::GridLayout::FILL, views::GridLayout::FILL,
            0, left_view_->GetPreferredSize().height());
        layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    }

    SetLayoutManager(new views::FillLayout());
    AddChildView(scroll_view->Container());

    // �����ʱ�����150��Ĳ���������ʾ�����������Ǵӵ�ǰ�򿪵�ʱ��㿪ʼ��ʾ
    // ���ܷ������ﻺ���˶���������ݣ�UI�ϳ�ʼֻչʾ���150�������
    auto lcs = StreamingReportService::GetInstance()->LastCacheSamples();
    std::list<StreamingReportInfo> sris;
    std::list<StreamingReportInfo>::reverse_iterator iter = lcs.rbegin();
    std::advance(iter, std::min(kMaxSampleCache, lcs.size()));
    sris.assign(iter.base(), lcs.end());

    for (auto& info : sris)
    {
        for (int i = 0; i < StreamingSampleType::COUNT; i++)
        {
            if (sub_views_[i])
            {
                sub_views_[i]->AppendSample(info.data[i]);
            }
        }
    }
}

void StreamingDiagramsMainView::UninitViews()
{
    StreamingReportService::GetInstance()->RemoveObserver(this);
}
