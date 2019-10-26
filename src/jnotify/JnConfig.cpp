#include <iostream>
#include "JnConfig.h"
#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

using namespace libconfig;

std::string JnConfig::get_config(const std::string &cfg_path)
{
  std::string err_str;
  if (!cfg_path.empty()) {
    try {
      config.readFile(cfg_path.c_str());
      return "";
    }
    catch (const ParseException &ex) {
      err_str = ex.what();
    }
    catch (const FileIOException &ex) {
      err_str = ex.what();
    }

    return err_str;
  }
  else {
    try {
      auto path = JnConfig::get_home_dir().append("/.config/jnotify/jnotify.cfg");
      config.readFile(path.c_str());
      return "";
    }
    catch (const ParseException &ex) {
      err_str = ex.what();
    }
    catch (const FileIOException &ex) {
      err_str = ex.what();
    }

    try {
      config.readFile("/etc/jnotify/jnotify.cfg");
      return "";
    }
    catch (const ParseException &ex) {
      err_str = ex.what();
    }
    catch (const FileIOException &ex) {
      err_str = ex.what();
    }
  }

  return err_str;
}

std::string JnConfig::get_home_dir()
{
#ifdef __linux__
  return getpwuid(getuid())->pw_dir;
#else
  std::cerr << "Platform not supported!" std::endl;
  return "";
#endif
}

std::string JnConfig::try_load(const std::string &cfg_path)
{
  auto ret_str = this->get_config(cfg_path);
  if (!ret_str.empty()) {
    return ret_str;
  }

  const Setting &root = this->config.getRoot();
  try {
    const Setting &config_endpoints = root["endpoints"];
    for (int i = 0; i < config_endpoints.getLength(); i++) {
      const Setting &config_endpoint = config_endpoints[i];

      JnEndpoint endpoint;
      if (!config_endpoint.lookupValue("url", endpoint.url)) {
        return "endpoint has no url";
      }
      else if (!config_endpoint.lookupValue("poll_rate", endpoint.poll_rate)) {
        return "endpoint has no poll rate";
      }

      this->endpoints.push_back(std::move(endpoint));
    }
  }
  catch (const SettingException &ex) {
    return ex.what();
  }

  this->loaded = true;
  return ret_str;
}

std::vector<JnEndpoint> JnConfig::get_endpoints()
{
  if (!this->loaded) {
    auto res = this->try_load();
    if (!res.empty()) {
      std::cerr << res << std::endl;
      return this->endpoints;
    }
  }

  return this->endpoints;
}

bool JnConfig::is_loaded()
{
  return this->loaded;
}
