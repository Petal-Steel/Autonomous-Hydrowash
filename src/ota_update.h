#pragma once

#include <Arduino.h>

class OtaUpdate {
public:
  void begin();
  void handle();
  bool isUpdating() const { return updating_; }

private:
  void startWifiConnect();
  void startOtaService();

  bool enabled_ = false;
  bool updating_ = false;
  bool wifiStarted_ = false;
  uint32_t bootMs_ = 0;
  uint32_t wifiStartMs_ = 0;
  uint32_t lastWifiRetryMs_ = 0;
  uint32_t lastWifiStatusMs_ = 0;
};
