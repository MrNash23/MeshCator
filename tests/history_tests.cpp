#include "core/App.h"
#include "core/MessageTime.h"
#include "core/MessageLog.h"
#include "mesh/SimBackend.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#define CHECK(x) do{if(!(x)){std::cerr<<"FAIL "<<__LINE__<<": "<<#x<<'\n';return 1;}}while(false)
struct B:mc::Backend{
 std::string saved,log;bool logOk=true;bool accept=true;int sends=0,resets=0;
 bool send(const mc::Chat&,const std::string&)override{return true;}
 mc::SendResult sendTracked(const mc::Chat&,const std::string&)override{++sends;return {accept,uint32_t(sends),2000};}
 bool advert(bool)override{return true;}bool saveSettings(const mc::Settings&)override{return true;}
 bool writeArchive(const std::string&s)override{if(accept)saved=s;return accept;}std::string readArchive()override{return saved;}
 bool appendMessageLog(const mc::Chat& c,const mc::Message& m)override{if(!logOk)return false;log+=mc::messageLogRecord(c,m);return true;}
 bool resetPath(const std::string&)override{++resets;return true;}
};
int main(){
 CHECK(mc::messageTime(1700000000)=="14.11.23 22:13 UTC");CHECK(mc::messageTime(0)=="--.--.-- --:-- UTC");
 {B backend;mc::App app(backend);app.status.epoch=1800000000;app.receive("peer","Peer","old",false,1700000000);CHECK(app.messages.back().timestamp==1700000000);app.receive("channel:0","Public","channel",true,1700000060);CHECK(app.messages.back().timestamp==1700000060);app.receive("peer","Peer","fallback",false);CHECK(app.messages.back().timestamp==1800000000);mc::App loaded(backend);CHECK(loaded.restoreArchive());CHECK(loaded.messages.front().timestamp==1700000000);}
 B b;mc::App a(b);a.settings.setupComplete=true;a.receive("alice","Alice","hello 🙂",false);a.open(0);a.type("Hi ❤️");CHECK(a.send());CHECK(a.messages.back().delivery==mc::Delivery::Queued);
 a.acknowledged("someone-else",1);CHECK(a.messages.back().delivery==mc::Delivery::Queued);
 a.acknowledged("alice",1);CHECK(a.messages.back().delivery==mc::Delivery::Delivered);
 a.type("again");CHECK(a.send());a.tick(2100);CHECK(a.messages.back().delivery==mc::Delivery::Unconfirmed);CHECK(a.retry(true)&&b.resets==1&&b.sends==3);a.acknowledged("alice",3);CHECK(a.messages.back().delivery==mc::Delivery::Delivered);
 a.type("draft 😃");a.tick(3200);CHECK(!a.archiveDirty);mc::App restored(b);CHECK(restored.restoreArchive());CHECK(restored.messages.size()==3);restored.open(0);CHECK(restored.draft=="draft 😃");
 auto good=a.archive(),bad=good;bad.back()^=1;CHECK(!restored.restore(bad)&&restored.messages.size()==3);
 a.type("!");b.accept=false;a.tick(4300);CHECK(a.archiveDirty&&!a.notice.empty());b.accept=true;a.tick(15000);CHECK(!a.archiveDirty);
 a.draft="pending";a.send();mc::App rebooted(b);CHECK(rebooted.restoreArchive());CHECK(rebooted.messages.back().delivery==mc::Delivery::Unconfirmed);
 CHECK(mc::normalizeChannelKey("AAAAAAAAAAAAAAAAAAAAAA==")==std::string(32,'0'));
 CHECK(mc::normalizeChannelKey("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=")==std::string(64,'0'));
 CHECK(mc::normalizeChannelKey("AAAAAAAAAAAAAAAAAAAAAB==").empty());
 CHECK(mc::normalizeChannelKey("foo").empty());
 std::string path="history-test-settings";mc::SimBackend disk(path);CHECK(disk.writeArchive(good));CHECK(disk.writeArchive(good));{std::ofstream f(path+".history");f<<"corrupt";}CHECK(disk.readArchive()==good);mc::App seeded(disk);disk.seed(seeded);CHECK(seeded.messages.size()==3);CHECK(disk.readArchive()==good);
 std::remove((path+".history").c_str());std::remove((path+".history.bak").c_str());
 mc::App channels(b);CHECK(channels.bindChannel("channel:0","Old","key-one"));channels.receive("channel:0","Old","old secret",true);CHECK(channels.bindChannel("channel:0","New","key-two"));CHECK(channels.chats.size()==2&&channels.messages.front().chat=="archive:key-one");channels.receive("channel:0","New","new secret",true);CHECK(channels.messages.back().chat=="channel:0");
 {B storage;mc::App large(storage);
  for(size_t i=0;i<mc::MaxMessages+8;++i)large.receive("peer","Peer",std::string(140,'x')+std::to_string(i),false,1700000000+i);
  CHECK(large.messages.size()==512);CHECK(large.messages.front().text==std::string(140,'x')+"8");
  CHECK(large.archive().size()>64000);mc::App loaded(storage);CHECK(loaded.restoreArchive());CHECK(loaded.messages.size()==512&&loaded.messages.back().text==large.messages.back().text);
  CHECK(std::count(storage.log.begin(),storage.log.end(),'\n')==520);CHECK(storage.log.find(std::string(140,'x')+"0\n")!=std::string::npos);
  storage.logOk=false;large.receive("peer","Peer","card full",false);CHECK(large.notice=="SD archive not saved");CHECK(large.messages.back().text=="card full");
 }
 {mc::Chat c{"channel:1","Private",true};mc::Message m{c.id,"hi\tthere\n🙂\\",false,mc::Delivery::Received,123};auto line=mc::messageLogRecord(c,m);CHECK(line=="123\tRX\tchannel\tchannel:1\tPrivate\t0\thi\\tthere\\n🙂\\\\\n");
  std::remove((path+".messages.tsv").c_str());CHECK(disk.appendMessageLog(c,m));std::ifstream f(path+".messages.tsv");std::string saved((std::istreambuf_iterator<char>(f)),{});CHECK(saved==line);std::remove((path+".messages.tsv").c_str());}
 std::cout<<"ACK, retry, timeout, drafts, integrity, fallback and legacy seed passed\n";
}
