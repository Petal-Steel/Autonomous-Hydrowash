#pragma once

#include "rc_input.h"

struct SafetyState {
  bool rcFailsafe = true;
  bool piTimeout = false;
  bool outputAllowed = false;
};

class Safety {
public:
  SafetyState updateManual(const RcChannels &channels);
};

