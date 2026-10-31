#include "core/App.h"
#include <iostream>
#include <limits>
#define CHECK(x) do{if(!(x)){std::cerr<<"FAIL "<<__LINE__<<": "<<#x<<'\n';return 1;}}while(false)
struct Backend:mc::Backend{
 bool awake=true;
 bool displayAwake() const override{return awake;}
 int alerts=0;bool sound=false,vibrate=false,wake=false,channel=false,accept=true;
 bool send(const mc::Chat&,const std::string&)override{return true;}
 bool advert(bool)override{return true;}
 bool saveSettings(const mc::Settings&)override{return accept;}
 void notifyReceived(bool s,bool v,bool w,bool c)override{++alerts;sound=s;vibrate=v;wake=w;channel=c;}
};
int main(){
 Backend b;mc::App a(b);a.screen=mc::Screen::Dashboard;
 a.receive("alice","Alice","hello",false);CHECK(b.alerts==1&&!b.sound&&b.vibrate&&b.wake&&!b.channel);
 a.settings.notificationRules["alice"]=0;a.receive("alice","Alice","silent",false);
 CHECK(b.alerts==1&&a.chats[0].unread==2&&a.messages.size()==2);
 a.settings.notificationRules["alice"]=3;a.settings.notificationMute=true;
 a.receive("alice","Alice","master mute",false);CHECK(b.alerts==1&&a.notificationMode("alice")==0);
 a.settings.notificationMute=false;a.settings.notificationWake=false;
 a.receive("alice","Alice","both",false);CHECK(b.alerts==2&&b.sound&&b.vibrate&&!b.wake);
 a.open(0);a.receive("alice","Alice","already reading",false);CHECK(b.alerts==2&&a.chats[0].unread==5);
 a.contextOpen=true;a.receive("alice","Alice","overlay",false);CHECK(b.alerts==3&&a.chats[0].unread==6);a.contextOpen=false;
 a.chats.push_back({"channel:0","Team",true,0,"channel-key:original"});
 a.settings.notificationRules["channel-key:original"]=0;a.receive("channel:0","Team","silent channel",true);CHECK(b.alerts==3);
 a.chats[1].notificationKey="channel-key:replacement";a.receive("channel:0","New team","different key",true);CHECK(b.alerts==4&&b.channel);
 a.notificationMenu=true;a.notificationTarget="alice";a.notificationChoice=0;b.accept=false;
 a.key(mc::Key::Confirm);CHECK(a.notificationMenu&&a.settings.notificationRules.count("alice"));
 b.accept=true;a.key(mc::Key::Confirm);CHECK(!a.notificationMenu&&!a.settings.notificationRules.count("alice"));
 a.notificationMenu=true;a.key(mc::Key::Home);CHECK(!a.notificationMenu&&a.screen==mc::Screen::Dashboard);
 b.awake=false;a.open(0);int before=b.alerts;
 a.receive("alice","Alice",std::string(145,'x')+"é",false);CHECK(b.alerts==before+1&&a.messages.back().text.size()==147&&a.chats[0].unread==7);
 mc::Settings restored;auto text=mc::encodeSettings(a.settings);CHECK(mc::decodeSettings(text,restored));
 CHECK(restored.notificationRules==a.settings.notificationRules&&restored.notificationMode==2&&!restored.notificationWake);
 auto n=text.find("N1 ");CHECK(n!=std::string::npos);CHECK(mc::decodeSettings(text.substr(0,n),restored));
 CHECK(!mc::decodeSettings(text.substr(0,n)+"N1 0 2",restored));
 CHECK(!mc::decodeSettings(text.substr(0,n)+"N1 0 2 40 1 1\n",restored));
 CHECK(!mc::decodeSettings(text.substr(0,n)+"N1 0 2 40 1 2\n\"duplicate\" 0\n\"duplicate\" 1\n",restored));
 {Backend wakeBackend;mc::App wakeApp(wakeBackend);wakeApp.receive("old","Old","hello",false);wakeApp.open(0);wakeApp.type("keep draft");wakeApp.menuOpen=true;wakeApp.historyOffset=12;wakeBackend.awake=false;
 wakeApp.receive("new","New","wake up",false);CHECK(wakeApp.screen==mc::Screen::Conversation&&wakeApp.activeChat=="new"&&!wakeApp.menuOpen&&wakeApp.historyOffset==0&&wakeApp.chats[1].unread==1);wakeApp.open(0);CHECK(wakeApp.draft=="keep draft");
 wakeApp.screen=mc::Screen::Dashboard;wakeBackend.awake=true;wakeApp.receive("new","New","awake",false);CHECK(wakeApp.screen==mc::Screen::Dashboard);
 wakeBackend.awake=false;wakeApp.settings.notificationMute=true;wakeApp.receive("new","New","muted",false);CHECK(wakeApp.screen==mc::Screen::Dashboard);
 wakeApp.settings.notificationMute=false;wakeApp.settings.notificationWake=false;wakeApp.receive("new","New","no wake",false);CHECK(wakeApp.screen==mc::Screen::Dashboard);
 wakeApp.settings.notificationWake=true;wakeApp.receive("channel:1","Team","channel wake",true);CHECK(wakeApp.activeChat=="channel:1"&&wakeApp.screen==mc::Screen::Conversation);
 wakeApp.screen=mc::Screen::Lock;wakeApp.receive("new","New","locked",false);CHECK(wakeApp.screen==mc::Screen::Lock);
 }
 {Backend badgeBackend;mc::App badges(badgeBackend);badges.screen=mc::Screen::Dashboard;
 CHECK(badges.unreadBadgeCount(true)==0&&badges.unreadBadgeCount(false)==0);
 badges.receive("dm","Direct","one",false);badges.receive("channel:0","Channel","one",true);badges.receive("channel:0","Channel","two",true);
 CHECK(badges.unreadBadgeCount(false)==1&&badges.unreadBadgeCount(true)==2);
 badges.settings.notificationMute=true;badges.receive("dm","Direct","muted",false);CHECK(badges.unreadBadgeCount(false)==2);
 mc::App restored(badgeBackend);CHECK(restored.restore(badges.archive()));CHECK(restored.unreadBadgeCount(false)==2&&restored.unreadBadgeCount(true)==2);
 badges.open(0);CHECK(badges.unreadBadgeCount(false)==2&&badges.unreadBadgeCount(true)==2);
 badges.receive("dm","Direct","already reading",false);CHECK(badges.unreadBadgeCount(false)==3);
 badges.menuOpen=true;badges.receive("dm","Direct","menu hides chat",false);CHECK(badges.unreadBadgeCount(false)==4);
 badges.chats[0].unread=std::numeric_limits<unsigned>::max();CHECK(badges.unreadBadgeCount(false)==100);
 badges.chats[1].unread=99;CHECK(badges.unreadBadgeCount(true)==99);badges.chats[1].unread=100;CHECK(badges.unreadBadgeCount(true)==100);
 }
 std::cout<<"notification policy, identity, persistence and navigation passed\n";
}
