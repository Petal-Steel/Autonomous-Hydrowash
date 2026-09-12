#pragma once

#include <Arduino.h>

class PiUart {
public:
  void begin();
  void update();

private:
  void handleLine(const char *line);

  char lineBuffer_[80] = {};
  size_t lineLength_ = 0;
  uint32_t lastHeartbeatMs_ = 0;
};
