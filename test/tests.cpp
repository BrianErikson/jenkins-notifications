#include <gtest/gtest.h>
#include <jnotify/JNotify.h>
#include <libnotify/notify.h>

TEST(notifications, notification)
{
  JNotify notification;
  ASSERT_TRUE(notify_is_initted());
  ASSERT_TRUE(notification.emit_notification("Test"));
}

TEST(http, http_get)
{
  JNotify jnotify;
  jnotify.register_url("http://www.google.com/", [](const std::string &html) {
    ASSERT_TRUE(!html.empty());
    std::cout << html << std::endl;
  });

  jnotify.query_endpoints();
}
