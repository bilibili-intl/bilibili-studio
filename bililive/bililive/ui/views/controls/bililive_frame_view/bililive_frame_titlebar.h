#ifndef BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_FRAME_VIEW_BILILIVE_FRAME_TITLEBAR_H
#define BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_FRAME_VIEW_BILILIVE_FRAME_TITLEBAR_H

#include "ui/views/view.h"

enum TitleBarButton
{
    TBB_CLOSE = 1 << 0,
    TBB_MAX = 1 << 1,
    TBB_MIN = 1 << 2,
    TBB_HELP = 1 << 3,
};

class BililiveFrameTitleBarDelegate
{
public:
    virtual bool OnTitleBarButtonPressed(TitleBarButton button) { return true; }
};

class BililiveFrameTitleBar : public views::View
{
public:
    static const char kDevViewClassName[];

    BililiveFrameTitleBar();
    virtual ~BililiveFrameTitleBar() = default;

    virtual int NonClientHitTest(const gfx::Point &point);
    virtual void SetTitle(const base::string16& title);

    void SetDelegate(BililiveFrameTitleBarDelegate *titlebar_deleagte) { delegate_ = titlebar_deleagte; }

    void SetActive(bool active);

protected:
    BililiveFrameTitleBarDelegate* delegate() { return delegate_; }
    bool active() const { return active_; }

    virtual void OnActiveChanged(bool prev_state) {}

    // View
    virtual const char* GetClassName() const override{ return kDevViewClassName; }

private:
    BililiveFrameTitleBarDelegate *delegate_;
    bool active_;

    DISALLOW_COPY_AND_ASSIGN(BililiveFrameTitleBar);
};

#endif