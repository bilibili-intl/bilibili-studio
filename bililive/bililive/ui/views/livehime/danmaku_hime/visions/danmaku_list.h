#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISION_DANMAKU_LIST_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISION_DANMAKU_LIST_H_

#include <memory>

#include "base/timer/timer.h"

#include "ui/base/events/event.h"

#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/vision_group.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/nav_down_button.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/overlay_scroll_bar.h"


class ScrollingInterpolator;

namespace dmkhime {

enum class DmkHitType;
class DanmakuVision;
class DanmakuListAdapter;
class OverlayScrollBar;
class DmkDataObject;


class ListItemRecycledListener {
public:
    virtual ~ListItemRecycledListener() = default;

    virtual void OnChildRecycled(DanmakuVision* vision) = 0;
};

class ListStatusChangeListener {
public:
    virtual ~ListStatusChangeListener() = default;

    virtual void OnListAnimationStart() = 0;
    virtual void OnListAutoScrollStateChanged(bool auto_scroll) = 0;
};


// �����б��֣�֧�ֻ��ա�
// �б����ݷ��� ListAdapter �У��� Notify* ��ط���֪ͨ�б����ݸ���
class DanmakuList : public VisionGroup {
public:
    explicit DanmakuList(bool can_interactive = true, float speed_factor = 1.f);
    ~DanmakuList();

    bool IsClickable(const gfx::Point& p) const;
    bool OnMouseWheel(const ui::MouseWheelEvent& event);
    bool OnMousePressed(
        const ui::MouseEvent& event, DmkHitType* type, const DmkDataObject** data);
    void OnMouseReleased(
        const ui::MouseEvent& event, DmkHitType* type, const DmkDataObject** data);
    void OnMouseCaptureLost();
    void OnMouseMoved(const ui::MouseEvent& event);
    bool OnMouseDragged(const ui::MouseEvent& event);

    void SetAdapter(DanmakuListAdapter* adapter);
    void SetRecycleListener(ListItemRecycledListener* listener);
    void SetStatusChangeListener(ListStatusChangeListener* listener);

    void SetNavDownText(const string16& text);

    /**
     * ֪ͨ�б�������������������ݼ�β����
     * ������һ���� Vision ����ײ��п�϶��һЩ��������������Ӧ��
     * �� Vision �ᱻ���룬ֱ����϶������������ʲô��������
     */
    void NotifyItemAddToBottom();

    /**
     * ֪ͨ�б�������������ݼ������Ƴ���
     * �������� Vision ����������������뱻�Ƴ�������Ӧ���� Vision�����Ƴ���Ӧ Vision��
     * ����Ƴ��󶥲����ֿ�϶�����彫���ƣ�������ƺ�ײ��Ŀ�϶������еĻ�����
     */
    void NotifyItemRemoveFromTop(int length);

    /**
     * ֪ͨ�б����ݼ�������δ֪�ı䡣
     * ���ӵ�ǰλ�����²��ֶ�Ӧ������������ Vision��
     */
    void NotifyDataSetChanged();

    /**
     * �ƶ������һ�ʹ����ȫ��ʾ��
     */
    void ScrollToBottom(bool smooth = true);
    void ScrollToBottomIfAtBottom(bool smooth = true);

    bool IsAnimating() const;
    bool IsAutoScrollToBottom() const;

    bool OnComputeScroll(int interval);

    void Refresh();

    void UpdateOverlayScrollBar();

protected:
    // VisionGroup
    void OnMeasure(int width, int height) override;
    void OnSizeChanged(int width, int height, int old_w, int old_h) override;
    void OnLayout(int left, int top, int right, int bottom) override;
    void OnDrawOver(Sculptor* s) override;

private:
    void OnScroll(int dy, OverlayScrollBar::Direction dir);
    void OnAutoScrollStateChanged();
    void OnAnimationStart();
    void OnHidingScrollBar();

    void FillItems(int pos, int offset);
    void ScrollToPosition(int pos, int offset = 0);
    void SmoothScrollToPosition(int pos, int offset = 0);
    DanmakuVision* MakeNewDanmakuVision(int pos);

    int DetermineScroll(int dy);
    void RecordCurPositionAndOffset();
    void ComputeTotalHeight(int* prev, int* next, int* to_bottom) const;

    void AwakeScrollBar(bool preparing_to_sleep);

    bool IsAtBottom(int dy) const;

    /**
     * ���ƴӵ�ǰλ�õ��ײ��ľ��롣
     * ���������߶���ȫ��ͬ������Ƶ�ֵ��׼ȷ�ġ�
     */
    int GetCurToBottomDistance() const;
    DanmakuVision* GetFirstVision() const;
    DanmakuVision* GetLastVision() const;
    DanmakuVision* GetFirstVisibleVision() const;
    DanmakuVision* GetLastVisibleVision() const;

    /**
     * �ٶ������ݣ����� Vision���������� dy��
     * �������ܻ���ֿ�϶����ʹ�ö�Ӧ�������������ֻ��� Vision ���������У���
     * ����ڼٶ����� dy �Ĺ����д�����������ʵ�ʿ��ƶ��ľ��룬���򷵻� dy��
     */
    int FillTopSpace(int dy);

    /**
     * �ٶ������ݣ����� Vision���������� dy��
     * �ײ����ܻ���ֿ�϶����ʹ�ö�Ӧ�������������ֻ��� Vision ���������У���
     * ����ڼٶ����� dy �Ĺ����д��ף�������ʵ�ʿ��ƶ��ľ��룬���򷵻� dy��
     */
    int FillBottomSpace(int dy);

    /**
     * �ٶ������ݣ����� Vision���������� dy ʱ��
     * ������ȫ���������� Vision��
     */
    void RecycleTopItems(int dy);

    /**
     * �ٶ������ݣ����� Vision���������� dy ʱ��
     * ������ȫ�����ײ��� Vision��
     */
    void RecycleBottomItems(int dy);

    int cur_position_;
    int cur_offset_in_position_;

    float scrolling_scale_ = 1;
    bool is_at_bottom_ = false;
    bool auto_scroll_to_bottom_ = true;
    ListStatusChangeListener* s_listener_ = nullptr;

    bool can_interactive_;
    bool is_pressed_on_nav_down_ = false;
    bool is_pressed_on_scrollbar_ = false;

    NavDownButton nav_down_button_;
    OverlayScrollBar scroll_bar_;
    std::vector<DanmakuVision*> recycler_;
    std::unique_ptr<DanmakuListAdapter> adapter_;

    base::OneShotTimer<DanmakuList> scrollbar_timer_;
    ListItemRecycledListener* recycled_listener_ = nullptr;
    std::unique_ptr<ScrollingInterpolator> interpolator_;
};

}

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISION_DANMAKU_LIST_H_