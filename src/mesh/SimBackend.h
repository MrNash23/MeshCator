#pragma once
#include "core/App.h"
namespace mc {
class SimBackend : public Backend {
  std::string path_;
  bool seeding_=false;
  std::map<std::string,std::pair<std::string,std::string>> channels_;
  bool saveChannels();
  unsigned channelSequence_ = 0;

public:
  explicit SimBackend(std::string path) : path_(std::move(path)) {}
  std::string createChannel(const std::string&,const std::string&,bool) override;
  bool channelInfo(const std::string&,std::string&,std::string&) override;
  bool updateChannel(const std::string&,const std::string&,const std::string&,bool) override;
  bool removeEntry(const std::string&,bool) override;
  bool setPath(const std::string&,const std::string&)override{return true;}
  bool setPermissions(const std::string&,uint8_t)override{return true;}
  bool useContactPermissions()override{return true;}
  bool renameContact(const std::string&,const std::string&) override {return true;}
  std::string exportEntry(const std::string&,bool) override;
  bool importEntry(const std::string&) override;
  std::string channelKey() override;
  bool readTile(int,int,int,std::vector<uint16_t>&) override;
  bool send(const Chat &, const std::string &) override {
    return true;
  }
  bool advert(bool) override {
    return true;
  }
  bool toggleFavorite(const std::string &) override { return true; }
  bool resetPath(const std::string &) override { return true; }
  bool appendMessageLog(const Chat&,const Message&) override;
  bool writeArchive(const std::string&) override;
  std::string readArchive() override;
  bool saveSettings(const Settings &) override;
  void load(App &);
  void seed(App &);
};
} // namespace mc
