#include "safety.h"

SafetyState Safety::updateManual(const RcChannels &channels) {
  SafetyState state;
  state.rcFailsafe = !channels.driveValid;
  state.piTimeout = false;
  state.outputAllowed = !state.rcFailsafe;
  return state;
}

