#include "core/App.h"
#include <iostream>
#include <algorithm>
#define CHECK(x) do{if(!(x)){std::cerr<<"FAIL "<<__LINE__<<": "<<#x<<'\n';return 1;}}while(false)
struct B:mc::Backend{
 int removed=0,remoteCalls=0,op=-1;uint8_t permissions=0;std::string path;bool accept=true;std::string command;
 bool send(const mc::Chat&,const std::string&)override{return true;}bool advert(bool)override{return true;}bool saveSettings(const mc::Settings&)override{return accept;}
 bool removeEntry(const std::string&,bool)override{if(accept)++removed;return accept;}
 bool setPath(const std::string&,const std::string&s)override{path=s;return accept;}
 bool setPermissions(const std::string&,uint8_t bits)override{permissions=bits;return accept;}
 bool remote(const std::string&,int operation,const std::string&s)override{op=operation;++remoteCalls;command=s;return true;}
};
int main(){B b;mc::App a(b);a.settings.setupComplete=true;a.receive("alice","Alice","hello",false);a.open(0);a.contextId="alice";
 a.action(mc::ContextAction::DeleteEntry);a.key(mc::Key::Confirm);CHECK(b.removed==0&&a.chats.size()==1);
 a.action(mc::ContextAction::DeleteEntry);a.key(mc::Key::Next);b.accept=false;a.key(mc::Key::Enter);CHECK(a.chats.size()==1&&a.utility==mc::Utility::DeleteEntry);b.accept=true;a.key(mc::Key::Enter);CHECK(b.removed==1&&a.chats.empty()&&a.messages.empty());
 a.receive("alice","Alice","new",false);a.open(0);a.action(mc::ContextAction::Emoji);a.key(mc::Key::Confirm);CHECK(a.draft=="🙂");
 a.action(mc::ContextAction::Remote);a.key(mc::Key::Confirm);CHECK(a.utility==mc::Utility::RemoteLogin);a.type("secret");a.key(mc::Key::Enter);CHECK(b.command=="secret"&&a.utilityText.find("secret")==std::string::npos&&b.remoteCalls==1);
 a.remoteReply("wrong","should ignore");CHECK(a.remoteDeadline!=0);a.tick(31000);CHECK(a.remoteDeadline==0&&a.utility==mc::Utility::RemoteOutput);
 a.key(mc::Key::Home);CHECK(a.screen==mc::Screen::Dashboard&&a.utility==mc::Utility::None);
 a.utilityOpen(mc::Utility::RemoteOutput);a.remotePeer="alice";a.remoteDeadline=99999;a.key(mc::Key::Home);a.remoteReply("alice","late reply");CHECK(a.utility==mc::Utility::None&&a.screen==mc::Screen::Dashboard);
 a.action(mc::ContextAction::Filter);a.key(mc::Key::Next);a.key(mc::Key::Enter);CHECK(a.settings.chatFilter==1);
 a.nodes.push_back({"alice","Alice",{52,13,true},0,false,1,false,2,100,"ab",0});a.contextId="alice";a.contextChannel=false;
 a.screen=mc::Screen::Contacts;auto actions=a.contextActions();CHECK(std::find(actions.begin(),actions.end(),mc::ContextAction::Remote)!=actions.end());
 a.nodes[0].type=1;actions=a.contextActions();CHECK(std::find(actions.begin(),actions.end(),mc::ContextAction::Remote)==actions.end());
 a.action(mc::ContextAction::SetPath);CHECK(a.utilityText=="ab");a.type("cd");b.accept=false;a.key(mc::Key::Enter);CHECK(a.utility==mc::Utility::SetPath&&a.nodes[0].path=="ab");b.accept=true;a.key(mc::Key::Enter);CHECK(b.path=="abcd"&&a.nodes[0].hops==2);
 a.action(mc::ContextAction::Permissions);a.key(mc::Key::Confirm);CHECK(b.permissions==1&&a.nodes[0].permissions==1);a.key(mc::Key::Next);a.key(mc::Key::Confirm);CHECK(b.permissions==3);a.key(mc::Key::Back);
 a.action(mc::ContextAction::ShowMap);CHECK(a.screen==mc::Screen::Map&&a.mapCenter.lat==52&&a.mapZoom==10);
 a.settings.setupComplete=false;int before=b.remoteCalls;a.action(mc::ContextAction::Ping);CHECK(b.remoteCalls==before&&a.remoteDeadline==0);a.settings.setupComplete=true;
 for(auto action:{mc::ContextAction::Ping,mc::ContextAction::Trace,mc::ContextAction::Telemetry}){a.action(action);CHECK(a.remoteDeadline!=0);a.remoteReply("wrong","bad");CHECK(a.remoteDeadline!=0);a.remoteReply("alice","response");CHECK(a.remoteDeadline==0&&a.utilityText=="response");}CHECK(b.op==6);
 a.status.epoch=50;a.action(mc::ContextAction::Details);CHECK(a.utilityText.find("-50")==std::string::npos&&a.utilityText.find("1:abcd")!=std::string::npos);
 a.nodes[0].name="A & B";a.action(mc::ContextAction::QR);CHECK(a.utilityText.find("name=A%20%26%20B&public_key=alice&type=1")!=std::string::npos);a.key(mc::Key::Home);CHECK(a.utility==mc::Utility::None);
 std::cout<<"Deletion confirmation, backend failure, emoji, filters and remote timeout passed\n";
}
