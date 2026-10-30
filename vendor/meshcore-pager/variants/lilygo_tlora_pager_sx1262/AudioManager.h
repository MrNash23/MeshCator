#pragma once
#include <Arduino.h>
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

enum class SoundEvent { MSG_DM, MSG_CHANNEL, ADVERT, ACK, LOW_BATTERY, BOOT };

class AudioManager {
public:
  static AudioManager &instance();
  bool begin();                    // I2S + ES8311 init + audio task
  void setVolume(uint8_t percent); // 0..100 (reg 0x32 + digital scale)
  void setEnabled(bool on) { _enabled = on; if(!on){_pending=-1;_taskEvent=-1;} }
  bool isEnabled() const { return _enabled; }
  void notify(SoundEvent ev); // queue (safe from any task)
  void loop();                // enforces cooldown, wakes audio task

private:
  AudioManager() = default;
  void codecWrite(uint8_t reg, uint8_t val);
  void paEnable(bool on);         // NS4150B amp gate via XL9555 (RMW)
  void tone(int freq_hz, int ms); // 48 kHz sine playback
  void chimeFor(SoundEvent ev);
  static void audioTask(void *arg); // FreeRTOS task — blocks on notify

  std::atomic<bool> _enabled = (AUDIO_ENABLED_DEFAULT != 0);
  std::atomic<uint8_t> _volume = AUDIO_DEFAULT_VOLUME;
  std::atomic<int> _pending = -1;
  std::atomic<int> _taskEvent = -1;
  TaskHandle_t _audioTaskHandle = nullptr;
  uint32_t _lastPlayMs = 0;
  static const uint32_t COOLDOWN_MS = 3000;
};