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
  std::vector<JnEndpoint> get_endpoints();

private:
  std::string get_config(const std::string &cfg_path = "");

  bool loaded = false;
  libconfig::Config config;
  std::vector<JnEndpoint> endpoints;
};