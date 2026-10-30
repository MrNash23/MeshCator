#pragma once
#include <string>
#include <vector>
namespace mc {
// Five rows fit above the pager footer. Wrap by UTF-8 codepoints, never bytes.
inline std::vector<std::string> utilityPages(const std::string& text){
 std::vector<std::string> pages;std::string page;unsigned rows=0,columns=0;
 auto newline=[&](){columns=0;if(++rows==5){pages.push_back(page);page.clear();rows=0;}else page+='\n';};
 for(size_t i=0;i<text.size();){unsigned char c=text[i];if(c=='\n'){++i;newline();continue;}if(columns==42)newline();
  size_t n=c<128?1:(c&0xe0)==0xc0?2:(c&0xf0)==0xe0?3:4;n=std::min(n,text.size()-i);
  page.append(text,i,n);i+=n;++columns;
 }
 if(!page.empty()||pages.empty())pages.push_back(page);return pages;
}
}
