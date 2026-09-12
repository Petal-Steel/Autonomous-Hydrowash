#include "pi_uart.h"

#include <string.h>

#include "config.h"

void PiUart::begin() {
  Serial1.begin(Config::PI_UART_BAUD, SERIAL_8N1, Config::PI_UART_RX_PIN, Config::PI_UART_TX_PIN);
  Serial1.println("ESP32_UART_READY");
}

void PiUart::update() {
  const uint32_t nowMs = millis();
  if ((nowMs - lastHeartbeatMs_) >= 1000) {
    lastHeartbeatMs_ = nowMs;
    Serial1.println("ESP32_HEARTBEAT");
  }

  while (Serial1.available() > 0) {
    const char c = static_cast<char>(Serial1.read());

    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      lineBuffer_[lineLength_] = '\0';
      if (lineLength_ > 0) {
        handleLine(lineBuffer_);
      }
      lineLength_ = 0;
      continue;
    }

    if (lineLength_ < sizeof(lineBuffer_) - 1) {
      lineBuffer_[lineLength_++] = c;
    } else {
      lineLength_ = 0;
      Serial1.println("ERR,line_too_long");
    }
  }
}

void PiUart::handleLine(const char *line) {
  if (strcmp(line, "PING") == 0) {
    Serial1.println("PONG");
    return;
  }

  Serial1.print("ERR,unknown:");
  Serial1.println(line);
}
