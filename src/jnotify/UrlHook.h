#pragma once
#include <string>
#include <functional>
#include <chrono>

namespace url_notify {
using UrlHookCallback = std::function<bool (const std::string &html)>;
using CurlHandle = void*;
using TimeType = std::chrono::steady_clock;
using TimePoint = TimeType::time_point;

class UrlHook
{
public:
  UrlHook(std::string url, UrlHookCallback callback, long long poll_rate);
  ~UrlHook();

  TimePoint get_next_execution();
  bool execute();
  std::string try_get();

  const long long poll_rate;

private:
  static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);

  TimePoint next_execution;
  UrlHookCallback callback;
  const std::string url;
  CurlHandle curl_handle = nullptr;
  std::string *write_buf = new std::string;
};

}
