#include "water_valve.h"

#include "config.h"
#include "rc_input.h"

void WaterValve::begin() {
  ledcSetup(Config::WATER_SERVO_LEDC_CHANNEL, Config::WATER_SERVO_HZ, Config::WATER_SERVO_BITS);
  ledcAttachPin(Config::WATER_SERVO_PIN, Config::WATER_SERVO_LEDC_CHANNEL);
  writeMicroseconds(Config::WATER_SERVO_DEFAULT_US);
}

void WaterValve::setFromRcPulse(uint16_t pulseUs, bool valid) {
  if (!valid) {
    writeMicroseconds(Config::WATER_SERVO_DEFAULT_US);
    return;
  }

  const float value = RcInput::normalizeUnipolar(pulseUs);
  const uint16_t outputUs = Config::WATER_SERVO_CLOSED_US +
                            static_cast<uint16_t>(value * (Config::WATER_SERVO_OPEN_US -
                                                           Config::WATER_SERVO_CLOSED_US));
  writeMicroseconds(outputUs);
}

void WaterValve::writeMicroseconds(uint16_t pulseUs) {
  outputPulseUs_ = constrain(pulseUs, Config::WATER_SERVO_CLOSED_US, Config::WATER_SERVO_OPEN_US);
  const uint32_t periodUs = 1000000UL / Config::WATER_SERVO_HZ;
  const uint32_t maxDuty = (1UL << Config::WATER_SERVO_BITS) - 1UL;
  const uint32_t duty = (static_cast<uint32_t>(outputPulseUs_) * maxDuty) / periodUs;
  ledcWrite(Config::WATER_SERVO_LEDC_CHANNEL, duty);
}
