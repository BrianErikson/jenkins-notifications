#pragma once
#include <string>
#include <functional>

namespace jn {
using UlCallback = std::function<void (const std::string &html)>;
using UlHandle = void*;

class UrlListener
{
public:
  UrlListener(std::string url, UlCallback callback, int poll_rate);
  ~UrlListener();

  bool execute();
  std::string try_get();

  jn::UlCallback callback;
  int poll_rate;

private:
  static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);

  const std::string url;
  UlHandle curl_handle = nullptr;
  std::string *write_buf = new std::string;
};
}
