#pragma once

#include <Arduino.h>

namespace Config {

// DM556 inputs are wired common-positive:
// PUL+/DIR+/ENA+ to 5 V, ESP32 pin drives the negative input.
// PUL and DIR are active when the ESP32 sinks current by pulling the pin LOW.
// ENA on this setup is a disable input: ENA- released = driver active,
// ENA- pulled LOW = driver disabled.
constexpr bool STEP_ACTIVE_LOW = true;
constexpr bool DIR_ACTIVE_LOW = true;
constexpr bool ENABLE_LOW_DISABLES_DRIVER = true;

constexpr uint8_t LEFT_STEP_PIN = 25;
constexpr uint8_t LEFT_DIR_PIN = 26;
constexpr uint8_t RIGHT_STEP_PIN = 32;
constexpr uint8_t RIGHT_DIR_PIN = 33;
constexpr uint8_t ENABLE_PIN = 27;
constexpr bool USE_ENABLE_PIN = false;
constexpr bool LEFT_MOTOR_INVERT_DIRECTION = true;
constexpr bool RIGHT_MOTOR_INVERT_DIRECTION = false;

constexpr uint8_t RC_THROTTLE_PIN = 23;
constexpr uint8_t RC_STEERING_PIN = 13;
constexpr uint8_t RC_WATER_PIN = 14;

constexpr uint8_t WATER_SERVO_PIN = 18;
constexpr uint8_t WATER_SERVO_LEDC_CHANNEL = 0;
constexpr uint32_t WATER_SERVO_HZ = 50;
constexpr uint8_t WATER_SERVO_BITS = 16;
// Closed moved about 20 degrees toward open from the original 1000 us.
constexpr uint16_t WATER_SERVO_CLOSED_US = 1112;
// Open moved about 35 degrees toward closed from the original 2000 us.
constexpr uint16_t WATER_SERVO_OPEN_US = 1806;
// Resting/default valve position when no valid manual command is present.
constexpr uint16_t WATER_SERVO_DEFAULT_US = WATER_SERVO_OPEN_US;

constexpr uint8_t PI_UART_RX_PIN = 16;
constexpr uint8_t PI_UART_TX_PIN = 17;
constexpr uint32_t PI_UART_BAUD = 115200;

constexpr bool OTA_ENABLED = true;
constexpr uint32_t OTA_WIFI_START_DELAY_MS = 10000;
constexpr uint32_t OTA_WIFI_CONNECT_TIMEOUT_MS = 30000;
constexpr uint32_t OTA_WIFI_RETRY_INTERVAL_MS = 30000;
constexpr const char *OTA_HOSTNAME = "hydrowash-esp32";

constexpr uint16_t RC_MIN_US = 1000;
constexpr uint16_t RC_CENTER_US = 1500;
constexpr uint16_t RC_MAX_US = 2000;
constexpr uint16_t RC_VALID_MIN_US = 850;
constexpr uint16_t RC_VALID_MAX_US = 2150;
constexpr uint32_t RC_SIGNAL_TIMEOUT_US = 100000; // 100 ms

// Manual drive tuning.
// Start conservative, then raise these as the rover proves stable.
// Stick deadzone around 1500 us.
constexpr uint16_t RC_DRIVE_DEADBAND_US = 35;
// Caps forward/reverse stick authority before mixing.
constexpr float MAX_THROTTLE_INPUT = 0.70f; // 0.0 to 1.0 stick authority
// Caps turning authority before mixing.
constexpr float MAX_STEERING_INPUT = 0.55f; // 0.0 to 1.0 stick authority
// Flip these if the RC transmitter channel direction is backwards.
constexpr bool RC_THROTTLE_INVERT = true;
constexpr bool RC_STEERING_INVERT = true;
// Smooths small receiver jitter. 0.0 = no response, 1.0 = no smoothing.
constexpr float RC_COMMAND_FILTER_ALPHA = 0.15f;
// RC command processing rate. Step pulses still run every loop.
constexpr uint32_t CONTROL_UPDATE_INTERVAL_MS = 20;
// Absolute max wheel speed.
constexpr float MAX_WHEEL_RPM = 30.0f;
// Ramp rate; lower is smoother, higher is snappier.
constexpr float ACCEL_RPM_PER_SEC = 60.0f;

constexpr float PULSES_PER_MOTOR_REV = 400.0f;
constexpr float GEAR_RATIO = 10.0f;
constexpr float WHEEL_PULSES_PER_REV = PULSES_PER_MOTOR_REV * GEAR_RATIO;

constexpr uint16_t STEP_PULSE_WIDTH_US = 8;
constexpr uint16_t DIR_SETUP_US = 10;
constexpr float MIN_STEP_RATE_HZ = 1.0f;

// Long serial prints can jitter software step timing. Enable only while diagnosing.
constexpr bool SERIAL_DEBUG_ENABLED = false;
constexpr uint32_t DEBUG_INTERVAL_MS = 250;

// Temporary bench diagnostic. When true, RC is ignored and both step outputs
// generate a slow fixed pulse train after boot. Set false for normal driving.
constexpr bool MOTOR_SIGNAL_TEST_ENABLED = false;
constexpr uint32_t MOTOR_SIGNAL_TEST_DELAY_MS = 3000;
constexpr uint32_t MOTOR_SIGNAL_TEST_STEP_INTERVAL_US = 2000; // 500 steps/sec
constexpr uint16_t MOTOR_SIGNAL_TEST_PULSE_WIDTH_US = 20;

// Temporary UART wiring diagnostic. When true, RC is ignored and ESP32 GPIO17
// toggles slowly for oscilloscope testing at the Pi RX pin.
constexpr bool PI_UART_TX_SCOPE_TEST_ENABLED = false;
constexpr uint32_t PI_UART_TX_SCOPE_TEST_INTERVAL_MS = 500;

// Temporary direction calibration mode. When true, normal skid-steer mixing is
// bypassed: throttle drives only the left wheel, steering drives only the right.
constexpr bool MOTOR_DIRECTION_CALIBRATION_ENABLED = false;
constexpr float MOTOR_DIRECTION_CALIBRATION_RPM = 12.0f;

// Temporary automatic motor direction test. RC drive is ignored while enabled.
// Sequence: left +, left -, right +, right -, then repeat.
constexpr bool MOTOR_AUTO_DIRECTION_TEST_ENABLED = true;
constexpr uint32_t MOTOR_AUTO_DIRECTION_TEST_START_DELAY_MS = 5000;
constexpr uint32_t MOTOR_AUTO_DIRECTION_TEST_STEP_MS = 4000;
constexpr float MOTOR_AUTO_DIRECTION_TEST_RPM = 10.0f;

} // namespace Config
