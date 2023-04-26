#ifndef BASE_NOTIFICATION_NOTIFICATION_OBSERVER_H
#define BASE_NOTIFICATION_NOTIFICATION_OBSERVER_H


#include "base/base_export.h"

namespace base{
  class NotificationDetails;
  class NotificationSource;

  // This is the base class for notification observers. When a matching
  // notification is posted to the notification service, Observe is called.
  class BASE_EXPORT NotificationObserver {
  public:
    virtual void Observe(int type,
      const NotificationSource& source,
      const NotificationDetails& details) = 0;

  protected:
    virtual ~NotificationObserver() {}
  };
}
#endif
