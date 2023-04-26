/*
 @ 0xCCCCCCCC
*/

#include "bililive/bililive/livehime/obs/obs_status.h"

#include "base/logging.h"
#include "base/notification/notification_service.h"

#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_thread.h"

namespace bililive {

OBSStatusMonitor::OBSStatusMonitor(OBSStatusDelegate* status_delegate)
    : delegate_(status_delegate)
{
    DCHECK(status_delegate);
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    auto all_sources = base::NotificationService::AllSources();

    registrar_.Add(this, bililive::NOTIFICATION_LIVEHIME_RECORDING_STARTING, all_sources);
    registrar_.Add(this, bililive::NOTIFICATION_LIVEHIME_START_RECORDING, all_sources);
    registrar_.Add(this, bililive::NOTIFICATION_LIVEHIME_RECORDING_STOPPING, all_sources);
    registrar_.Add(this, bililive::NOTIFICATION_LIVEHIME_STOP_RECORDING, all_sources);
    registrar_.Add(this, bililive::NOTIFICATION_LIVEHIME_RECORDING_ERROR, all_sources);

    registrar_.Add(this, bililive::NOTIFICATION_LIVEHIME_STREAMING_FEEDBACK, all_sources);
    registrar_.Add(this, bililive::NOTIFICATION_LIVEHIME_STREAM_ENCODED_QP, all_sources);
}

OBSStatusMonitor::~OBSStatusMonitor()
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));
    registrar_.RemoveAll();
}

void OBSStatusMonitor::Observe(int type, const base::NotificationSource& source,
                               const base::NotificationDetails& details)
{
    switch (type) {
        case bililive::NOTIFICATION_LIVEHIME_RECORDING_STARTING:
            delegate_->OnRecordingStarting();
            break;

        case bililive::NOTIFICATION_LIVEHIME_START_RECORDING:
            delegate_->OnStartRecording();
            break;

        case bililive::NOTIFICATION_LIVEHIME_RECORDING_STOPPING:
            delegate_->OnRecordingStopping();
            break;

        case bililive::NOTIFICATION_LIVEHIME_STOP_RECORDING:
            delegate_->OnStopRecording(base::Details<RecordingDetails>(details)->video_path);
            break;

        case bililive::NOTIFICATION_LIVEHIME_RECORDING_ERROR: {
            auto params = base::Details<RecordingDetails>(details).ptr();
            delegate_->OnRecordingError(params->video_path, params->error_code, params->error_msg);
        }
            break;

        case bililive::NOTIFICATION_LIVEHIME_STREAMING_FEEDBACK: {
            auto params = base::Details<StreamingMetricsDetails>(details).ptr();
            delegate_->OnStreamingMetricsFeedback(params->bandwidth_speed, params->frame_loss_rate);
        }
            break;

        case bililive::NOTIFICATION_LIVEHIME_STREAM_ENCODED_QP:
            delegate_->OnStreamEncodedQPFeedback(*base::Details<int>(details).ptr());
            break;

        default:
            NOTREACHED() << "Unhandled notification";
            break;
    }
}

}   // namespace bililive

