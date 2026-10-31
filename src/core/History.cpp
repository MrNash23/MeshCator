#include "App.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
namespace mc {
static uint32_t checksum(const std::string&s){uint32_t c=2166136261u;for(unsigned char b:s)c=(c^b)*16777619u;return c;}
std::string sealArchive(const std::string&s){return std::to_string(checksum(s))+"\n"+s;}
bool unsealArchive(const std::string&s,std::string&body){auto p=s.find('\n');if(p==std::string::npos||p>10)return false;std::istringstream in(s.substr(0,p));uint32_t n;char x;if(!(in>>n)||in>>x)return false;body=s.substr(p+1);return checksum(body)==n;}
void App::recountUnread(){
 for(auto& c:chats){c.unread=0;for(const auto& m:messages)if(m.chat==c.id&&m.unread)++c.unread;}
}
void App::readCurrentMessage(){
 if(screen!=Screen::Conversation||menuOpen||contextOpen||helpOpen||notificationMenu||utility!=Utility::None||!backend_.displayAwake())return;
 size_t offset=0;
 for(auto m=messages.rbegin();m!=messages.rend();++m)if(m->chat==activeChat&&offset++==historyOffset){if(m->unread){m->unread=false;recountUnread();scheduleArchive();changed();}break;}
}
void App::scheduleArchive(){if(!archiveDirty)archiveDue=clockMs+1000;archiveDirty=true;}
std::string App::archive()const{
 std::ostringstream o;o<<"MCH2\n"<<chats.size()<<'\n';
 for(auto &c:chats)o<<std::quoted(c.id)<<' '<<std::quoted(c.name)<<' '<<c.channel<<' '<<c.unread<<' '<<std::quoted(c.notificationKey)<<'\n';
 o<<messages.size()<<'\n';for(auto&m:messages)o<<std::quoted(m.chat)<<' '<<std::quoted(m.text)<<' '<<m.own<<' '<<int(m.delivery)<<' '<<m.timestamp<<' '<<m.unread<<'\n';
 auto drafts=drafts_;if(!activeChat.empty())drafts[activeChat]=draft;
 for(auto i=drafts.begin();i!=drafts.end();)if(i->second.empty())i=drafts.erase(i);else ++i;
 o<<drafts.size()<<'\n';for(auto&d:drafts)o<<std::quoted(d.first)<<' '<<std::quoted(d.second)<<'\n';return sealArchive(o.str());
}
bool App::restore(const std::string&encoded){
 if(encoded.size()>MaxArchiveBytes)return false;std::string body;if(!unsealArchive(encoded,body))return false;
 std::istringstream in(body);std::string tag;size_t count;if(!(in>>tag>>count)||(tag!="MCH1"&&tag!="MCH2")||count>64)return false;
 const bool legacy=tag=="MCH1";
 std::vector<Chat> cs;std::deque<Message> ms;std::map<std::string,std::string> ds;
 for(size_t i=0;i<count;++i){Chat c;if(!(in>>std::quoted(c.id)>>std::quoted(c.name)>>c.channel>>c.unread>>std::quoted(c.notificationKey))||c.id.empty()||c.id.size()>96||!validText(c.name)||c.name.size()>64||c.notificationKey.size()>96)return false;for(auto&x:cs)if(x.id==c.id)return false;cs.push_back(c);}
 auto known=[&](const std::string&id){return std::any_of(cs.begin(),cs.end(),[&](const Chat&c){return c.id==id;});};
 if(!(in>>count)||count>MaxMessages)return false;
 for(size_t i=0;i<count;++i){Message m;int state;if(!(in>>std::quoted(m.chat)>>std::quoted(m.text)>>m.own>>state>>m.timestamp)||!known(m.chat)||!validText(m.text)||state<0||state>int(Delivery::Unconfirmed))return false;if(!legacy&&(!(in>>m.unread)||(m.own&&m.unread)))return false;m.delivery=static_cast<Delivery>(state);if(m.delivery==Delivery::Queued)m.delivery=Delivery::Unconfirmed;ms.push_back(m);}
 if(!(in>>count)||count>64)return false;
 for(size_t i=0;i<count;++i){std::string k,v;if(!(in>>std::quoted(k)>>std::quoted(v))||!known(k)||v.size()>MaxTextBytes||(!validText(v)&&v.find_first_not_of(' ')!=std::string::npos)||ds.count(k))return false;ds[k]=v;}
 if(in>>tag)return false;
 if(legacy)for(const auto& c:cs){unsigned left=c.unread;for(auto m=ms.rbegin();m!=ms.rend()&&left;++m)if(m->chat==c.id&&!m->own){m->unread=true;--left;}}
 chats=std::move(cs);messages=std::move(ms);drafts_=std::move(ds);activeChat.clear();draft.clear();recountUnread();archiveDirty=false;changed();return true;
}
bool App::restoreArchive(){auto s=backend_.readArchive();if(s.empty())return true;if(restore(s))return true;notice=translate(settings.language,"Verlauf beschädigt");return false;}
bool App::saveArchive(){if(!archiveDirty)return true;if(backend_.writeArchive(archive())){archiveDirty=false;return true;}notice=translate(settings.language,"Verlauf nicht gespeichert");archiveDue=clockMs+10000;changed();return false;}
void App::tick(uint32_t now){clockMs=now;if(remoteDeadline&&int32_t(now-remoteDeadline)>=0)remoteReply(remotePeer,translate(settings.language,"Keine Antwort / Zeitlimit"));for(auto&m:messages)if(m.delivery==Delivery::Queued&&m.deadline&&int32_t(now-m.deadline)>=0){m.delivery=Delivery::Unconfirmed;m.deadline=0;scheduleArchive();changed();}if(archiveDirty&&int32_t(now-archiveDue)>=0)saveArchive();}
void App::acknowledged(const std::string&id,uint32_t ack){for(auto&m:messages)if(m.own&&m.chat==id&&m.ack==ack&&(m.delivery==Delivery::Queued||m.delivery==Delivery::Unconfirmed)){m.delivery=Delivery::Delivered;m.deadline=0;scheduleArchive();changed();}saveArchive();}
bool App::bindChannel(const std::string&id,const std::string&name,const std::string&key){
 auto c=std::find_if(chats.begin(),chats.end(),[&](const Chat&v){return v.id==id;});
 if(c!=chats.end()&&(name.empty()||(!c->notificationKey.empty()&&c->notificationKey!=key))){
  std::string archived="archive:"+(c->notificationKey.empty()?c->id:c->notificationKey);
  auto existing=std::find_if(chats.begin(),chats.end(),[&](const Chat&v){return v.id==archived;});
  for(auto&m:messages)if(m.chat==id)m.chat=archived;
  if(drafts_.count(id)){drafts_[archived]=drafts_[id];drafts_.erase(id);}
  if(activeChat==id)activeChat=archived;
  if(existing==chats.end()){c->id=archived;c->name+=" [archive]";}else chats.erase(c);
  c=chats.end();recountUnread();scheduleArchive();
 }
 if(name.empty())return true;
 if(c==chats.end()){if(chats.size()>=64){notice=translate(settings.language,"Chat-Limit erreicht");return false;}chats.push_back({id,name,true,0,key});scheduleArchive();changed();}
 else if(c->name!=name||c->notificationKey!=key){c->name=name;c->notificationKey=key;scheduleArchive();changed();}
 return true;
}
bool App::retry(bool flood){
 if(!settings.setupComplete||activeChat.rfind("archive:",0)==0)return false;
 auto c=std::find_if(chats.begin(),chats.end(),[&](const Chat&v){return v.id==activeChat;});if(c==chats.end())return false;
 for(auto i=messages.rbegin();i!=messages.rend();++i)if(i->chat==activeChat&&i->own&&(i->delivery==Delivery::Failed||i->delivery==Delivery::Unconfirmed)){
  if(flood&&!backend_.resetPath(activeChat)){notice=translate(settings.language,"Aktion fehlgeschlagen");changed();return false;}
  auto r=backend_.sendTracked(*c,i->text);i->delivery=r.ok?(status.simulation?Delivery::Simulated:Delivery::Queued):Delivery::Failed;i->ack=r.ack;i->deadline=r.timeout?clockMs+r.timeout:0;i->timestamp=status.epoch;
  if(r.ok&&draft==i->text)draft.clear();
  notice=translate(settings.language,r.ok?"Erneut gesendet":"Aktion fehlgeschlagen");scheduleArchive();saveArchive();changed();return r.ok;
 }
 notice=translate(settings.language,"Keine unbestätigte Nachricht");changed();return false;
}
}
