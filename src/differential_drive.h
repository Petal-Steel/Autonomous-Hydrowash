#pragma once

struct TankCommand {
  float left = 0.0f;
  float right = 0.0f;
};

class DifferentialDrive {
public:
  static TankCommand mix(float throttle, float steering);
  static TankCommand normalizedToRpm(const TankCommand &command);
};

