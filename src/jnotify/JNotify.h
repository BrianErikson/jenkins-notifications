#pragma once
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>
#include "common.h"
#include "UrlListener.h"
#include "JnConfig.h"

using CurlHandle = void*;
using SharedUrlListener = std::shared_ptr<jn::UrlListener>;
using TimePoint = std::chrono::steady_clock::time_point;

class JNotify
{
public:
  JNotify();
  ~JNotify();

  static bool emit_notification(const std::string &message);
  void force_query_endpoints();
  SharedUrlListener register_endpoint(const std::string &url, const jn::UlCallback &callback,
                                      unsigned int poll_rate);
  bool init_config();
  std::vector<jn::Endpoint> get_endpoints();

  /// Blocking call for the runtime
  int run();

private:
  static void jenkins_trigger(const std::string &json);
  TimePoint get_next_query_time();

  std::vector<SharedUrlListener> url_listeners{};
  std::unordered_map<SharedUrlListener, TimePoint> timeouts;
  JnConfig config;
};
