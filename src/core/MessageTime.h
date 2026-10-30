#pragma once
#include <cstdint>
#include <ctime>
#include <string>
namespace mc {
inline std::string messageTime(uint32_t timestamp){
 if(!timestamp)return "--.--.-- --:-- UTC";
 time_t value=timestamp;auto tm=std::gmtime(&value);char text[32];
 if(!tm||!std::strftime(text,sizeof(text),"%d.%m.%y %H:%M UTC",tm))return "--.--.-- --:-- UTC";
 return text;
}
}
