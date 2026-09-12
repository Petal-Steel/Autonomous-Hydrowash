#pragma once

#include <Arduino.h>

struct RcChannels {
  uint16_t throttleUs = 1500;
  uint16_t steeringUs = 1500;
  uint16_t waterUs = 1000;
  bool throttleValid = false;
  bool steeringValid = false;
  bool waterValid = false;
  bool driveValid = false;
};

class RcInput {
public:
  void begin();
  RcChannels read() const;

  static float normalizeCentered(uint16_t pulseUs);
  static float normalizeUnipolar(uint16_t pulseUs);

private:
  static bool pulseValid(uint16_t pulseUs);
};

