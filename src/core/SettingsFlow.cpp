#include "App.h"
#include <algorithm>
namespace mc {
void App::beginSetting(SettingField field) {
  editField=field; editValue=settingValue(settings,field); editTyped=false;
  auto options=settingOptions(field);
  auto found=std::find(options.begin(),options.end(),editValue);
  settingChoice=found==options.end()?0:int(found-options.begin());
  settingsEditing=true; notice.clear(); changed();
}
bool App::commitSetting() {
  Settings next=settings; std::string error;
  auto options=settingOptions(editField);
  auto value=options.empty()?editValue:options[settingChoice];
  if(!setSetting(next,editField,value,error)) {notice=translate(settings.language,error);changed();return false;}
  if(!backend_.saveSettings(next)){notice=translate(settings.language,"Einstellungen nicht gespeichert");changed();return false;}
  settings=next;settingsEditing=false;notice.clear();changed();return true;
}
void App::beginSetup(){
 if(settings.setupComplete){auto next=settings;next.setupComplete=false;if(!backend_.saveSettings(next)){notice=translate(settings.language,"Einstellungen nicht gespeichert");changed();return;}settings=next;}
 setupWizard=true;setupStep=0;screen=Screen::Settings;settingsCategory=-1;selected=0;beginSetting(SettingField::Language);}
bool App::handleSettingsKey(Key k){
 if(screen!=Screen::Settings||menuOpen||contextOpen||helpOpen||notificationMenu)return false;
 if(k==Key::Home||k==Key::Menu||k==Key::Mode)return false;
 if(k==Key::Delete&&(!settingsEditing||!settingOptions(editField).empty()))k=Key::Back;
 if(settingsEditing){
  auto options=settingOptions(editField);
  if(k==Key::Next||k==Key::Previous){if(!options.empty())settingChoice=(settingChoice+int(options.size())+(k==Key::Next?1:-1))%options.size();}
  else if(k==Key::Delete&&options.empty()){eraseUtf8(editValue);editTyped=true;}
  else if(k==Key::Back){settingsEditing=false;setupWizard=false;}
  else if(k==Key::Enter||k==Key::Confirm){
   if(commitSetting()&&setupWizard){
    static const SettingField steps[]={SettingField::Language,SettingField::Preset,SettingField::Name,SettingField::Mode,SettingField::Bluetooth,SettingField::Setup};
    if(++setupStep<6)beginSetting(steps[setupStep]);else{setupWizard=false;openHelp(true);}
   }
  }
 }else{
  auto fields=settingFields(settingsCategory);size_t count=settingsCategory<0?12:settingsCategory==7?2:fields.size();
  if(k==Key::Next||k==Key::Previous){if(count)selected=(selected+count+(k==Key::Next?1:-1))%count;}
  else if(k==Key::Back){if(settingsCategory>=0){selected=settingsCategory;settingsCategory=-1;}else {screen=Screen::Dashboard;selected=0;}}
  else if(k==Key::Enter||k==Key::Confirm){
   if(settingsCategory==7){if(selected==0)beginSetting(SettingField::ChatFilter);else beginChannel();}
   else if(settingsCategory>=0){if(selected<fields.size())beginSetting(fields[selected]);}
   else if(selected==0)beginSetting(SettingField::Language);
   else if(selected==1)beginSetting(SettingField::Theme);
   else if(selected==9)beginSetup();
   else if(selected==10)notice="MeshCator 0.3.8-dev";
   else if(selected==11)openHelp();
   else {settingsCategory=int(selected);selected=0;}
  }
 }
 changed();return true;
}
}
