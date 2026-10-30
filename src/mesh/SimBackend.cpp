#include "core/MessageLog.h"
#include "mesh/SimBackend.h"
#include <cstdio>
#include "maps/BmpTile.h"
#include <ctime>
#include <fstream>
#include <random>
#include <sstream>
#include <iomanip>
namespace mc {
bool SimBackend::saveChannels(){auto file=path_+".channels",temp=file+".tmp";std::ofstream f(temp);for(auto&c:channels_)f<<std::quoted(c.first)<<' '<<std::quoted(c.second.first)<<' '<<std::quoted(c.second.second)<<'\n';f.close();return f&&std::rename(temp.c_str(),file.c_str())==0;}
std::string SimBackend::createChannel(const std::string&name,const std::string&key,bool priv){
 if(name.empty()||name.size()>30||!validText(name))return {};
 auto n=priv?name:name[0]=='#'?name:"#"+name;for(auto&c:channels_)if(c.second.first==n)return {};std::string id;do{id="channel:demo-"+std::to_string(++channelSequence_);}while(channels_.count(id));channels_[id]={n,key};if(saveChannels())return id;channels_.erase(id);return {};
}
bool SimBackend::channelInfo(const std::string&id,std::string&n,std::string&k){auto i=channels_.find(id);if(i==channels_.end())return false;n=i->second.first;k=i->second.second;return true;}
bool SimBackend::updateChannel(const std::string&id,const std::string&n,const std::string&k,bool){auto i=channels_.find(id);if(i==channels_.end())return false;auto previous=i->second;i->second={n,k};if(saveChannels())return true;i->second=previous;return false;}
bool SimBackend::removeEntry(const std::string&id,bool channel){if(!channel)return true;auto prev=channels_;channels_.erase(id);if(saveChannels())return true;channels_=prev;return false;}
std::string SimBackend::exportEntry(const std::string&id,bool channel){if(!channel)return "DEMO contact / no signed radio advertisement";std::string n,k;if(!channelInfo(id,n,k))return {};std::ostringstream o;o<<"MC-CHANNEL1 "<<std::quoted(n)<<' '<<k;return o.str();}
bool SimBackend::importEntry(const std::string&s){std::istringstream in(s);std::string tag,n,k,x;if(!(in>>tag>>std::quoted(n)>>k)||tag!="MC-CHANNEL1"||in>>x||normalizeChannelKey(k).empty())return false;return !createChannel(n,normalizeChannelKey(k),true).empty();}
std::string SimBackend::channelKey(){std::random_device random;static const char hex[]="0123456789abcdef";std::string s;for(int i=0;i<32;++i)s+=hex[random()&15];return s;}
bool SimBackend::readTile(int z,int x,int y,std::vector<uint16_t>&pixels){
 auto slash=path_.find_last_of('/');auto dir=slash==std::string::npos?std::string("."):path_.substr(0,slash);
 std::ifstream f(dir+"/tiles/"+std::to_string(z)+"/"+std::to_string(x)+"/"+std::to_string(y)+".bmp",std::ios::binary);
 if(!f)return false;
 return readBmpTile([&](uint32_t off,uint8_t*data,size_t len){f.seekg(off);return bool(f.read(reinterpret_cast<char*>(data),len));},pixels);
}
bool SimBackend::saveSettings(const Settings &s) {
  auto temp = path_ + ".tmp";
  std::ofstream f(temp, std::ios::trunc);
  if (!f)
    return false;
  f << encodeSettings(s);
  f.close();
  if (!f)
    return false;
  return std::rename(temp.c_str(), path_.c_str()) == 0;
}
bool SimBackend::appendMessageLog(const Chat& c,const Message& m){std::ofstream f(path_+".messages.tsv",std::ios::app|std::ios::binary);f<<messageLogRecord(c,m);f.flush();return bool(f);}
bool SimBackend::writeArchive(const std::string&s){
 if(seeding_)return true;
 auto file=path_+".history",temp=file+".tmp",bak=file+".bak";
 std::ofstream f(temp,std::ios::binary|std::ios::trunc);f<<s;f.close();if(!f)return false;
 std::ifstream old(file);if(old.good()){std::remove(bak.c_str());if(std::rename(file.c_str(),bak.c_str()))return false;}
 return std::rename(temp.c_str(),file.c_str())==0;
}
std::string SimBackend::readArchive(){bool found=false;for(auto suffix:{".history",".history.bak"}){std::ifstream f(path_+suffix,std::ios::binary);if(f)found=true;std::string s((std::istreambuf_iterator<char>(f)),{}),body;if(unsealArchive(s,body))return s;}return found?"CORRUPT":"";}
void SimBackend::load(App &a) {
  std::ifstream f(path_);
  std::string encoded((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  if (decodeSettings(encoded, a.settings)) return;
  f.clear(); f.seekg(0);
  unsigned version;
  int theme;
  std::string name;
  if (!(f >> version >> theme) || version != 1 || theme < 0 || theme > 3)
    return;
  f.ignore();
  std::getline(f, name);
  if (name.empty() || name.size() > 31)
    return;
  a.settings = {};
  a.settings.version=version; a.settings.theme=static_cast<Theme>(theme); a.settings.name=name;
  int language = 0;
  if (f >> language && language >= 0 && language < 4)
    a.settings.language = static_cast<Language>(language);
  int helpVersion = 0;
  if (f >> helpVersion && helpVersion >= 0 && helpVersion <= 255)
    a.settings.helpVersion = static_cast<uint8_t>(helpVersion);
}
void SimBackend::seed(App &a) {
  seeding_=true;
  channels_={{"channel:0",{"#berlin",std::string(32,'0')}},{"channel:1",{"Public",std::string(32,'1')}}};
  std::ifstream catalog(path_+".channels");if(catalog){channels_.clear();std::string id,n,k;while(catalog>>std::quoted(id)>>std::quoted(n)>>std::quoted(k))channels_[id]={n,k};}

  a.status = {
      78, 8, true, true, static_cast<uint32_t>(std::time(nullptr)), {52.5200, 13.4050, true}};
  a.receive("channel:0", "#berlin", "Bin am Tor, wer kommt noch?", true);
  a.receive("channel:1", "Public", "Hallo Mesh!", true);
  a.receive("tracy", "Tracy", "Bis gleich!", false);
  a.nodes = {{"rpt", "RPT-Mitte", {52.5245, 13.4105, true}, -7, true, 1},
             {"tracy", "Tracy", {52.517, 13.401, true}, -11, true, 2},
             {"weather", "Wetter-01", {52.519, 13.416, true}, -14, true, 3},
             {"unknown", "Ohne GPS", {}, -9, true, 1}};
  a.nodes[0].type=2;a.nodes[0].path="a3";a.nodes[0].lastAdvert=a.status.epoch-120;a.nodes[2].type=4;
  for (int i = 0; i < 60; ++i)
    a.sample(-115 + (i % 7));
  a.notice.clear(); // DEMO remains visible in the header; keep transient notices free.
  load(a);
  a.restoreArchive();
  seeding_=false;
}
} // namespace mc
