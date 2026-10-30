#include "mesh/SimBackend.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
int main() {
  auto file = std::filesystem::temp_directory_path() /
    ("meshcator-settings-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
  mc::SimBackend backend(file.string());
  mc::App source(backend);
  for (int i = 0; i < 4; ++i) {
    source.settings.language = static_cast<mc::Language>(i);
    source.settings.theme = mc::Theme::Purple;
    source.settings.helpVersion = 1;
    if (!backend.saveSettings(source.settings)) return 1;
    mc::App restored(backend);
    backend.load(restored);
    if (restored.settings.language != source.settings.language || restored.settings.theme != mc::Theme::Purple || restored.settings.helpVersion != 1) return 2;
  }
  { std::ofstream f(file); f << "1\n2\nLegacy\n"; }
  mc::App legacy(backend);
  backend.load(legacy);
  if (legacy.settings.language != mc::Language::English || legacy.settings.theme != mc::Theme::Orange || legacy.settings.helpVersion != 0) return 3;
  { std::ofstream f(file); f << "1\n0\nInvalid\n99\n"; }
  mc::App invalid(backend);
  backend.load(invalid);
  if (invalid.settings.language != mc::Language::English) return 4;
  // A fresh entry via the global menu must not resume a stale settings category.
  source.screen=mc::Screen::Settings;source.settingsCategory=6;source.selected=0;
  source.beginSetting(mc::SettingField::Brightness);
  source.key(mc::Key::Menu);source.menuSelected=5;source.key(mc::Key::Confirm);
  if(source.screen!=mc::Screen::Settings||source.settingsCategory!=-1||source.settingsEditing||source.selected!=0)return 5;
  // Closing the menu without choosing a destination keeps the prior category.
  source.settingsCategory=6;source.key(mc::Key::Menu);source.key(mc::Key::Back);
  if(source.settingsCategory!=6)return 6;
  source.screen=mc::Screen::Dashboard;source.selected=5;source.key(mc::Key::Enter);
  if(source.settingsCategory!=-1||source.selected!=0)return 7;
  // Back inside settings still moves up one level.
  source.settingsCategory=6;source.key(mc::Key::Back);
  if(source.settingsCategory!=-1||source.selected!=6)return 8;
  // Keyboard Back regression: choices cancel, text still deletes.
  source.settingsCategory=-1;auto language=source.settings.language;
  source.beginSetting(mc::SettingField::Language);source.key(mc::Key::Next);source.key(mc::Key::Delete);
  if(source.settingsEditing||source.settings.language!=language||source.screen!=mc::Screen::Settings)return 9;
  source.settingsCategory=6;source.key(mc::Key::Delete);if(source.settingsCategory!=-1)return 10;
  source.beginSetting(mc::SettingField::Name);source.editValue="Test";source.key(mc::Key::Delete);
  if(!source.settingsEditing||source.editValue!="Tes")return 11;
  source.key(mc::Key::Back);if(source.settingsEditing)return 12;
  source.beginChannel();source.key(mc::Key::Delete);if(source.channelCreating)return 13;
  source.beginChannel();source.channelStage=1;source.channelName="ABC";source.key(mc::Key::Delete);
  if(!source.channelCreating||source.channelName!="AB")return 14;
  source.channelStage=3;source.key(mc::Key::Delete);if(source.channelStage!=2)return 15;
  source.key(mc::Key::Home);if(source.screen!=mc::Screen::Dashboard||source.channelCreating)return 16;
  std::filesystem::remove(file);
  std::cout << "Language persistence and legacy default passed\n";
}
