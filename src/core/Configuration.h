#pragma once
#include <string>
#include <vector>
namespace mc {
struct Settings;
enum class SettingField { Language, Theme, Preset, Frequency, Bandwidth, SF, CR, Power, Mode, Name, GPS, ShareLocation, Bluetooth, BluetoothName, BluetoothPIN, Brightness, Timeout, MapSource, Contacts, Setup, Version, ChatFilter, NotificationMute, NotificationMode, NotificationVolume, NotificationWake };
std::vector<SettingField> settingFields(int category);
const char *settingLabel(SettingField field);
std::vector<std::string> settingOptions(SettingField field);
std::string settingValue(const Settings &, SettingField);
bool setSetting(Settings &, SettingField, const std::string &, std::string &error);
std::string encodeSettings(const Settings &);
bool decodeSettings(const std::string &, Settings &);
}
