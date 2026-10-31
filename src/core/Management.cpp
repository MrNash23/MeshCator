#include "App.h"
#include "Emoji.h"
#include "NodePath.h"
#include "UtilityText.h"
#include <algorithm>
#include <ctime>
#include <cstdio>
namespace mc {
void App::utilityOpen(Utility u,const std::string&t){utility=u;utilityText=t;utilityChoice=0;contextOpen=false;notice.clear();changed();}
bool App::action(ContextAction a){
 utilityTarget=contextId;utilityChannel=contextChannel;
 if(a==ContextAction::Filter)utilityOpen(Utility::ChatMenu);
 else if(a==ContextAction::NewChannel){contextOpen=false;beginChannel();}
 else if(a==ContextAction::Import)utilityOpen(Utility::Import);
 else if(a==ContextAction::ClearHistory)utilityOpen(Utility::ClearHistory);
 else if(a==ContextAction::DeleteEntry)utilityOpen(Utility::DeleteEntry);
 else if(a==ContextAction::Emoji)utilityOpen(Utility::Emoji);
 else if(a==ContextAction::Export){auto data=backend_.exportEntry(contextId,contextChannel);utilityOpen(Utility::Export,data.empty()?translate(settings.language,"Aktion fehlgeschlagen"):data);}
 else if(a==ContextAction::Rename){std::string n;for(auto&v:nodes)if(v.id==contextId)n=v.name;utilityOpen(Utility::Rename,n);}
 else if(a==ContextAction::Remote)utilityOpen(Utility::RemoteMenu);
 else if(a==ContextAction::SetPath){std::string path;for(auto&n:nodes)if(n.id==contextId)path=n.path;utilityOpen(Utility::SetPath,path);}
 else if(a==ContextAction::Permissions)utilityOpen(Utility::Permissions);
 else if(a==ContextAction::QR){for(auto&n:nodes)if(n.id==contextId){std::string name;const char*d="0123456789ABCDEF";for(unsigned char c:n.name){if((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'||c=='~')name+=c;else{name+='%';name+=d[c>>4];name+=d[c&15];}}utilityOpen(Utility::QR,"meshcore://contact/add?name="+name+"&public_key="+n.id+"&type="+std::to_string(n.type));}}
 else if(a==ContextAction::ShowMap){for(auto&n:nodes)if(n.id==contextId&&n.position.valid){mapCenter=n.position;mapCaption=n.name;mapZoom=10;contextOpen=false;screen=Screen::Map;}}
 else if(a==ContextAction::Details&&!contextChannel){
  for(auto&n:nodes)if(n.id==contextId){
   auto tr=[&](const char*s){return translate(settings.language,s);};
   std::string text=n.name+"\n"+(n.type==2?"Repeater":n.type==3?"Room server":n.type==4?"Sensor":"Companion")+"\n"+n.id;
   text+="\n"+tr("Ausgangspfad")+": "+(n.hops<0?"Flood":n.hops==0?"Zero hop":n.path);
   text+="\nHops: "+(n.hops<0?std::string("Flood"):std::to_string(n.hops));
   std::string date="?";if(n.lastAdvert){time_t stamp=n.lastAdvert;auto tm=std::gmtime(&stamp);if(tm){char buf[32];std::strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M UTC",tm);date=buf;}}
   text+="\n"+tr("Letztes Advert")+": "+date;
   if(n.lastAdvert&&status.epoch>=n.lastAdvert)text+=" ("+std::to_string(status.epoch-n.lastAdvert)+" s)";
   if(n.position.valid){text+="\n"+std::to_string(n.position.lat)+", "+std::to_string(n.position.lon);if(status.position.valid)text+="\n"+std::to_string(int(locate(status.position,n.position).meters))+" m";}
   utilityOpen(Utility::NodeDetails,text);break;
  }
 }
 else if(a==ContextAction::Ping||a==ContextAction::Trace||a==ContextAction::Telemetry){
  if(a==ContextAction::Trace){bool valid=false;for(auto&n:nodes)if(n.id==contextId){uint8_t code,flags,key[4]={};std::vector<uint8_t> path,route;valid=n.hops>=0&&parseNodePath(n.path,code,path)&&nodeTracePath(code,path.data(),key,n.type==2,route,flags);}if(!valid){notice=translate(settings.language,"Trace: kurzer Pfad mit 1/2-Byte-Hashes nötig");changed();return true;}}
  remotePeer=contextId;remoteDeadline=clockMs+30000;int op=a==ContextAction::Ping?4:a==ContextAction::Trace?5:6;if(settings.setupComplete&&backend_.remote(remotePeer,op,""))utilityOpen(Utility::RemoteOutput,translate(settings.language,"Warte auf Antwort"));else{remoteDeadline=0;notice=translate(settings.language,"Aktion fehlgeschlagen");}}
 else if(a==ContextAction::Share){contextOpen=false;notice=translate(settings.language,settings.setupComplete&&backend_.shareContact(contextId)?"Advert eingeplant":"Aktion fehlgeschlagen");}
 else if(a==ContextAction::EditChannel){std::string name,key;if(!backend_.channelInfo(contextId,name,key)){notice=translate(settings.language,"Aktion fehlgeschlagen");return true;}beginChannel();channelEditing=contextId;channelKind=1;channelName=name;channelSecret=key;channelStage=1;contextOpen=false;}
 else return false;
 changed();return true;
}
void App::remoteReply(const std::string&id,const std::string&text){if(id!=remotePeer||!remoteDeadline)return;remoteDeadline=0;utilityOpen(Utility::RemoteOutput,text.substr(0,1024));}
bool App::handleUtility(Key k){
 if(utility==Utility::None)return false;
 if(k==Key::Home||k==Key::Menu){remoteDeadline=0;utility=Utility::None;utilityText.clear();return false;}
 if(k==Key::Back){remoteDeadline=0;utility=Utility::None;utilityText.clear();changed();return true;}
 bool input=utility==Utility::Rename||utility==Utility::Import||utility==Utility::RemoteLogin||utility==Utility::RemoteCommand||utility==Utility::SetPath;
 if(k==Key::Delete){if(input)eraseUtf8(utilityText);else{utility=Utility::None;utilityText.clear();}changed();return true;}
 size_t count=utility==Utility::TelemetryPolicy?2:utility==Utility::Permissions?4:utility==Utility::Emoji?emojiCount:utility==Utility::ChatMenu?5:utility==Utility::RemoteMenu?12:utility==Utility::DeleteEntry||utility==Utility::ClearHistory?2:input?1:utility==Utility::QR?1:utilityPages(utilityText).size();
 if(k==Key::Next||k==Key::Previous)utilityChoice=(utilityChoice+count+(k==Key::Next?1:-1))%count;
 if(k==Key::Confirm||k==Key::Enter){
  auto u=utility;
  if(u==Utility::SetPath){uint8_t encoded;std::vector<uint8_t> bytes;if(parseNodePath(utilityText,encoded,bytes)&&backend_.setPath(utilityTarget,utilityText)){for(auto&n:nodes)if(n.id==utilityTarget){n.path=formatNodePath(encoded,bytes.data());n.hops=encoded&63;}utility=Utility::None;}else notice=translate(settings.language,"Aktion fehlgeschlagen");}
  else if(u==Utility::TelemetryPolicy){if(utilityChoice==0)utilityOpen(Utility::Permissions);else if(backend_.useContactPermissions())utilityOpen(Utility::Permissions);else notice=translate(settings.language,"Aktion fehlgeschlagen");}
  else if(u==Utility::Permissions){if(utilityChoice==3){utilityOpen(Utility::TelemetryPolicy);changed();return true;}for(auto&n:nodes)if(n.id==utilityTarget){uint8_t bits=n.permissions^(1<<utilityChoice);if(backend_.setPermissions(utilityTarget,bits))n.permissions=bits;else notice=translate(settings.language,"Aktion fehlgeschlagen");}}
  else if(u==Utility::Emoji){auto e=emojis[utilityChoice];utility=Utility::None;if(!type(e)){utility=Utility::Emoji;notice=translate(settings.language,"Maximal 150 UTF-8 Bytes");}}
  else if(u==Utility::DeleteEntry||u==Utility::ClearHistory){
   if(utilityChoice==0){utility=Utility::None;changed();return true;}
   if(u==Utility::DeleteEntry&&utilityTarget.rfind("archive:",0)!=0&&!backend_.removeEntry(utilityTarget,utilityChannel)){notice=translate(settings.language,"Aktion fehlgeschlagen");changed();return true;}
   messages.erase(std::remove_if(messages.begin(),messages.end(),[&](const Message&m){return m.chat==utilityTarget;}),messages.end());
   if(u==Utility::DeleteEntry){chats.erase(std::remove_if(chats.begin(),chats.end(),[&](const Chat&c){return c.id==utilityTarget;}),chats.end());nodes.erase(std::remove_if(nodes.begin(),nodes.end(),[&](const Node&n){return n.id==utilityTarget;}),nodes.end());drafts_.erase(utilityTarget);if(activeChat==utilityTarget){draft.clear();activeChat.clear();}screen=Screen::Chats;selected=0;}
   recountUnread();utility=Utility::None;scheduleArchive();saveArchive();
  }else if(u==Utility::Rename){if(validText(utilityText)&&utilityText.size()<=31&&backend_.renameContact(utilityTarget,utilityText)){for(auto&n:nodes)if(n.id==utilityTarget)n.name=utilityText;for(auto&c:chats)if(c.id==utilityTarget)c.name=utilityText;utility=Utility::None;scheduleArchive();saveArchive();}else notice=translate(settings.language,"Aktion fehlgeschlagen");}
  else if(u==Utility::Import){if(backend_.importEntry(utilityText)){utility=Utility::None;notice=translate(settings.language,"Importiert");}else notice=translate(settings.language,"Aktion fehlgeschlagen");}
  else if(u==Utility::ChatMenu){auto choice=utilityChoice;utility=Utility::None;if(choice<3){auto next=settings;next.chatFilter=choice;if(backend_.saveSettings(next)){settings=next;selected=0;}else notice=translate(settings.language,"Einstellungen nicht gespeichert");}else if(choice==3)beginChannel();else utilityOpen(Utility::Import);}
  else if(u==Utility::RemoteMenu){auto choice=utilityChoice;if(choice==0)utilityOpen(Utility::RemoteLogin);else if(choice>=3){const char* commands[]={"","get radio","get tx","get advert.interval","get repeat","set name ","set tx ","set advert.interval ","set repeat "};utilityOpen(Utility::RemoteCommand,commands[choice-3]);}else {remotePeer=utilityTarget;remoteDeadline=clockMs+30000;if(settings.setupComplete&&backend_.remote(remotePeer,int(choice),""))utilityOpen(Utility::RemoteOutput,translate(settings.language,"Warte auf Antwort"));else{remoteDeadline=0;notice=translate(settings.language,"Aktion fehlgeschlagen");}}}
  else if(u==Utility::RemoteLogin||u==Utility::RemoteCommand){remotePeer=utilityTarget;remoteDeadline=clockMs+30000;bool ok=settings.setupComplete&&!utilityText.empty()&&backend_.remote(remotePeer,u==Utility::RemoteLogin?0:3,utilityText);utilityText.clear();if(ok)utilityOpen(Utility::RemoteOutput,translate(settings.language,"Warte auf Antwort"));else{remoteDeadline=0;notice=translate(settings.language,"Aktion fehlgeschlagen");}}
  else utility=Utility::None;
 }
 changed();return true;
}
}
