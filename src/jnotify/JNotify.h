#pragma once

#include <string>

class JNotify
{
public:
  JNotify();
  ~JNotify();

  bool emit_notification(const std::string &message);
};
