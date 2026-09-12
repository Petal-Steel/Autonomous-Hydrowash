#include <Arduino.h>

#include "config.h"
#include "differential_drive.h"
#include "mode_manager.h"
#include "ota_update.h"
#include "pi_uart.h"
#include "rc_input.h"
#include "safety.h"
#include "stepper_drive.h"
#include "water_valve.h"

RcInput rcInput;
Safety safety;
StepperDrive stepperDrive;
WaterValve waterValve;
ModeManager modeManager;
PiUart piUart;
OtaUpdate otaUpdate;

void runMotorSignalTest() {
  static bool initialized = false;
  static uint32_t bootMs = 0;
  static uint32_t lastStepUs = 0;
  static uint32_t pulseStartUs = 0;
  static bool pulseActive = false;

  if (!initialized) {
    initialized = true;
    bootMs = millis();

    pinMode(Config::LEFT_STEP_PIN, OUTPUT);
    pinMode(Config::LEFT_DIR_PIN, OUTPUT);
    pinMode(Config::RIGHT_STEP_PIN, OUTPUT);
    pinMode(Config::RIGHT_DIR_PIN, OUTPUT);

    digitalWrite(Config::LEFT_STEP_PIN, HIGH);
    digitalWrite(Config::RIGHT_STEP_PIN, HIGH);
    digitalWrite(Config::LEFT_DIR_PIN, HIGH);
    digitalWrite(Config::RIGHT_DIR_PIN, HIGH);

    Serial.println("MOTOR SIGNAL TEST ENABLED");
    Serial.println("Both STEP pins will pulse after 3 seconds. RC is ignored.");
  }

  if ((millis() - bootMs) < Config::MOTOR_SIGNAL_TEST_DELAY_MS) {
    return;
  }

  const uint32_t nowUs = micros();
  if (pulseActive) {
    if ((nowUs - pulseStartUs) >= Config::MOTOR_SIGNAL_TEST_PULSE_WIDTH_US) {
      digitalWrite(Config::LEFT_STEP_PIN, HIGH);
      digitalWrite(Config::RIGHT_STEP_PIN, HIGH);
      pulseActive = false;
    }
    return;
  }

  if ((nowUs - lastStepUs) >= Config::MOTOR_SIGNAL_TEST_STEP_INTERVAL_US) {
    digitalWrite(Config::LEFT_STEP_PIN, LOW);
    digitalWrite(Config::RIGHT_STEP_PIN, LOW);
    pulseStartUs = nowUs;
    lastStepUs = nowUs;
    pulseActive = true;
  }
}

void printDebug(const RcChannels &rc,
                float throttle,
                float steering,
                const TankCommand &targetRpm,
                const SafetyState &safetyState) {
  Serial.print("RC thr=");
  Serial.print(rc.throttleUs);
  Serial.print("us steer=");
  Serial.print(rc.steeringUs);
  Serial.print("us water=");
  Serial.print(rc.waterUs);
  Serial.print("us norm thr=");
  Serial.print(throttle, 3);
  Serial.print(" steer=");
  Serial.print(steering, 3);
  Serial.print(" target L=");
  Serial.print(targetRpm.left, 1);
  Serial.print("rpm R=");
  Serial.print(targetRpm.right, 1);
  Serial.print("rpm current L=");
  Serial.print(stepperDrive.leftCurrentRpm(), 1);
  Serial.print("rpm R=");
  Serial.print(stepperDrive.rightCurrentRpm(), 1);
  Serial.print("rpm valve=");
  Serial.print(waterValve.outputPulseUs());
  Serial.print("us failsafe=");
  Serial.println(safetyState.rcFailsafe ? "YES" : "NO");
}

void runPiUartTxScopeTest() {
  static bool initialized = false;
  static bool levelHigh = true;
  static uint32_t lastToggleMs = 0;

  if (!initialized) {
    initialized = true;
    pinMode(Config::PI_UART_TX_PIN, OUTPUT);
    digitalWrite(Config::PI_UART_TX_PIN, HIGH);
    Serial.println("PI UART TX SCOPE TEST ENABLED");
    Serial.println("GPIO17 toggles every 500 ms. RC drive is ignored.");
  }

  const uint32_t nowMs = millis();
  if ((nowMs - lastToggleMs) >= Config::PI_UART_TX_SCOPE_TEST_INTERVAL_MS) {
    lastToggleMs = nowMs;
    levelHigh = !levelHigh;
    digitalWrite(Config::PI_UART_TX_PIN, levelHigh ? HIGH : LOW);
  }
}

void runMotorAutoDirectionTest() {
  static bool initialized = false;
  static uint32_t bootMs = 0;

  if (!initialized) {
    initialized = true;
    bootMs = millis();
    Serial.println("MOTOR AUTO DIRECTION TEST ENABLED");
    Serial.println("Sequence repeats: left +, left -, right +, right -. RC drive is ignored.");
  }

  const uint32_t nowMs = millis();
  if ((nowMs - bootMs) < Config::MOTOR_AUTO_DIRECTION_TEST_START_DELAY_MS) {
    stepperDrive.stopNow();
    return;
  }

  const uint32_t phase =
      ((nowMs - bootMs - Config::MOTOR_AUTO_DIRECTION_TEST_START_DELAY_MS) /
       Config::MOTOR_AUTO_DIRECTION_TEST_STEP_MS) %
      4;

  const float rpm = Config::MOTOR_AUTO_DIRECTION_TEST_RPM;
  switch (phase) {
  case 0:
    stepperDrive.setTargetRpm(rpm, 0.0f);
    break;
  case 1:
    stepperDrive.setTargetRpm(-rpm, 0.0f);
    break;
  case 2:
    stepperDrive.setTargetRpm(0.0f, rpm);
    break;
  default:
    stepperDrive.setTargetRpm(0.0f, -rpm);
    break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  rcInput.begin();
  stepperDrive.begin();
  waterValve.begin();
  piUart.begin();
  otaUpdate.begin();
  modeManager.forceManual();

  Serial.println("ESP32 pressure-washing rover controller ready");
  Serial.println("Mode: manual RC");
}

void loop() {
  if (Config::PI_UART_TX_SCOPE_TEST_ENABLED) {
    runPiUartTxScopeTest();
    return;
  }

  if (Config::MOTOR_SIGNAL_TEST_ENABLED) {
    runMotorSignalTest();
    return;
  }

  stepperDrive.update();
  otaUpdate.handle();
  if (otaUpdate.isUpdating()) {
    stepperDrive.stopNow();
    return;
  }

  piUart.update();

  if (Config::MOTOR_AUTO_DIRECTION_TEST_ENABLED) {
    runMotorAutoDirectionTest();
    return;
  }

  static uint32_t lastControlMs = 0;
  const uint32_t nowMs = millis();
  if ((nowMs - lastControlMs) < Config::CONTROL_UPDATE_INTERVAL_MS) {
    return;
  }
  lastControlMs = nowMs;

  const RcChannels rc = rcInput.read();
  const SafetyState safetyState = safety.updateManual(rc);

  static float throttle = 0.0f;
  static float steering = 0.0f;
  static TankCommand targetRpm;

  if (modeManager.mode() == DriveMode::ManualRc && safetyState.outputAllowed) {
    const float throttleSign = Config::RC_THROTTLE_INVERT ? -1.0f : 1.0f;
    const float steeringSign = Config::RC_STEERING_INVERT ? -1.0f : 1.0f;
    const float rawThrottle =
        RcInput::normalizeCentered(rc.throttleUs) * throttleSign * Config::MAX_THROTTLE_INPUT;
    const float rawSteering =
        RcInput::normalizeCentered(rc.steeringUs) * steeringSign * Config::MAX_STEERING_INPUT;
    throttle += (rawThrottle - throttle) * Config::RC_COMMAND_FILTER_ALPHA;
    steering += (rawSteering - steering) * Config::RC_COMMAND_FILTER_ALPHA;

    if (Config::MOTOR_DIRECTION_CALIBRATION_ENABLED) {
      targetRpm.left = throttle * Config::MOTOR_DIRECTION_CALIBRATION_RPM;
      targetRpm.right = steering * Config::MOTOR_DIRECTION_CALIBRATION_RPM;
    } else {
      targetRpm = DifferentialDrive::normalizedToRpm(DifferentialDrive::mix(throttle, steering));
    }

    stepperDrive.setTargetRpm(targetRpm.left, targetRpm.right);
  } else {
    throttle = 0.0f;
    steering = 0.0f;
    targetRpm = {};
    stepperDrive.stopNow();
  }

  waterValve.setFromRcPulse(rc.waterUs, rc.waterValid && !safetyState.rcFailsafe);

  static uint32_t lastDebugMs = 0;
  if (Config::SERIAL_DEBUG_ENABLED && (nowMs - lastDebugMs) >= Config::DEBUG_INTERVAL_MS) {
    lastDebugMs = nowMs;
    printDebug(rc, throttle, steering, targetRpm, safetyState);
  }
}
