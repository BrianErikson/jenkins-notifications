#pragma once
#include <vector>
#include <libconfig.h++>

namespace url_notify {
struct Endpoint {
  /// Url to poll
  std::string url;
  /// Rate to poll the endpoint at the url, in milliseconds
  long long poll_rate;
};

class Config
{
public:
  /// \brief Attempt config initialization.
  /// \param cfg_path: Use a specific configuration path, if specified
  /// Returns an error string if failed, else empty
  std::string try_load(const std::string &cfg_path = "");

  bool is_loaded();
  std::vector<Endpoint> get_endpoints();

private:
  static std::string get_home_dir();
  std::string get_config(const std::string &cfg_path = "");

  bool loaded = false;
  libconfig::Config config;
  std::vector<Endpoint> endpoints;
};

}
