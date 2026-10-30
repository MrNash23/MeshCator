#pragma once
#include "App.h"
namespace mc {
// One UTF-8 TSV record per initial send/receive; delivery is the state at logging time.
inline std::string messageLogRecord(const Chat& c,const Message& m) {
 auto escape=[](const std::string& text){std::string out;for(char ch:text){switch(ch){
 case '\\':out+="\\\\";break;case '\t':out+="\\t";break;case '\r':out+="\\r";break;case '\n':out+="\\n";break;default:out+=ch;}}return out;};
 return std::to_string(m.timestamp)+"\t"+(m.own?"TX":"RX")+"\t"+(c.channel?"channel":"direct")+"\t"+escape(c.id)+"\t"+escape(c.name)+"\t"+std::to_string(int(m.delivery))+"\t"+escape(m.text)+"\n";
}
}
