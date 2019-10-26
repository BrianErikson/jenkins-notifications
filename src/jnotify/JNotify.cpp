#include "JNotify.h"
#include "JnConfig.h"
#include <curl/curl.h>
#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <memory>
#include <algorithm>
#include <iostream>
#include <ctime>
#include <chrono>
#include <rapidjson/document.h>
#include <sstream>

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

void JNotify::force_query_endpoints()
{
  for (auto &listener : this->curl_handles) {
    listener->callback(listener->try_get());
  }
}

bool JNotify::init_config()
{
  auto res = this->config.try_load();
  if (!this->config.is_loaded()) {
    std::cerr << res << std::endl;
  }
  return this->config.is_loaded();
}

std::vector<JnEndpoint> JNotify::get_endpoints()
{
  return this->config.get_endpoints();
}

int JNotify::run()
{
  if (!this->config.is_loaded() && !this->init_config()) {
    std::cerr << "Unable to load configuration" << std::endl;
    return 1;
  }

  const auto endpoints = this->config.get_endpoints();
  if (endpoints.empty()) {
    std::cout << "No endpoints to listen for" << std::endl;
    return 2;
  }

  for (const auto &endpoint : this->config.get_endpoints()) {
    this->register_url(endpoint.url, [=](const std::string &html) {
      std::cout << html << std::endl;
      rapidjson::Document doc;
      doc.Parse(html.c_str());

      std::stringstream ss;
      if (doc.IsObject()) {
        auto last_build = doc["lastBuild"]["number"].GetInt();
        auto last_failed = doc["lastFailedBuild"]["number"].GetInt();
        if (last_build == last_failed) {
          ss << "Build #" << last_build  << " Failed for " << doc["fullDisplayName"].GetString();
        }
        else {
          ss << doc["fullDisplayName"].GetString() << " is good.";
        }
      }
      else {
        ss << "ERROR: Url is not JSON: " << endpoint.url;
        std::cerr << "URL " << endpoint.url << " contained: " << '\n' << html << std::endl;
      }
      this->emit_notification(ss.str());
    });
  }

  /*
  bool quit = false;
  std::chrono::time_point clock_last;

  while (!quit) {
    for (const auto &endpoint : this->config.get_endpoints()) {
      endpoint.poll_rate
    }
    clock_last = std::chrono::high_resolution_clock::now();
    // TODO: Query endpoints on an interval specified by poll_rate of each endpoint
  }
   */
  this->force_query_endpoints();

  return 0;
}
