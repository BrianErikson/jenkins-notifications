#pragma once
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include "UrlHook.h"
#include "Config.h"

using CurlHandle = void*;
using SharedUrlHook = std::shared_ptr<url_notify::UrlHook>;

class JNotify
{
public:
  JNotify();
  ~JNotify();

  static bool emit_notification(const std::string &message);
  void force_query_endpoints();
  SharedUrlHook register_endpoint(const std::string &url, unsigned int poll_rate,
                                  const url_notify::UrlHookCallback &callback);
  bool init_config();
  std::vector<url_notify::Endpoint> get_endpoints();

  /// Blocking call for the runtime
  int run();

private:
  static bool jenkins_trigger(const std::string &json);
  SharedUrlHook& get_next_hook();

  std::vector<SharedUrlHook> hooks{};
  url_notify::Config config;
};
