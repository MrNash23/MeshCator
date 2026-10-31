#include "core/App.h"
#include <iostream>
#define CHECK(x) do{if(!(x)){std::cerr<<"FAIL "<<__LINE__<<": "<<#x<<'\n';return 1;}}while(false)
struct Backend:mc::Backend{
 bool send(const mc::Chat&,const std::string&)override{return true;}
 bool advert(bool)override{return true;}
 bool saveSettings(const mc::Settings&)override{return true;}
};
int main(){
 for(bool channel:{false,true}){
  Backend b;mc::App a(b);a.screen=mc::Screen::Dashboard;
  a.receive("a","Team","Alice: first",channel,100);
  a.receive("b","Other","unrelated",false,100);
  a.receive("a","Team","Bob: second",channel,100);
  a.messages.push_back({"a","own interleaved",true,mc::Delivery::Delivered,100});
  a.receive("a","Team","Carol: third",channel,100);
  a.open(0);CHECK(a.historyOffset==3&&a.chats[0].unread==3&&a.messages[0].unread);
  a.key(mc::Key::Next);CHECK(a.historyOffset==2&&!a.messages[0].unread&&a.chats[0].unread==2);
  a.key(mc::Key::Back);CHECK(a.chats[0].unread==1&&a.messages.back().unread);
  a.open(0);CHECK(a.historyOffset==0);
  a.key(mc::Key::Previous);CHECK(a.chats[0].unread==0);
  a.receive("a","Team","Dave: fourth",channel,101);CHECK(a.historyOffset==2&&a.chats[0].unread==1);
  a.key(mc::Key::Home);a.open(0);CHECK(a.historyOffset==0&&a.messages.back().unread);
  mc::App restored(b);CHECK(restored.restore(a.archive()));restored.open(0);CHECK(restored.historyOffset==0&&restored.chats[0].unread==1);
  CHECK(restored.chats[1].unread==1); // other chat never read
 }
 Backend b;mc::App legacy(b);
 auto old=mc::sealArchive("MCH1\n1\n\"a\" \"Alice\" 0 2 \"\"\n4\n\"a\" \"old\" 0 0 1\n\"a\" \"first new\" 0 0 2\n\"a\" \"own\" 1 4 3\n\"a\" \"last new\" 0 0 4\n0\n");
 CHECK(legacy.restore(old));legacy.open(0);CHECK(legacy.historyOffset==2&&legacy.chats[0].unread==2);
 legacy.key(mc::Key::Home);CHECK(legacy.chats[0].unread==1);
 mc::App roundtrip(b);CHECK(roundtrip.restore(legacy.archive()));roundtrip.open(0);CHECK(roundtrip.historyOffset==0&&roundtrip.chats[0].unread==1);
 std::cout<<"Per-message unread, oldest-first, identical timestamps, incoming stability and MCH1 migration passed\n";
}
