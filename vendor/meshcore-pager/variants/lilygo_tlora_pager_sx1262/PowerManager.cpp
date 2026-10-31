#include "PowerManager.h"

#include <WiFi.h>
#include <Wire.h>

// Board helper configures output direction and uses XL9555 output registers 0x02/0x03.
// P3 is LoRa power: never disable it here. P5 is the unused NFC supply.
extern "C" bool meshcator_power_output(uint8_t pin, bool enabled);
extern "C" void tpager_gps_power(bool enabled);

PowerManager &PowerManager::instance() {
  static PowerManager s;
  return s;
}

void PowerManager::begin() {
  meshcator_power_output(5, false);

#if POWER_DISABLE_BLE
  WiFi.mode(WIFI_OFF);
  btStop();
#endif
  _lastActivity = millis();
}

// Backlight control. If TPagerST7796Display exposes setBrightness()/TFT_BL,
// prefer that; otherwise drive the AW9364 pin (42) with LEDC as here.
#define BL_PIN 42
void PowerManager::setBacklight(uint8_t level) {
  static bool ledc_ready = false;
  if (ledc_ready && level == _backlight) return;
  if (!ledc_ready) {
    ledcSetup(0, 5000, 8); // arduino-esp32 2.x LEDC API
    ledcAttachPin(BL_PIN, 0);
    ledc_ready = true;
  }
  ledcWrite(0, level);
  _backlight = level;
}

void PowerManager::onActivity() {
  _lastActivity = millis();
  if (_backlight != _targetBrightness) setBacklight(_targetBrightness);
}

void PowerManager::setTargetBrightness(uint8_t level) {
  _targetBrightness = level;
  if (_backlight > 0) setBacklight(level); // apply immediately if screen is on
}

void PowerManager::screenOff() {
  setBacklight(0);
}

void PowerManager::loop() {
  uint32_t now = millis(), idle = (now - _lastActivity) / 1000;

  // Screen: dim -> off. Biggest single battery win.
  if (_backlight > 0) {
    if (idle >= _screenTimeout)
      setBacklight(0);
    else if (idle >= _screenTimeout / 2)
      setBacklight(_targetBrightness / 4);
  }

  // GPS duty cycle: skipped entirely when GPS is disabled by the user.
  if (!_gpsDisabled) {
    if (now - _gpsWindowStart >= (_gpsOn ? GPS_ON_SECS : GPS_OFF_SECS) * 1000UL) {
      _gpsOn = !_gpsOn;
      _gpsWindowStart = now;
      tpager_gps_power(_gpsOn);
    }
  }
}

void PowerManager::setGpsEnabled(bool en) {
  _gpsDisabled = !en;
  if (!en) {
    // Hard off immediately.
    _gpsOn = false;
    tpager_gps_power(false);
  } else {
    // Resume duty cycle from now — turn on right away.
    _gpsOn = true;
    _gpsWindowStart = millis();
    tpager_gps_power(true);
  }
}