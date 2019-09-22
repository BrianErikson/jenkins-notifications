#pragma once
#include <memory>
#include <vector>
#include <string>
#include "UrlListener.h"

using CurlHandle = void*;

class JNotify
{
public:
  JNotify();
  ~JNotify();

  bool emit_notification(const std::string &message);
  void query_endpoints();
  void register_url(const std::string &url, const UlCallback &callback);

private:
  std::vector<std::unique_ptr<UrlListener>> curl_handles{};

};
