#pragma once

enum class DriveMode {
  ManualRc,
  AutonomousUart
};

class ModeManager {
public:
  DriveMode mode() const { return mode_; }
  void forceManual() { mode_ = DriveMode::ManualRc; }

private:
  DriveMode mode_ = DriveMode::ManualRc;
};

