#include <curl/curl.h>
#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <memory>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <rapidjson/document.h>
#include <sstream>
#include <thread>
#include <atomic>
#include <list>
#include "JNotify.h"
#include "JnConfig.h"
#ifdef __linux__
#include <csignal>
#endif

using namespace jn;

JNotify::JNotify()
{
  curl_global_init(CURL_GLOBAL_ALL);
  notify_init("JNotify");
}

JNotify::~JNotify()
{
  // Free curl handles before deinit curl
  this->url_listeners.erase(this->url_listeners.begin(), this->url_listeners.end());
  notify_uninit();
  curl_global_cleanup();
}

int queue_size = 10;
std::list<std::string> prev_notifications{};
bool JNotify::emit_notification(const std::string &message)
{
  for (const auto &prev : prev_notifications) {
    if (prev == message) {
      return false;
    }
  }

  prev_notifications.push_front(message);
  while (prev_notifications.size() > queue_size) {
    prev_notifications.pop_back();
  }

  NotifyNotification *notification = notify_notification_new("jnotify",
      message.c_str(), "");
  bool retval = notify_notification_show(notification, nullptr);

  g_object_unref (G_OBJECT(notification));
  return retval;
}

SharedUrlListener JNotify::register_endpoint(const std::string &url, const UlCallback &callback,
                                             unsigned int poll_rate)
{
  SharedUrlListener listener = std::make_shared<UrlListener>(url, callback, poll_rate);
  this->url_listeners.push_back(listener);
  return listener;
}

void JNotify::force_query_endpoints()
{
  for (auto &listener : this->url_listeners) {
    listener->execute();
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

std::vector<Endpoint> JNotify::get_endpoints()
{
  return this->config.get_endpoints();
}

TimePoint JNotify::get_next_query_time()
{
  TimePoint shortest = TimePoint::max();
  for (const auto &pair : this->timeouts) {
    if (pair.second < shortest) {
      shortest = pair.second;
    }
  }

  return shortest;
}

void JNotify::jenkins_trigger(const std::string &json)
{
  rapidjson::Document doc;
  doc.Parse(json.c_str());

  std::stringstream ss;
  if (doc.IsObject()) {
    auto last_build = doc["lastBuild"]["number"].GetInt();
    auto last_failed = doc["lastFailedBuild"]["number"].GetInt();
    if (last_build == last_failed) {
      ss << "Build #" << last_build << " failed for " << doc["fullDisplayName"].GetString();
    } else {
      ss << doc["fullDisplayName"].GetString() << " has no failed builds.";
    }
  } else {
    ss << "ERROR: One of the Jenkins urls provided did not contain json.";
    std::cerr << "ERROR: invalid json returned:\n" << json << std::endl;
  }

  JNotify::emit_notification(ss.str());
}

std::atomic<bool> quit = false;
void on_exit(int sig) {
  quit = true;
}

int JNotify::run()
{
#ifdef __linux__
  signal(SIGINT, on_exit);
  signal(SIGABRT, on_exit);
  signal(SIGALRM, on_exit);
  signal(SIGFPE, on_exit);
  signal(SIGHUP, on_exit);
  signal(SIGILL, on_exit);
  signal(SIGKILL, on_exit);
  signal(SIGPIPE, on_exit);
  signal(SIGQUIT, on_exit);
  signal(SIGSEGV, on_exit);
  signal(SIGTERM, on_exit);
  signal(SIGUSR1, on_exit);
  signal(SIGUSR2, on_exit);
#endif

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
    this->register_endpoint(endpoint.url, &JNotify::jenkins_trigger, endpoint.poll_rate);
  }

  queue_size = this->url_listeners.size();

  this->force_query_endpoints();
  auto now = std::chrono::steady_clock::now();

  for (const auto &listener : this->url_listeners) {
    this->timeouts[listener] = now + std::chrono::minutes(listener->poll_rate);
  }

  TimePoint next_query = this->get_next_query_time();
  while (!quit) {
    if (now >= next_query) {
      auto iter = std::find_if(this->timeouts.begin(), this->timeouts.end(),
          [&](const auto &pair) {
            return pair.second == next_query;
          });

      if (iter != this->timeouts.end()) {
        auto listener = (*iter).first;
        listener->execute();
        (*iter).second = now + std::chrono::minutes(listener->poll_rate);
      }
      else {
        throw std::runtime_error("Timeout requested, but listener not found! Aborting.");
      }
    }

    now = std::chrono::steady_clock::now();
    next_query = this->get_next_query_time();
    while (now < next_query) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      now = std::chrono::steady_clock::now();
    }
  }

  return 0;
}
