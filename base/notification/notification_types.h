#ifndef BASE_NOTIFICATION_NOTIFICATION_TYPES_H
#define BASE_NOTIFICATION_NOTIFICATION_TYPES_H

#include "base/base_export.h"

// This file describes various types used to describe and filter notifications
// that pass through the NotificationService.
//
// Only notifications that are fired from the content module should be here. We
// should never have a notification that is fired by the embedder and listened
// to by content.
namespace base{
  enum BASE_EXPORT NotificationType {
    NOTIFICATION_CONTENT_START = 0,

    // General -----------------------------------------------------------------

    // Special signal value to represent an interest in all notifications.
    // Not valid when posting a notification.
    NOTIFICATION_ALL = NOTIFICATION_CONTENT_START,

    // Custom notifications used by the embedder should start from here.
    NOTIFICATION_CONTENT_END,
  };
}
#endif  // BASE_NOTIFICATION_NOTIFICATION_TYPES_H
