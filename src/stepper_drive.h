#pragma once

#include <Arduino.h>

struct StepperPins {
  uint8_t stepPin;
  uint8_t dirPin;
};

class StepperMotor {
public:
  void begin(const StepperPins &pins, bool invertDirection);
  void setTargetRpm(float rpm);
  void stopNow();
  void update(uint32_t nowUs);

  float targetRpm() const { return targetRpm_; }
  float currentRpm() const { return currentRpm_; }
  float stepRateHz() const { return stepRateHz_; }

private:
  void setDirection(bool forward, uint32_t nowUs);
  void writeStep(bool active);

  StepperPins pins_{};
  bool invertDirection_ = false;
  bool directionForward_ = true;
  bool pulseActive_ = false;
  bool initialized_ = false;
  uint32_t lastRampUs_ = 0;
  uint32_t lastStepUs_ = 0;
  uint32_t pulseStartUs_ = 0;
  uint32_t dirChangedUs_ = 0;
  float targetRpm_ = 0.0f;
  float currentRpm_ = 0.0f;
  float stepRateHz_ = 0.0f;
};

class StepperDrive {
public:
  void begin();
  void setTargetRpm(float leftRpm, float rightRpm);
  void stopNow();
  void update();

  float leftCurrentRpm() const { return left_.currentRpm(); }
  float rightCurrentRpm() const { return right_.currentRpm(); }
  float leftTargetRpm() const { return left_.targetRpm(); }
  float rightTargetRpm() const { return right_.targetRpm(); }

private:
  StepperMotor left_;
  StepperMotor right_;
};

