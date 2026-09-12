#pragma once

#include <Arduino.h>

class WaterValve {
public:
  void begin();
  void setFromRcPulse(uint16_t pulseUs, bool valid);
  uint16_t outputPulseUs() const { return outputPulseUs_; }

private:
  void writeMicroseconds(uint16_t pulseUs);

  uint16_t outputPulseUs_ = 1806;
};
