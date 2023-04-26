#include "bililive/bililive/livehime/notify_ui_control/notify_ui_controller.h"

#include "base/ext/callable_callback.h"

#include "bilibase/basic_types.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"


namespace
{
    void OnShowLayerMessageBoxEndDialog(livehime::NotifyUICloseClosure layer_ui_close_callback,
        const base::string16&, void* data)
    {
        if (!layer_ui_close_callback.is_null())
        {
            layer_ui_close_callback.Run();
        }
    }

    void PracticalShowLayerMessageBox(
        gfx::NativeView parent_widget,
        const base::string16& title,
        const base::string16& message,
        const base::string16& buttons,
        livehime::UIPropType animate_ui_type,
        livehime::NotifyUICloseClosure layer_ui_close_callback)
    {
        livehime::MessageBoxEndDialogSignalHandler handler;
        handler.closure = base::Bind(OnShowLayerMessageBoxEndDialog, layer_ui_close_callback);
        livehime::ShowMessageBox(parent_widget, title, message, buttons, &handler,
            livehime::MessageBoxType_NONEICON, L"", false, animate_ui_type);
    }
}

namespace livehime
{
    NotifyUIController* NotifyUIController::GetInstance()
    {
        return Singleton<NotifyUIController>::get();
    }

    NotifyUIController::NotifyUIController()
        : weakptr_factory_(this)
    {
    }

    NotifyUIController::~NotifyUIController()
    {
    }

    void NotifyUIController::AddObserver(NotifyUIControllerObserver *observer)
    {
        observer_list_.AddObserver(observer);
    }

    void NotifyUIController::RemoveObserver(NotifyUIControllerObserver *observer)
    {
        observer_list_.RemoveObserver(observer);
    }

    void NotifyUIController::AddNotifyTask(NotifyUILayer layer, NotifyUIShowClosure show_closure)
    {
        LOG(INFO) << "[nuic] add layer task " << bilibase::enum_cast(layer);

        NotifyTask task{ layer, show_closure };
        task_queue_.push(task);

        if (running_)
        {
            ExecTask();
        }
    }

    void NotifyUIController::StartNotify()
    {
        if (running_)
        {
            return;
        }

        LOG(INFO) << "[nuic] start running.";

        FOR_EACH_OBSERVER(NotifyUIControllerObserver, observer_list_, OnNotifyUIStartRunning(task_queue_.size()));
        running_ = true;
        ExecTask();
    }

    void NotifyUIController::SuspendNotify()
    {
        running_ = false;
    }

    void NotifyUIController::ResumeNotify()
    {
        FOR_EACH_OBSERVER(NotifyUIControllerObserver, observer_list_, OnNotifyUIResume(task_queue_.size()));
        running_ = true;
        ExecTask();
    }

    void NotifyUIController::StopNotify()
    {
        if (running_)
        {
            LOG(INFO) << "[nuic] stop running.";
        }

        running_ = false;
        task_queue_ = TaskQueue();
        FOR_EACH_OBSERVER(NotifyUIControllerObserver, observer_list_, OnNotifyUIStoped());
    }

    void NotifyUIController::OnNotifyUIClose(NotifyUILayer layer)
    {
        LOG(INFO) << "[nuic] layer ui close " << bilibase::enum_cast(layer);

        task_pending_ = false;
        FOR_EACH_OBSERVER(NotifyUIControllerObserver, observer_list_, OnNotifyUIClosed(layer));

        if (running_)
        {
            base::MessageLoop::current()->PostTask(FROM_HERE,
                base::Bind(&NotifyUIController::ExecTask, weakptr_factory_.GetWeakPtr()));
        }
    }

    void NotifyUIController::ExecTask()
    {
        if (!task_queue_.empty())
        {
            if (task_pending_)
            {
                return;
            }

            NotifyTask task = task_queue_.top();
            task_queue_.pop();

            LOG(INFO) << "[nuic] layer ui task run " << bilibase::enum_cast(task.layer);

            if (!task.show_closure.is_null())
            {
                FOR_EACH_OBSERVER(NotifyUIControllerObserver, observer_list_, OnNotifyUIPreShow(task.layer));

                task_pending_ = true;
                task.show_closure.Run(base::Bind(&NotifyUIController::OnNotifyUIClose, weakptr_factory_.GetWeakPtr(),
                    task.layer));

                FOR_EACH_OBSERVER(NotifyUIControllerObserver, observer_list_, OnNotifyUIPostShow(task.layer));
            }
        }
        else
        {
            LOG(INFO) << "[nuic] all layer ui show completed.";

            FOR_EACH_OBSERVER(NotifyUIControllerObserver, observer_list_, OnNotifyUIAllShowCompleted());
        }
    }



    // 弹出有窗口优先层级的公共MessageBox
    void ShowLayerMessageBox(NotifyUILayer layer,
        gfx::NativeView parent_widget,
        const base::string16& title,
        const base::string16& message,
        const base::string16& buttons,
        UIPropType animate_ui_type/* = UIPropType::Unknown*/)
    {
        NotifyUIController::GetInstance()->AddNotifyTask(layer,
            base::Bind(PracticalShowLayerMessageBox, parent_widget, title, message, buttons,
                animate_ui_type));
    }
}
