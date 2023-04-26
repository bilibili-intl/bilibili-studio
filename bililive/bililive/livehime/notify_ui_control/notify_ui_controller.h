#pragma once

#include <queue>

#include "base/observer_list.h"
#include "base/memory/singleton.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string16.h"

#include "ui/gfx/native_widget_types.h"

#include "bililive/bililive/livehime/notify_ui_control/notify_ui_control_type.h"
#include "bililive/bililive/livehime/notify_ui_control/notify_ui_prop.h"


namespace livehime
{
    class NotifyUIControllerObserver
    {
    public:
        virtual ~NotifyUIControllerObserver() = default;

        virtual void OnNotifyUIStartRunning(size_t cc) {}
        virtual void OnNotifyUIResume(size_t cc) {}
        virtual void OnNotifyUIStoped() {}
        virtual void OnNotifyUIPreShow(NotifyUILayer layer) {}
        virtual void OnNotifyUIPostShow(NotifyUILayer layer) {}
        virtual void OnNotifyUIClosed(NotifyUILayer layer) {}
        virtual void OnNotifyUIAllShowCompleted() {}
    };

    // 与具体业务无关的UI展示层级控制器
    class NotifyUIController
    {
        struct NotifyTask 
        {
            NotifyUILayer layer;
            NotifyUIShowClosure show_closure;

            bool operator<(const NotifyTask& rht) const
            {
                return layer > rht.layer; //小顶堆
            }
        };
    public:
        static NotifyUIController* GetInstance();

        void AddObserver(NotifyUIControllerObserver *observer);
        void RemoveObserver(NotifyUIControllerObserver *observer);

        // 添加UI提示展示任务
        void AddNotifyTask(NotifyUILayer layer, NotifyUIShowClosure show_closure);

        // 开始按UI提示层级逐级执行展示任务
        void StartNotify();

        // 暂停执行提示任务
        void SuspendNotify();
        void ResumeNotify();

        // 中止提示任务执行，清空任务队列
        void StopNotify();

    private:
        NotifyUIController();
        ~NotifyUIController();

        void OnNotifyUIClose(NotifyUILayer layer);
        void ExecTask();

    private:
        friend struct DefaultSingletonTraits<NotifyUIController>;

        typedef std::priority_queue<NotifyTask> TaskQueue;
        TaskQueue task_queue_;
        bool running_ = false;
        bool task_pending_ = false;

        ObserverList<NotifyUIControllerObserver> observer_list_;

        base::WeakPtrFactory<NotifyUIController> weakptr_factory_;

        DISALLOW_COPY_AND_ASSIGN(NotifyUIController);
    };


    // 弹出有窗口优先层级的公共MessageBox
    void ShowLayerMessageBox(NotifyUILayer layer,
        gfx::NativeView parent_widget,
        const base::string16& title,
        const base::string16& message,
        const base::string16& buttons,
        UIPropType animate_ui_type = UIPropType::Unknown);
}
