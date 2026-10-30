#include "App.h"
#include <algorithm>
namespace mc {
std::string normalizeChannelKey(const std::string&s){
 const std::string digits="0123456789abcdef",alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
 if((s.size()==32||s.size()==64)&&s.find_first_not_of("0123456789abcdefABCDEF")==std::string::npos){auto out=s;for(char&c:out)if(c>='A'&&c<='F')c+=32;return out;}
 if(s.size()!=24&&s.size()!=44)return {};
 std::string bytes;unsigned bits=0,value=0;bool pad=false;int padding=0;
 for(char c:s){if(c=='='){pad=true;++padding;continue;}auto n=alphabet.find(c);if(pad||n==std::string::npos)return {};value=(value<<6)|n;bits+=6;if(bits>=8){bits-=8;bytes+=char((value>>bits)&255);}}
 if((bytes.size()!=16&&bytes.size()!=32)||padding!=(bytes.size()==16?2:1)||(value&((1u<<bits)-1)))return {};
 std::string out;for(unsigned char c:bytes){out+=digits[c>>4];out+=digits[c&15];}return out;
}

std::vector<size_t> App::visibleChats()const{
 std::vector<size_t> indices;
 for(size_t i=0;i<chats.size();++i)if(settings.chatFilter==0||(settings.chatFilter==1&&chats[i].channel)||(settings.chatFilter==2&&!chats[i].channel))indices.push_back(i);
 return indices;
}
void App::beginChannel(){channelEditing.clear();channelCreating=true;channelStage=channelKind=0;channelName.clear();channelSecret.clear();channelTyped=false;notice.clear();changed();}
bool App::handleChannelKey(Key k){
 if(!channelCreating||menuOpen||helpOpen)return false;
 if(k==Key::Home||k==Key::Menu)return false;
 if(k==Key::Delete&&channelStage!=1&&channelStage!=2)k=Key::Back;
 if(k==Key::Back){if(channelStage>0)--channelStage;else{channelCreating=false;channelSecret.clear();}channelTyped=false;}
 else if(k==Key::Delete&&(channelStage==1||channelStage==2)){eraseUtf8(channelStage==1?channelName:channelSecret);channelTyped=true;}
 else if(k==Key::Next||k==Key::Previous){if(channelStage==0)channelKind=1-channelKind;}
 else if(k==Key::Enter||k==Key::Confirm){
  if(channelStage==0){channelStage=1;channelTyped=false;}
  else if(channelStage==1){
   if(!validText(channelName)||channelName.size()>30){notice=translate(settings.language,"Ungültiger Wert");}
   else {if(channelKind==1){channelStage=2;if(channelSecret.empty())channelSecret=backend_.channelKey();}else channelStage=3;channelTyped=false;notice.clear();}
  }else if(channelStage==2){
   auto normalized=normalizeChannelKey(channelSecret);
   if(normalized.empty())notice=translate(settings.language,"128/256 Bit: Hex oder Base64");
   else{channelSecret=normalized;channelStage=3;notice.clear();}
  }else{
   if(channelEditing.empty()&&chats.size()>=64){notice=translate(settings.language,"Chat-Limit erreicht");changed();return true;}
   auto id=channelEditing.empty()?backend_.createChannel(channelName,channelSecret,channelKind==1):(backend_.updateChannel(channelEditing,channelName,channelSecret,channelKind==1)?channelEditing:std::string());
   if(id.empty())notice=translate(settings.language,"Aktion fehlgeschlagen");
   else{if(std::none_of(chats.begin(),chats.end(),[&](const Chat&c){return c.id==id;}))chats.push_back({id,channelKind==1?channelName:channelName[0]=='#'?channelName:"#"+channelName,true,0});else for(auto&c:chats)if(c.id==id)c.name=channelKind==1?channelName:channelName[0]=='#'?channelName:"#"+channelName;channelCreating=false;scheduleArchive();saveArchive();channelSecret.clear();notice=translate(settings.language,"Kanal gespeichert");}
  }
 }
 changed();return true;
}
}
