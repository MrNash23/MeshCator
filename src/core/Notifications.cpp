#include "App.h"
#include <algorithm>
namespace mc {
std::string App::notificationKey(const std::string&id)const{
 auto chat=std::find_if(chats.begin(),chats.end(),[&](const Chat&c){return c.id==id;});
 return chat!=chats.end()&&!chat->notificationKey.empty()?chat->notificationKey:id;
}
int App::notificationMode(const std::string&id)const{
 if(settings.notificationMute)return 0;
 auto found=settings.notificationRules.find(notificationKey(id));
 return found==settings.notificationRules.end()?settings.notificationMode:found->second;
}
bool App::handleNotificationKey(Key k){
 if(!notificationMenu||menuOpen)return false;
 if(k==Key::Home||k==Key::Menu)return false;
 if(k==Key::Back||k==Key::Delete)notificationMenu=false;
 else if(k==Key::Next||k==Key::Previous)notificationChoice=(notificationChoice+5+(k==Key::Next?1:-1))%5;
 else if(k==Key::Enter||k==Key::Confirm){
  auto next=settings;
  if(notificationChoice==0)next.notificationRules.erase(notificationTarget);
  else{
   if(next.notificationRules.size()>=32&&!next.notificationRules.count(notificationTarget)){notice=translate(settings.language,"Maximal 32 Ausnahmen");changed();return true;}
   next.notificationRules[notificationTarget]=notificationChoice-1;
  }
  if(backend_.saveSettings(next)){settings=next;notificationMenu=false;notice.clear();}
  else notice=translate(settings.language,"Einstellungen nicht gespeichert");
 }
 changed();return true;
}
}
