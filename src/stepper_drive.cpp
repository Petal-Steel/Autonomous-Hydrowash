#include "stepper_drive.h"

#include "config.h"

namespace {

void setupCommonPositiveInput(uint8_t pin) {
  pinMode(pin, OUTPUT_OPEN_DRAIN);
  digitalWrite(pin, HIGH);
}

void writeCommonPositiveInput(uint8_t pin, bool active) {
  digitalWrite(pin, active ? LOW : HIGH);
}

float rpmToStepRate(float rpm) {
  return abs(rpm) * Config::WHEEL_PULSES_PER_REV / 60.0f;
}

void setDriversEnabled(bool enabled) {
  if (!Config::USE_ENABLE_PIN) {
    return;
  }

  if (Config::ENABLE_LOW_DISABLES_DRIVER) {
    if (enabled) {
      pinMode(Config::ENABLE_PIN, INPUT);
    } else {
      pinMode(Config::ENABLE_PIN, OUTPUT);
      digitalWrite(Config::ENABLE_PIN, LOW);
    }
    return;
  }

  pinMode(Config::ENABLE_PIN, OUTPUT);
  digitalWrite(Config::ENABLE_PIN, enabled ? LOW : HIGH);
}

} // namespace

void StepperMotor::begin(const StepperPins &pins, bool invertDirection) {
  pins_ = pins;
  invertDirection_ = invertDirection;

  setupCommonPositiveInput(pins_.stepPin);
  setupCommonPositiveInput(pins_.dirPin);
  writeStep(false);
  directionForward_ = false;
  setDirection(true, micros());

  lastRampUs_ = micros();
  lastStepUs_ = lastRampUs_;
  initialized_ = true;
}

void StepperMotor::setTargetRpm(float rpm) {
  targetRpm_ = constrain(rpm, -Config::MAX_WHEEL_RPM, Config::MAX_WHEEL_RPM);
}

void StepperMotor::stopNow() {
  targetRpm_ = 0.0f;
  currentRpm_ = 0.0f;
  stepRateHz_ = 0.0f;
  writeStep(false);
}

void StepperMotor::update(uint32_t nowUs) {
  if (!initialized_) {
    return;
  }

  const float dtSec = static_cast<float>(nowUs - lastRampUs_) / 1000000.0f;
  lastRampUs_ = nowUs;

  const float maxDelta = Config::ACCEL_RPM_PER_SEC * dtSec;
  const float delta = targetRpm_ - currentRpm_;
  if (abs(delta) <= maxDelta) {
    currentRpm_ = targetRpm_;
  } else {
    currentRpm_ += delta > 0.0f ? maxDelta : -maxDelta;
  }

  stepRateHz_ = rpmToStepRate(currentRpm_);
  if (stepRateHz_ < Config::MIN_STEP_RATE_HZ) {
    writeStep(false);
    return;
  }

  setDirection(currentRpm_ >= 0.0f, nowUs);

  if (pulseActive_) {
    if ((nowUs - pulseStartUs_) >= Config::STEP_PULSE_WIDTH_US) {
      writeStep(false);
    }
    return;
  }

  if ((nowUs - dirChangedUs_) < Config::DIR_SETUP_US) {
    return;
  }

  const uint32_t intervalUs = static_cast<uint32_t>(1000000.0f / stepRateHz_);
  if ((nowUs - lastStepUs_) >= intervalUs) {
    writeStep(true);
    pulseStartUs_ = nowUs;
    lastStepUs_ = nowUs;
  }
}

void StepperMotor::setDirection(bool forward, uint32_t nowUs) {
  if (forward == directionForward_) {
    return;
  }

  directionForward_ = forward;
  const bool logicalForward = invertDirection_ ? !forward : forward;
  writeCommonPositiveInput(pins_.dirPin, logicalForward);
  dirChangedUs_ = nowUs;
}

void StepperMotor::writeStep(bool active) {
  pulseActive_ = active;
  writeCommonPositiveInput(pins_.stepPin, active);
}

void StepperDrive::begin() {
  if (Config::USE_ENABLE_PIN) {
    setDriversEnabled(true);
  }

  left_.begin({Config::LEFT_STEP_PIN, Config::LEFT_DIR_PIN}, Config::LEFT_MOTOR_INVERT_DIRECTION);
  right_.begin({Config::RIGHT_STEP_PIN, Config::RIGHT_DIR_PIN}, Config::RIGHT_MOTOR_INVERT_DIRECTION);
}

void StepperDrive::setTargetRpm(float leftRpm, float rightRpm) {
  setDriversEnabled(true);
  left_.setTargetRpm(leftRpm);
  right_.setTargetRpm(rightRpm);
}

void StepperDrive::stopNow() {
  setDriversEnabled(false);
  left_.stopNow();
  right_.stopNow();
}

void StepperDrive::update() {
  const uint32_t nowUs = micros();
  left_.update(nowUs);
  right_.update(nowUs);
}
