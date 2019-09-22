#pragma once

struct JnEndpoint {
  /// Url to poll
  std::string url;
  /// Rate to poll the endpoint at the url, in minutes;
  unsigned int poll_rate;
};
