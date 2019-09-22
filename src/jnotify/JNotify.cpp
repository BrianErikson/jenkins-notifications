#include "JNotify.h"
#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <memory>

JNotify::JNotify()
{
  notify_init("JNotify");
}

JNotify::~JNotify()
{
  notify_uninit();
}

bool JNotify::emit_notification(const std::string &message)
{
  NotifyNotification *notification = notify_notification_new(message.c_str(), "", "");

  bool retval = notify_notification_show(notification, nullptr);

  g_object_unref (G_OBJECT(notification));
  return retval;
}
