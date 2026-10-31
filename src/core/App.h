#pragma once
#include "I18n.h"
#include "Configuration.h"
#include <array>
#include <cstdint>
#include <deque>
#include <map>
#include <string>
#include <vector>
namespace mc {
constexpr size_t MaxTextBytes = 150, MaxMessages = 512, MaxArchiveBytes = 256 * 1024;
enum class Screen { Chats, Contacts, Radar, Map, Signal, Settings, Conversation, Lock, Dashboard };
enum class Theme { Cards, Bands, Orange, Purple };
enum class Key {
  Context,
  Delete,
  Home,
  Confirm,
  Menu,
  Next,
  Previous,
  Enter,
  Back,
  Mode,
  ThemeNext,
  Lock,
  Advert,
  Flood,
  ZoomIn,
  ZoomOut
};
enum class ContextAction { OpenChat, Details, Favorite, ResetPath, Latest, Modes, Close, Notifications, Retry, RetryFlood, ClearHistory, Emoji, EditChannel, DeleteEntry, Share, Export, Import, Rename, Remote, Filter, NewChannel, SetPath, Ping, Trace, Telemetry, Permissions, ShowMap, QR };
enum class Delivery { Received, Queued, Failed, Simulated, Delivered, Unconfirmed };
struct Position {
  double lat = 0, lon = 0;
  bool valid = false;
};
struct Node {
  std::string id, name;
  Position position;
  float snr = 0;
  bool hasSnr = false;
  int hops = -1;
  bool favorite = false;
  int type = 1;
  uint32_t lastAdvert = 0;
  std::string path;
  uint8_t permissions = 0;
};
struct Chat {
  std::string id, name;
  bool channel = false;
  unsigned unread = 0;
  std::string notificationKey;
};
struct Message {
  std::string chat, text;
  bool own = false;
  Delivery delivery = Delivery::Received;
  uint32_t timestamp = 0;
  uint32_t ack = 0, deadline = 0;
  bool unread = false;
};
struct Status {
  int battery = -1, sats = 0;
  bool fix = false, simulation = false;
  uint32_t epoch = 0;
  Position position;
  bool usbPowered = false, charging = false;
};
struct Waypoint { std::string name; Position position; };
struct Settings {
  uint32_t version = 1;
  Theme theme = Theme::Cards;
  std::string name = "MeshCator";
  Language language = Language::English;
  uint8_t helpVersion = 0;
  bool setupComplete = false;
  int preset = -1;
  double frequency = 869.618, bandwidth = 62.5;
  int sf = 8, cr = 8, power = 14, hashBytes = 1;
  bool repeat = false, gps = true, shareLocation = false, bluetooth = false;
  std::string bluetoothName = "MeshCator", bluetoothPIN = "123456";
  int brightness = 80, timeout = 90, mapSource = 0;
  bool showContacts = true;
  int chatFilter = 0;
  std::vector<Waypoint> waypoints;
  bool notificationMute = false, notificationWake = true;
  int notificationMode = 2, notificationVolume = 40;
  std::map<std::string,int> notificationRules;
};
enum class Utility {None,DeleteEntry,ClearHistory,Import,Export,Rename,Emoji,RemoteMenu,RemoteLogin,RemoteCommand,RemoteOutput,ChatMenu,SetPath,Permissions,TelemetryPolicy,QR,NodeDetails,About};
struct SendResult {bool ok=false;uint32_t ack=0, timeout=0;};
struct Backend {
  virtual ~Backend() = default;
  virtual bool displayAwake() const { return true; }
  virtual void notifyReceived(bool, bool, bool, bool) {}
  virtual std::string createChannel(const std::string &, const std::string &, bool) { return {}; }
  virtual std::string channelKey() { return {}; }
  virtual bool readTile(int, int, int, std::vector<uint16_t>&) { return false; }
  virtual bool mapHome(Position&, int&) { return false; }
  virtual bool send(const Chat &, const std::string &) = 0;
  virtual SendResult sendTracked(const Chat &c,const std::string &s){return {send(c,s),0,0};}
  virtual bool channelInfo(const std::string&,std::string&,std::string&){return false;}
  virtual bool updateChannel(const std::string&,const std::string&,const std::string&,bool){return false;}
  virtual bool removeEntry(const std::string&,bool){return false;}
  virtual bool renameContact(const std::string&,const std::string&){return false;}
  virtual bool shareContact(const std::string&){return false;}
  virtual std::string exportEntry(const std::string&,bool){return {};}
  virtual bool importEntry(const std::string&){return false;}
  virtual bool remote(const std::string&,int,const std::string&){return false;}
  virtual bool setPath(const std::string&,const std::string&){return false;}
  virtual int telemetryPolicy()const{return 21;}
  virtual bool useContactPermissions(){return false;}
  virtual bool setPermissions(const std::string&,uint8_t){return false;}
  virtual bool appendMessageLog(const Chat&, const Message&){return true;}
  virtual bool writeArchive(const std::string&){return true;}
  virtual std::string readArchive(){return {};}
  virtual bool advert(bool flood) = 0;
  virtual bool toggleFavorite(const std::string &) { return false; }
  virtual bool resetPath(const std::string &) { return false; }
  virtual bool saveSettings(const Settings &) = 0;
};
struct Bearing {
  double meters, degrees;
};
Bearing locate(Position from, Position to);
bool validText(const std::string &);
void eraseUtf8(std::string &);
std::string normalizeChannelKey(const std::string&);
std::string sealArchive(const std::string&);
bool unsealArchive(const std::string&,std::string&);
class App {
  Backend &backend_;
  std::map<std::string, std::string> drafts_;

public:
  explicit App(Backend &backend) : backend_(backend) {}
  Screen screen = Screen::Dashboard;
  Screen returnScreen = Screen::Dashboard;
  size_t returnSelected = 0;
  Settings settings;
  Status status;
  std::vector<Chat> chats;
  std::vector<Node> nodes;
  std::deque<Message> messages;
  std::deque<int> noise;
  std::vector<Position> track;
  size_t selected = 0;
  size_t historyOffset = 0;
  std::string activeChat, draft, notice;
  double radius = 2000;
  int mapZoom = 2;
  Position mapCenter{0, 0, false};
  Position offlineMapCenter{0, 0, false};
  bool mapHomeLoaded = false;
  void prepareMap();
  bool panMap(char key);
  std::string mapCaption;
  int navigationTarget = -1;
  bool waypointNaming = false, waypointList = false;
  std::string waypointName;
  size_t waypointSelected = 0;
  Position waypointPosition;
  int telemetryPolicy()const{return backend_.telemetryPolicy();}
  bool handleMapKey(Key);
  bool notificationMenu = false;
  int notificationChoice = 0;
  std::string notificationTarget;
  std::string notificationKey(const std::string &) const;
  int notificationMode(const std::string &) const;
  bool handleNotificationKey(Key);
  std::vector<size_t> visibleChats() const;
  bool channelCreating = false, channelTyped = false;
  int channelStage = 0, channelKind = 0;
  std::string channelName, channelSecret;
  std::string channelEditing;
  void beginChannel();
  bool handleChannelKey(Key);
  bool readTile(int z,int x,int y,std::vector<uint16_t>& pixels) { return backend_.readTile(z,x,y,pixels); }
  bool helpOpen = false, firstRunHelp = false;
  size_t helpPage = 0;
  void startIntroduction();
  void openHelp(bool firstRun = false);
  bool contextOpen = false, contextDetails = false;
  size_t contextSelected = 0;
  std::string contextId;
  bool contextChannel = false;
  std::vector<ContextAction> contextActions() const;
  void showContext();
  void openNodeChat(const std::string &);
  int settingsCategory = -1;
  SettingField editField = SettingField::Language;
  std::string editValue;
  bool editTyped = false, setupWizard = false;
  int setupStep = 0;
  Settings settingsBeforeEdit;
  void beginSetting(SettingField);
  bool commitSetting();
  void beginSetup();
  bool handleSettingsKey(Key);
  bool settingsEditing = false;
  int settingChoice = 0;
  bool mapFullscreen = false, mapMenuOpen = false;
  size_t mapMenuSelected = 0;
  bool menuOpen = false;
  size_t menuSelected = 0;
  bool dirty = true, paused = false;
  uint64_t revision = 0;
  Utility utility=Utility::None;
  std::string utilityTarget,utilityText,remotePeer;
  bool utilityChannel=false;
  size_t utilityChoice=0;
  uint32_t remoteDeadline=0;
  bool handleUtility(Key);
  bool action(ContextAction);
  void utilityOpen(Utility,const std::string& text="");
  void remoteReply(const std::string&,const std::string&);
  uint32_t clockMs=0, archiveDue=0;
  bool archiveDirty=false;
  void tick(uint32_t);
  void acknowledged(const std::string&,uint32_t);
  bool retry(bool flood=false);
  void scheduleArchive();
  bool saveArchive();
  bool restoreArchive();
  std::string archive() const;
  bool restore(const std::string&);
  void key(Key);
  bool type(const std::string &);
  bool send();
  bool bindChannel(const std::string&,const std::string&,const std::string&);
  void receive(const std::string &chat, const std::string &name, const std::string &text,
               bool channel, uint32_t timestamp = 0);
  void sample(int dbm);
  void changed() {
    dirty = true;
    ++revision;
  }
  unsigned unreadBadgeCount(bool channel) const;
  void readCurrentMessage();
  void recountUnread();
  void open(size_t);
  void persist();
  size_t itemCount() const;
};
} // namespace mc
