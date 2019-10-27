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
#include "Config.h"
#ifdef __linux__
#include <csignal>
#endif

using namespace url_notify;

JNotify::JNotify()
{
  curl_global_init(CURL_GLOBAL_ALL);
  notify_init("JNotify");
}

JNotify::~JNotify()
{
  // Free curl handles before deinit curl
  this->hooks.erase(this->hooks.begin(), this->hooks.end());
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

SharedUrlHook JNotify::register_endpoint(const std::string &url, unsigned int poll_rate,
                                         const url_notify::UrlHookCallback &callback)
{
  SharedUrlHook listener = std::make_shared<UrlHook>(url, callback, poll_rate);
  this->hooks.push_back(listener);
  return listener;
}

void JNotify::force_query_endpoints()
{
  for (auto &hook : this->hooks) {
    hook->execute();
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

SharedUrlHook& JNotify::get_next_hook()
{
  SharedUrlHook &shortest = this->hooks[0];
  for (const auto &hook : this->hooks) {
    if (hook->get_next_execution() < shortest->get_next_execution()) {
      shortest = hook;
    }
  }

  return shortest;
}

bool JNotify::jenkins_trigger(const std::string &json)
{
  rapidjson::Document doc;
  doc.Parse(json.c_str());

  bool err = true;
  std::stringstream ss;
  if (doc.IsObject()) {
    auto last_build = doc["lastBuild"]["number"].GetInt();
    auto last_failed = doc["lastFailedBuild"]["number"].GetInt();
    if (last_build == last_failed) {
      ss << "Build #" << last_build << " failed for " << doc["fullDisplayName"].GetString();
    } else {
      ss << "Build #" << last_build << " passed for " << doc["fullDisplayName"].GetString();
    }
    err = false;
  } else {
    ss << "ERROR: One of the Jenkins URLs provided did not contain json.";
    std::cerr << "ERROR: invalid json returned:\n" << json << std::endl;
  }

  JNotify::emit_notification(ss.str());
  return err;
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
    this->register_endpoint(endpoint.url, endpoint.poll_rate, &JNotify::jenkins_trigger);
  }

  queue_size = this->hooks.size();

  this->force_query_endpoints();
  auto now = std::chrono::steady_clock::now();

  SharedUrlHook &next_hook = this->get_next_hook();
  while (!quit) {
    if (now >= next_hook->get_next_execution()) {
      next_hook->execute();
    }

    now = std::chrono::steady_clock::now();
    next_hook = this->get_next_hook();
    auto next_exec = next_hook->get_next_execution();
    while (now < next_exec) {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      now = std::chrono::steady_clock::now();
    }
  }

  return 0;
}
