#pragma once
#include <memory>
#include <vector>
#include <string>
#include "UrlListener.h"
#include "JnConfig.h"

using CurlHandle = void*;

class JNotify
{
public:
  JNotify();
  ~JNotify();

  bool emit_notification(const std::string &message);
  void force_query_endpoints();
  void register_url(const std::string &url, const UlCallback &callback);
  bool init_config();
  std::vector<JnEndpoint> get_endpoints();

  /// Blocking call for the runtime
  int run();

private:
  std::vector<std::unique_ptr<UrlListener>> curl_handles{};
  JnConfig config;
};
