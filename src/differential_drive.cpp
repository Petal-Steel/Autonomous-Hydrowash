#include "differential_drive.h"

#include <Arduino.h>

#include "config.h"

TankCommand DifferentialDrive::mix(float throttle, float steering) {
  TankCommand command;
  command.left = throttle + steering;
  command.right = throttle - steering;

  const float maxMagnitude = max(abs(command.left), abs(command.right));
  if (maxMagnitude > 1.0f) {
    command.left /= maxMagnitude;
    command.right /= maxMagnitude;
  }

  return command;
}

TankCommand DifferentialDrive::normalizedToRpm(const TankCommand &command) {
  TankCommand rpm;
  rpm.left = command.left * Config::MAX_WHEEL_RPM;
  rpm.right = command.right * Config::MAX_WHEEL_RPM;
  return rpm;
}

