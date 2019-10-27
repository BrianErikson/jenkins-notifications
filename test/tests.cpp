#include <gtest/gtest.h>
#include <jnotify/JNotify.h>
#include <libnotify/notify.h>
#include <configuration.h>

TEST(notifications, notification)
{
  JNotify notification;
  ASSERT_TRUE(notify_is_initted());
  ASSERT_TRUE(notification.emit_notification("Test"));
}

TEST(http, http_get)
{
  SharedUrlHook listener = std::make_shared<url_notify::UrlHook>("http://www.google.com/",
      [](const std::string &html) -> bool {
        return !html.empty();
      }, 1000);
  ASSERT_TRUE(listener->execute());
}

TEST(config, config_get)
{
  url_notify::Config config;
  std::cout << config.try_load(std::string(TEST_RES) + "jnotify.cfg") << std::endl;
  ASSERT_TRUE(config.is_loaded());
}
