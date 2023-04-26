// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "notification_service_impl.h"
#include "base/lazy_instance.h"
#include "base/threading/thread_local.h"
#include "notification_types.h"
#include "notification_details.h"
#include "notification_service.h"
#include "notification_observer.h"

namespace {

base::LazyInstance<base::ThreadLocalPointer<base::NotificationServiceImpl> >
    lazy_tls_ptr = LAZY_INSTANCE_INITIALIZER;

}  // namespace

namespace base{
  // static
  NotificationServiceImpl* NotificationServiceImpl::current() {
    return lazy_tls_ptr.Pointer()->Get();
  }

  // static
  NotificationService* NotificationService::current() {
    return NotificationServiceImpl::current();
  }

  // static
  NotificationService* NotificationService::Create() {
    return new NotificationServiceImpl;
  }

  // static
  bool NotificationServiceImpl::HasKey(const NotificationSourceMap& map,
    const NotificationSource& source) {
    return map.find(source.map_key()) != map.end();
  }

  NotificationServiceImpl::NotificationServiceImpl() {
    DCHECK(current() == NULL);
    lazy_tls_ptr.Pointer()->Set(this);
  }

  void NotificationServiceImpl::AddObserver(NotificationObserver* observer,
    int type,
    const NotificationSource& source) {
    // We have gotten some crashes where the observer pointer is NULL. The problem
    // is that this happens when we actually execute a notification, so have no
    // way of knowing who the bad observer was. We want to know when this happens
    // in release mode so we know what code to blame the crash on (since this is
    // guaranteed to crash later).
    CHECK(observer);

    NotificationObserverList* observer_list;
    if (HasKey(observers_[type], source)) {
      observer_list = observers_[type][source.map_key()];
    }
    else {
      observer_list = new NotificationObserverList;
      observers_[type][source.map_key()] = observer_list;
    }

    observer_list->AddObserver(observer);
  }

  void NotificationServiceImpl::RemoveObserver(NotificationObserver* observer,
    int type,
    const NotificationSource& source) {
    // This is a very serious bug.  An object is most likely being deleted on
    // the wrong thread, and as a result another thread's NotificationServiceImpl
    // has its deleted pointer in its map.  A garbge object will be called in the
    // future.
    // NOTE: when this check shows crashes, use BrowserThread::DeleteOnIOThread or
    // other variants as the trait on the object.
    CHECK(HasKey(observers_[type], source));

    NotificationObserverList* observer_list =
      observers_[type][source.map_key()];
    if (observer_list) {
      observer_list->RemoveObserver(observer);
      if (!observer_list->size()) {
        observers_[type].erase(source.map_key());
        delete observer_list;
      }
    }
  }

  void NotificationServiceImpl::Notify(int type,
    const NotificationSource& source,
    const NotificationDetails& details) {
    DCHECK_GT(type, NOTIFICATION_ALL) <<
      "Allowed for observing, but not posting.";

    // There's no particular reason for the order in which the different
    // classes of observers get notified here.

    // Notify observers of all types and all sources
    if (HasKey(observers_[NOTIFICATION_ALL], AllSources()) &&
      source != AllSources()) {
      FOR_EACH_OBSERVER(NotificationObserver,
        *observers_[NOTIFICATION_ALL][AllSources().map_key()],
        Observe(type, source, details));
    }

    // Notify observers of all types and the given source
    if (HasKey(observers_[NOTIFICATION_ALL], source)) {
      FOR_EACH_OBSERVER(NotificationObserver,
        *observers_[NOTIFICATION_ALL][source.map_key()],
        Observe(type, source, details));
    }

    // Notify observers of the given type and all sources
    if (HasKey(observers_[type], AllSources()) &&
      source != AllSources()) {
      FOR_EACH_OBSERVER(NotificationObserver,
        *observers_[type][AllSources().map_key()],
        Observe(type, source, details));
    }

    // Notify observers of the given type and the given source
    if (HasKey(observers_[type], source)) {
      FOR_EACH_OBSERVER(NotificationObserver,
        *observers_[type][source.map_key()],
        Observe(type, source, details));
    }
  }


  NotificationServiceImpl::~NotificationServiceImpl() {
    lazy_tls_ptr.Pointer()->Set(NULL);

    for (int i = 0; i < static_cast<int>(observers_.size()); i++) {
      NotificationSourceMap omap = observers_[i];
      for (NotificationSourceMap::iterator it = omap.begin();
        it != omap.end(); ++it)
        delete it->second;
    }
  }
}