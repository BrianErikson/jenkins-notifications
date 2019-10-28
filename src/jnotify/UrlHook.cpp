#include <utility>
#include <curl/curl.h>
#include <cassert>
#include <iostream>
#include "UrlHook.h"

using namespace url_notify;

UrlHook::UrlHook(std::string url, UrlHookCallback callback, long long poll_rate) :
    poll_rate(poll_rate),
    next_execution(TimeType::now() + std::chrono::milliseconds(poll_rate)),
    callback(std::move(callback)),
    url(std::move(url))
{
  this->curl_handle = curl_easy_init();
  assert(this->curl_handle);
  curl_easy_setopt(this->curl_handle, CURLOPT_URL, this->url.c_str());
  curl_easy_setopt(this->curl_handle, CURLOPT_WRITEFUNCTION, &UrlHook::write_callback);
  curl_easy_setopt(this->curl_handle, CURLOPT_WRITEDATA, this->write_buf);
  curl_easy_setopt(this->curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(this->curl_handle, CURLOPT_TIMEOUT, 5L);
}

UrlHook::~UrlHook()
{
  curl_easy_cleanup(this->curl_handle);
  delete this->write_buf;
}

size_t UrlHook::write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
  auto *data = static_cast<std::string*>(userdata);
  assert(size == 1);
  data->append(ptr, nmemb);

  return nmemb;
}

std::string UrlHook::try_get()
{
  CURLcode res = curl_easy_perform(this->curl_handle);
  if (res != CURLE_OK) {
    std::cerr << "GET failed: " << curl_easy_strerror(res) << " for URL " << this->url << std::endl;
  }

  std::string data = *this->write_buf;
  this->write_buf->clear();
  return data;
}

bool UrlHook::execute()
{
  auto data = this->try_get();
  this->next_execution = TimeType::now() + std::chrono::milliseconds(this->poll_rate);

  if (data.empty()) {
    return false;
  }

  return this->callback(data);
}

TimePoint UrlHook::get_next_execution()
{
  return this->next_execution;
}
