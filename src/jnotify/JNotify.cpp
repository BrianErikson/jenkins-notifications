#include "JNotify.h"
#include <curl/curl.h>
#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <memory>
#include <algorithm>

JNotify::JNotify()
{
  curl_global_init(CURL_GLOBAL_ALL);
  notify_init("JNotify");
}

JNotify::~JNotify()
{
  // Free curl handles before deinit curl
  this->curl_handles.erase(this->curl_handles.begin(), this->curl_handles.end());
  notify_uninit();
  curl_global_cleanup();
}

bool JNotify::emit_notification(const std::string &message)
{
  NotifyNotification *notification = notify_notification_new(message.c_str(), "", "");

  bool retval = notify_notification_show(notification, nullptr);

  g_object_unref (G_OBJECT(notification));
  return retval;
}

void JNotify::register_url(const std::string &url, const UlCallback &callback)
{
  this->curl_handles.push_back(std::make_unique<UrlListener>(url, callback));
}

void JNotify::query_endpoints()
{
  for (auto &listener : this->curl_handles) {
    listener->callback(listener->try_get());
  }
}
