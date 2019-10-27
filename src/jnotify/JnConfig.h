#pragma once
#include <vector>
#include <libconfig.h++>
#include "common.h"

class JnConfig
{
public:
  /// \brief Attempt config initialization.
  /// \param cfg_path: Use a specific configuration path, if specified
  /// Returns an error string if failed, else empty
  std::string try_load(const std::string &cfg_path = "");

  bool is_loaded();
  std::vector<jn::Endpoint> get_endpoints();

private:
  static std::string get_home_dir();
  std::string get_config(const std::string &cfg_path = "");

  bool loaded = false;
  libconfig::Config config;
  std::vector<jn::Endpoint> endpoints;
};
