#pragma once
#include <string>
namespace mc {
enum class Language { English, French, Spanish, German };
std::string translate(Language language, const std::string &key);
const char *languageName(Language language);
}
