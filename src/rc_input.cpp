#include "rc_input.h"

#include "config.h"

namespace {

struct PulseCapture {
  explicit PulseCapture(uint8_t inputPin) : pin(inputPin) {}

  uint8_t pin;
  volatile uint32_t riseUs = 0;
  volatile uint32_t widthUs = 1500;
  volatile uint32_t lastPulseUs = 0;
};

PulseCapture throttleCapture{Config::RC_THROTTLE_PIN};
PulseCapture steeringCapture{Config::RC_STEERING_PIN};
PulseCapture waterCapture{Config::RC_WATER_PIN};

void IRAM_ATTR handlePulse(PulseCapture &capture) {
  const uint32_t now = micros();
  if (digitalRead(capture.pin) == HIGH) {
    capture.riseUs = now;
  } else {
    capture.widthUs = now - capture.riseUs;
    capture.lastPulseUs = now;
  }
}

void IRAM_ATTR throttleIsr() {
  handlePulse(throttleCapture);
}

void IRAM_ATTR steeringIsr() {
  handlePulse(steeringCapture);
}

void IRAM_ATTR waterIsr() {
  handlePulse(waterCapture);
}

uint16_t clampPulse(uint32_t pulseUs) {
  if (pulseUs < Config::RC_VALID_MIN_US) {
    return Config::RC_VALID_MIN_US;
  }
  if (pulseUs > Config::RC_VALID_MAX_US) {
    return Config::RC_VALID_MAX_US;
  }
  return static_cast<uint16_t>(pulseUs);
}

} // namespace

void RcInput::begin() {
  pinMode(Config::RC_THROTTLE_PIN, INPUT);
  pinMode(Config::RC_STEERING_PIN, INPUT);
  pinMode(Config::RC_WATER_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(Config::RC_THROTTLE_PIN), throttleIsr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Config::RC_STEERING_PIN), steeringIsr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Config::RC_WATER_PIN), waterIsr, CHANGE);
}

RcChannels RcInput::read() const {
  RcChannels channels;
  uint32_t now = micros();

  noInterrupts();
  const uint32_t throttleWidth = throttleCapture.widthUs;
  const uint32_t steeringWidth = steeringCapture.widthUs;
  const uint32_t waterWidth = waterCapture.widthUs;
  const uint32_t throttleAge = now - throttleCapture.lastPulseUs;
  const uint32_t steeringAge = now - steeringCapture.lastPulseUs;
  const uint32_t waterAge = now - waterCapture.lastPulseUs;
  interrupts();

  channels.throttleUs = clampPulse(throttleWidth);
  channels.steeringUs = clampPulse(steeringWidth);
  channels.waterUs = clampPulse(waterWidth);

  channels.throttleValid = throttleAge <= Config::RC_SIGNAL_TIMEOUT_US && pulseValid(channels.throttleUs);
  channels.steeringValid = steeringAge <= Config::RC_SIGNAL_TIMEOUT_US && pulseValid(channels.steeringUs);
  channels.waterValid = waterAge <= Config::RC_SIGNAL_TIMEOUT_US && pulseValid(channels.waterUs);
  channels.driveValid = channels.throttleValid && channels.steeringValid;

  return channels;
}

float RcInput::normalizeCentered(uint16_t pulseUs) {
  int32_t delta = static_cast<int32_t>(pulseUs) - Config::RC_CENTER_US;
  if (abs(delta) <= Config::RC_DRIVE_DEADBAND_US) {
    return 0.0f;
  }

  float value;
  if (delta > 0) {
    value = static_cast<float>(delta - Config::RC_DRIVE_DEADBAND_US) /
            static_cast<float>(Config::RC_MAX_US - Config::RC_CENTER_US - Config::RC_DRIVE_DEADBAND_US);
  } else {
    value = static_cast<float>(delta + Config::RC_DRIVE_DEADBAND_US) /
            static_cast<float>(Config::RC_CENTER_US - Config::RC_MIN_US - Config::RC_DRIVE_DEADBAND_US);
  }

  return constrain(value, -1.0f, 1.0f);
}

float RcInput::normalizeUnipolar(uint16_t pulseUs) {
  float value = static_cast<float>(pulseUs - Config::RC_MIN_US) /
                static_cast<float>(Config::RC_MAX_US - Config::RC_MIN_US);
  return constrain(value, 0.0f, 1.0f);
}

bool RcInput::pulseValid(uint16_t pulseUs) {
  return pulseUs >= Config::RC_VALID_MIN_US && pulseUs <= Config::RC_VALID_MAX_US;
}
