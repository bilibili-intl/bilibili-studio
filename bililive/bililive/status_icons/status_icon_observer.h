#ifndef BILILIVE_BILILIVE_STATUS_ICONS_STATUS_ICON_OBSERVER_H_
#define BILILIVE_BILILIVE_STATUS_ICONS_STATUS_ICON_OBSERVER_H_

class StatusIconObserver
{
public:
    virtual void OnStatusIconClicked() = 0;

#if defined(OS_WIN)
    virtual void OnBalloonClicked() {}
#endif

protected:
    virtual ~StatusIconObserver() {}
};

#endif