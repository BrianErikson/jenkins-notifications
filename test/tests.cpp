#include <gtest/gtest.h>
#include <jnotify/JNotify.h>
#include <libnotify/notify.h>

TEST(notifications, notification)
{
  JNotify notification;
  ASSERT_TRUE(notify_is_initted());
  ASSERT_TRUE(notification.emit_notification("Test"));
}