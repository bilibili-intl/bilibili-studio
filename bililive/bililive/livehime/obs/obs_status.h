/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_BILILIVE_LIVEHIME_OBS_OBS_STATUS_H_
#define BILILIVE_BILILIVE_LIVEHIME_OBS_OBS_STATUS_H_

#include <string>

#include "base/files/file_path.h"
#include "base/notification/notification_observer.h"
#include "base/notification/notification_registrar.h"

#include "bililive/bililive/livehime/obs/obs_status_details.h"

namespace bililive {

// If a class wants to receive obs status change notifications, it must implement this delegate,
// and join into a OBSStatusMonitor instance.
// All delegate functions are called on UI thread.
class OBSStatusDelegate {
public:
    virtual void OnRecordingStarting() {}
    virtual void OnStartRecording() {}
    virtual void OnRecordingStopping() {}
    virtual void OnStopRecording(const base::FilePath& video_path) {}
    virtual void OnRecordingError(const base::FilePath& video_path, obs_proxy::RecordingErrorCode error_code,
        const std::wstring& error_message) {}

    virtual void OnStreamingMetricsFeedback(double bandwidth_speed, double frame_loss_rate) {}
    virtual void OnStreamEncodedQPFeedback(int qp) {}

protected:
    virtual ~OBSStatusDelegate() {}
};

class OBSStatusMonitor : public base::NotificationObserver {
public:
    explicit OBSStatusMonitor(OBSStatusDelegate* status_delegate);

    ~OBSStatusMonitor();

private:
    void Observe(int type,
                 const base::NotificationSource& source,
                 const base::NotificationDetails& details) override;

    DISALLOW_COPY_AND_ASSIGN(OBSStatusMonitor);

private:
    base::NotificationRegistrar registrar_;
    OBSStatusDelegate* delegate_;
};

}   // namespace bililive

#endif  // BILILIVE_BILILIVE_LIVEHIME_OBS_OBS_STATUS_H_
