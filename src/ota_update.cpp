#include "ota_update.h"

#include <ArduinoOTA.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include "config.h"

#if __has_include("secrets.h")
#include "secrets.h"
#endif

#ifndef HYDROWASH_WIFI_SSID
#define HYDROWASH_WIFI_SSID ""
#endif

#ifndef HYDROWASH_WIFI_PASSWORD
#define HYDROWASH_WIFI_PASSWORD ""
#endif

#ifndef HYDROWASH_OTA_PASSWORD
#define HYDROWASH_OTA_PASSWORD ""
#endif

void OtaUpdate::begin() {
  if (!Config::OTA_ENABLED) {
    return;
  }

  if (strlen(HYDROWASH_WIFI_SSID) == 0) {
    Serial.println("OTA disabled: WiFi SSID not configured in secrets.h");
    return;
  }

  bootMs_ = millis();
  lastWifiRetryMs_ = bootMs_ - Config::OTA_WIFI_RETRY_INTERVAL_MS;
  Serial.println("OTA WiFi connect delayed");
}

void OtaUpdate::startWifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  esp_wifi_set_max_tx_power(34); // 8.5 dBm, lower peak current than default.
  WiFi.setHostname(Config::OTA_HOSTNAME);
  WiFi.begin(HYDROWASH_WIFI_SSID, HYDROWASH_WIFI_PASSWORD);

  wifiStarted_ = true;
  wifiStartMs_ = millis();
  lastWifiRetryMs_ = wifiStartMs_;
  lastWifiStatusMs_ = wifiStartMs_;
  Serial.println("OTA WiFi connect started");
  Serial.print("OTA WiFi MAC=");
  Serial.println(WiFi.macAddress());
}

void OtaUpdate::startOtaService() {
  ArduinoOTA.setHostname(Config::OTA_HOSTNAME);
  if (strlen(HYDROWASH_OTA_PASSWORD) > 0) {
    ArduinoOTA.setPassword(HYDROWASH_OTA_PASSWORD);
  }

  ArduinoOTA.onStart([this]() {
    updating_ = true;
    Serial.println("OTA update starting");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("OTA update complete");
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.print("OTA error ");
    Serial.println(static_cast<int>(error));
  });

  ArduinoOTA.begin();
  enabled_ = true;

  Serial.print("OTA ready: ");
  Serial.print(Config::OTA_HOSTNAME);
  Serial.print(".local IP=");
  Serial.println(WiFi.localIP());
}

void OtaUpdate::handle() {
  if (!Config::OTA_ENABLED || enabled_) {
    if (enabled_) {
      ArduinoOTA.handle();
    }
    return;
  }

  if (!wifiStarted_) {
    const uint32_t nowMs = millis();
    if ((nowMs - bootMs_) < Config::OTA_WIFI_START_DELAY_MS) {
      return;
    }

    if ((nowMs - lastWifiRetryMs_) >= Config::OTA_WIFI_RETRY_INTERVAL_MS) {
      startWifiConnect();
    }
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    startOtaService();
    return;
  }

  const uint32_t nowMs = millis();
  if ((nowMs - lastWifiStatusMs_) >= 5000) {
    lastWifiStatusMs_ = nowMs;
    Serial.print("OTA WiFi waiting, status=");
    Serial.print(static_cast<int>(WiFi.status()));
    Serial.print(" elapsed=");
    Serial.print((nowMs - wifiStartMs_) / 1000);
    Serial.println("s");
  }

  if ((nowMs - wifiStartMs_) >= Config::OTA_WIFI_CONNECT_TIMEOUT_MS) {
    Serial.println("OTA WiFi connect timed out; will retry later");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    wifiStarted_ = false;
    lastWifiRetryMs_ = nowMs;
    return;
  }

  if (enabled_) {
    ArduinoOTA.handle();
  }
}
