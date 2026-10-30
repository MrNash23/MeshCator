#include "core/App.h"
#include <algorithm>
#include <cmath>
namespace mc {
Bearing locate(Position a, Position b) {
  constexpr double pi = 3.14159265358979323846;
  double p = a.lat * pi / 180, q = b.lat * pi / 180, d = (b.lon - a.lon) * pi / 180;
  double h =
      std::pow(std::sin((q - p) / 2), 2) + std::cos(p) * std::cos(q) * std::pow(std::sin(d / 2), 2);
  return {
      6371000 * 2 * std::asin(std::sqrt(std::clamp(h, 0.0, 1.0))),
      std::fmod(std::atan2(std::sin(d) * std::cos(q),
                           std::cos(p) * std::sin(q) - std::sin(p) * std::cos(q) * std::cos(d)) *
                        180 / pi +
                    360,
                360)};
}
bool validText(const std::string &s) {
  if (s.empty() || s.size() > MaxTextBytes)
    return false;
  bool visible = false;
  for (size_t i = 0; i < s.size();) {
    auto c = static_cast<unsigned char>(s[i]);
    if (c < 32 || c == 127)
      return false;
    visible |= c != ' ';
    if (c < 128) {
      ++i;
      continue;
    }
    size_t n = c >= 0xC2 && c <= 0xDF   ? 2
               : c >= 0xE0 && c <= 0xEF ? 3
               : c >= 0xF0 && c <= 0xF4 ? 4
                                        : 0;
    if (!n || i + n > s.size())
      return false;
    for (size_t j = 1; j < n; ++j)
      if ((static_cast<unsigned char>(s[i + j]) & 0xC0) != 0x80)
        return false;
    auto d = static_cast<unsigned char>(s[i + 1]);
    if ((c == 0xE0 && d < 0xA0) || (c == 0xED && d >= 0xA0) || (c == 0xF0 && d < 0x90) ||
        (c == 0xF4 && d >= 0x90))
      return false;
    i += n;
  }
  return visible;
}
void eraseUtf8(std::string &s) {
  if (s.empty())
    return;
  size_t i = s.size() - 1;
  while (i > 0 && (static_cast<unsigned char>(s[i]) & 0xC0) == 0x80)
    --i;
  s.resize(i);
}
size_t App::itemCount() const {
  if(screen==Screen::Dashboard)return 6;
  if (screen == Screen::Chats)
    return visibleChats().size()+1;
  if (screen == Screen::Contacts || screen == Screen::Radar)
    return nodes.size();
  return 1;
}
void App::persist() {
  if (!backend_.saveSettings(settings))
    notice = translate(settings.language, "Einstellungen nicht gespeichert");
  changed();
}
void App::open(size_t i) {
  if (i >= chats.size())
    return;
  if (!activeChat.empty())
    drafts_[activeChat] = draft;
  if (screen != Screen::Conversation) { returnScreen = screen; returnSelected = selected; }
  activeChat = chats[i].id;
  draft = drafts_[activeChat];
  chats[i].unread = 0;
  scheduleArchive();
  screen = Screen::Conversation;
  historyOffset = 0;
  changed();
}
bool App::type(const std::string &s) {
  if(utility!=Utility::None){if(utility!=Utility::Import&&utility!=Utility::Rename&&utility!=Utility::RemoteLogin&&utility!=Utility::RemoteCommand&&utility!=Utility::SetPath)return false;size_t cap=utility==Utility::SetPath?130:utility==Utility::Import?600:utility==Utility::Rename?31:120;if(utilityText.size()+s.size()>cap)return false;for(unsigned char c:s)if(c<32||c==127)return false;utilityText+=s;changed();return true;}

  if(channelCreating&&!menuOpen){
    if(channelStage!=1&&channelStage!=2)return false;
    auto &value=channelStage==1?channelName:channelSecret;
    std::string next=(channelTyped?value:std::string())+s;
    if(next.size()>(channelStage==1?30:64)||(!validText(next)&&next.find_first_not_of(' ')!=std::string::npos))return false;
    value=next;channelTyped=true;changed();return true;
  }
  if (waypointNaming && screen == Screen::Map) {if(waypointName.size()+s.size()>31)return false;auto next=waypointName+s;if(!validText(next)&&next.find_first_not_of(' ')!=std::string::npos)return false;waypointName=next;changed();return true;}
  if (screen == Screen::Settings && settingsEditing && settingOptions(editField).empty() && !menuOpen && !helpOpen) {
    std::string next = (editTyped ? editValue : std::string()) + s;
    if (next.size() > 31 || (!validText(next) && next.find_first_not_of(' ') != std::string::npos)) return false;
    editValue = next; editTyped = true; changed(); return true;
  }
  if (helpOpen || contextOpen || menuOpen || notificationMenu || screen != Screen::Conversation)
    return false;
  if (draft.size() + s.size() > MaxTextBytes) {
    notice = translate(settings.language, "Maximal 150 UTF-8 Bytes");
    changed();
    return false;
  }
  std::string next = draft + s;
  if (!validText(next) && next.find_first_not_of(' ') != std::string::npos)
    return false;
  draft = next;
  scheduleArchive();
  changed();
  return true;
}
bool App::send() {
  if(activeChat.rfind("archive:",0)==0){notice=translate(settings.language,"Archiv: nur lesen");changed();return false;}
  if (!settings.setupComplete) { notice=translate(settings.language,"Senden gesperrt");changed();return false; }
  if (!validText(draft)) {
    notice = translate(settings.language, "Leere oder ungueltige Nachricht");
    changed();
    return false;
  }
  auto it =
      std::find_if(chats.begin(), chats.end(), [&](const Chat &c) { return c.id == activeChat; });
  if (it == chats.end()) {
    notice = translate(settings.language, "Kontakt nicht vorhanden");
    changed();
    return false;
  }
  auto result = backend_.sendTracked(*it, draft);
  bool ok=result.ok;
  messages.push_back(
      {activeChat, draft, true,
       ok ? (status.simulation ? Delivery::Simulated : Delivery::Queued) : Delivery::Failed,
       status.epoch,result.ack,result.timeout?clockMs+result.timeout:0});
  while (messages.size() > MaxMessages)
    messages.pop_front();
  if (ok) {
    draft.clear();
    notice = status.simulation ? translate(settings.language, "Simuliert / kein Funk") : translate(settings.language, "An Funkwarteschlange uebergeben");
  } else
    notice = translate(settings.language, "Senden fehlgeschlagen / Entwurf erhalten");
  if(!backend_.appendMessageLog(*it,messages.back()))notice=translate(settings.language,"SD-Archiv nicht gespeichert");
  scheduleArchive();saveArchive();
  changed();
  return ok;
}
void App::receive(const std::string &id, const std::string &name, const std::string &text,
                  bool channel, uint32_t timestamp) {
  if (!validText(text))
    return;
  auto c = std::find_if(chats.begin(), chats.end(), [&](const Chat &x) { return x.id == id; });
  if (c == chats.end()) {
    if (chats.size() >= 64)
      return;
    chats.push_back({id, name, channel, 0});
    c = chats.end() - 1;
  }
  const Chat logChat=*c;
  const bool wasAsleep=!backend_.displayAwake();
  bool reading=!wasAsleep&&screen==Screen::Conversation&&activeChat==id&&!menuOpen&&!contextOpen&&!helpOpen&&!notificationMenu;
  if(utility!=Utility::None)reading=false;
  if(!reading)++c->unread;
  int alertMode=notificationMode(id);
  if(!reading&&alertMode)backend_.notifyReceived(alertMode&1,alertMode&2,settings.notificationWake,channel);
  messages.push_back({id, text, false, Delivery::Received, timestamp ? timestamp : status.epoch});
  while (messages.size() > MaxMessages)
    messages.pop_front();
  if(wasAsleep&&alertMode&&settings.notificationWake&&screen!=Screen::Lock){
    size_t index=static_cast<size_t>(c-chats.begin());
    key(Key::Home); // close overlays and preserve the current message draft
    open(index);historyOffset=0;
  }
  if(!backend_.appendMessageLog(logChat,messages.back()))notice=translate(settings.language,"SD-Archiv nicht gespeichert");
  scheduleArchive();saveArchive();
  changed();
}
void App::sample(int n) {
  if (paused || n > 0 || n < -150)
    return;
  noise.push_back(n);
  while (noise.size() > 60)
    noise.pop_front();
  if (screen == Screen::Signal)
    changed();
}
std::vector<ContextAction> App::contextActions() const {
  if(contextId.rfind("archive:",0)==0)return {ContextAction::Latest,ContextAction::ClearHistory,ContextAction::DeleteEntry,ContextAction::Close};
  std::vector<ContextAction> actions;
  if (screen == Screen::Conversation) actions.push_back(ContextAction::Latest);
  else {auto node=std::find_if(nodes.begin(),nodes.end(),[&](const Node&n){return n.id==contextId;});if(node==nodes.end()||node->type==1)actions.push_back(ContextAction::OpenChat);}
  actions.push_back(ContextAction::Details);
  if (!contextChannel && std::any_of(nodes.begin(), nodes.end(), [&](const Node &n) { return n.id == contextId; })) {
    actions.push_back(ContextAction::Favorite);
    actions.push_back(ContextAction::ResetPath);
    actions.push_back(ContextAction::SetPath);
    actions.push_back(ContextAction::Ping);actions.push_back(ContextAction::Trace);
    actions.push_back(ContextAction::Telemetry);actions.push_back(ContextAction::Permissions);
    auto n=std::find_if(nodes.begin(),nodes.end(),[&](const Node&v){return v.id==contextId;});
    if(n->position.valid)actions.push_back(ContextAction::ShowMap);
    actions.push_back(ContextAction::Rename);actions.push_back(ContextAction::Share);actions.push_back(ContextAction::QR);
    if(n->type==2||n->type==3)actions.push_back(ContextAction::Remote);
  }
  if(screen==Screen::Conversation){actions.push_back(ContextAction::Retry);if(!contextChannel)actions.push_back(ContextAction::RetryFlood);actions.push_back(ContextAction::ClearHistory);actions.push_back(ContextAction::Emoji);}
  if(contextChannel)actions.push_back(ContextAction::EditChannel);
  actions.push_back(ContextAction::Export);actions.push_back(ContextAction::DeleteEntry);
  actions.push_back(ContextAction::Notifications);
  if(screen==Screen::Conversation){actions.push_back(ContextAction::Filter);actions.push_back(ContextAction::NewChannel);actions.push_back(ContextAction::Import);}
  actions.push_back(ContextAction::Modes);
  actions.push_back(ContextAction::Close);
  return actions;
}
void App::showContext() {
  if(screen==Screen::Chats||screen==Screen::Contacts&&nodes.empty()){utilityOpen(Utility::ChatMenu);return;}
  if (screen == Screen::Conversation) {
    contextId = activeChat;
    auto c = std::find_if(chats.begin(), chats.end(), [&](const Chat &c) { return c.id == activeChat; });
    if (c == chats.end()) return;
    contextChannel = c->channel;
  } else if ((screen == Screen::Contacts || screen == Screen::Radar) && selected < nodes.size()) {
    contextId = nodes[selected].id;
    contextChannel = false;
  } else { key(Key::Menu); return; }
  contextSelected = 0;
  contextDetails = false;
  contextOpen = true;
  notice.clear();
  changed();
}
void App::openNodeChat(const std::string &id) {
  auto n = std::find_if(nodes.begin(), nodes.end(), [&](const Node &n) { return n.id == id; });
  if (n == nodes.end()) return;
  auto c = std::find_if(chats.begin(), chats.end(), [&](const Chat &c) { return c.id == id; });
  if (c == chats.end()) {
    if (chats.size() >= 64) { notice = translate(settings.language, "Chat-Limit erreicht"); return; }
    chats.push_back({id, n->name, false, 0});
    open(chats.size() - 1);
  } else open(static_cast<size_t>(c - chats.begin()));
}
void App::openHelp(bool firstRun) {
  helpOpen = true;
  firstRunHelp = firstRun;
  helpPage = 0;
  notice.clear();
  changed();
}
void App::startIntroduction() {
  if (!settings.setupComplete) beginSetup();
  else if (settings.helpVersion < 1) openHelp(true);
}
void App::key(Key k) {
  if(handleUtility(k))return;
  if (handleNotificationKey(k) || handleChannelKey(k) || handleSettingsKey(k) || handleMapKey(k)) return;
  if (k == Key::Delete) {
    if (screen == Screen::Conversation && !helpOpen && !menuOpen && !contextOpen) {
      eraseUtf8(draft);scheduleArchive();
      changed();
      return;
    }
    k = Key::Back;
  }
  if (k == Key::Home) {
    if (!activeChat.empty()) drafts_[activeChat] = draft;
    utility=Utility::None;utilityText.clear();
    helpOpen = false;
    menuOpen = false;
    contextOpen = contextDetails = false;
    settingsEditing = false;
    mapFullscreen = mapMenuOpen = waypointNaming = waypointList = false;
    notificationMenu=false;
    setupWizard = channelCreating = false; channelSecret.clear(); settingsCategory = -1;
    screen = Screen::Dashboard;
    selected = 0;
    historyOffset = 0;
    notice.clear();
    changed();
    return;
  }
  if (helpOpen) {
    if (k == Key::Back || k == Key::Delete) {
      if (helpPage) --helpPage;
      else helpOpen = false;
    } else if (k == Key::Next) helpPage = std::min<size_t>(3, helpPage + 1);
    else if (k == Key::Previous) { if (helpPage) --helpPage; }
    else if (k == Key::Confirm || k == Key::Enter) {
      if (helpPage < 3) ++helpPage;
      else {
        auto previous = settings.helpVersion;
        settings.helpVersion = 1;
        if (previous >= 1 || backend_.saveSettings(settings)) {
          helpOpen = false;
          if (firstRunHelp) { screen = Screen::Dashboard; selected = 0; }
        } else {
          settings.helpVersion = previous;
          notice = translate(settings.language, "Einstellungen nicht gespeichert");
        }
      }
    } else if (k == Key::Menu || k == Key::Context) {
      helpOpen = false;
      key(Key::Menu);
    }
    changed();
    return;
  }
  if (k == Key::Confirm) {
    if (!contextOpen && !menuOpen && screen == Screen::Conversation) { showContext(); return; }
    k = Key::Enter;
  }
  if (screen == Screen::Lock && k == Key::Menu) screen = Screen::Dashboard;
  if (screen == Screen::Lock) {
    if (k == Key::Enter || k == Key::Lock) {
      screen = Screen::Dashboard;
      changed();
    }
    return;
  }
  if (k == Key::Context) { if (contextOpen) { contextOpen = false; changed(); } else showContext(); return; }
  if (k == Key::Menu) {
    notificationMenu=false;
    setupWizard = channelCreating = waypointNaming = waypointList = false;
    channelSecret.clear();
    contextOpen = contextDetails = false;
    mapMenuOpen = false;
    settingsEditing = false;
    menuOpen = !menuOpen;
    menuSelected = static_cast<int>(screen) < 6 ? static_cast<int>(screen) : 0;
    changed();
    return;
  }
  if (contextOpen) {
    auto actions = contextActions();
    if (contextDetails) {
      if (k == Key::Back || k == Key::Enter) contextDetails = false;
    } else if (k == Key::Next || k == Key::Previous) {
      contextSelected = (contextSelected + actions.size() + (k == Key::Next ? 1 : -1)) % actions.size();
    } else if (k == Key::Back) contextOpen = false;
    else if (k == Key::Enter && contextSelected < actions.size()) {
      auto action = actions[contextSelected];
      if(this->action(action)){}
      else if(action==ContextAction::Retry||action==ContextAction::RetryFlood){contextOpen=false;retry(action==ContextAction::RetryFlood);}
      else if (action == ContextAction::Details) contextDetails = true;
      else if (action == ContextAction::Notifications) {
        notificationTarget=notificationKey(contextId);auto rule=settings.notificationRules.find(notificationTarget);
        notificationChoice=rule==settings.notificationRules.end()?0:rule->second+1;notificationMenu=true;contextOpen=false;
      }
      else if (action == ContextAction::Close) contextOpen = false;
      else if (action == ContextAction::Modes) { key(Key::Menu); return; }
      else if (action == ContextAction::OpenChat) { contextOpen = false; openNodeChat(contextId); }
      else if (action == ContextAction::Latest) { historyOffset = 0; contextOpen = false; }
      else {
        bool ok = action == ContextAction::Favorite ? backend_.toggleFavorite(contextId) : backend_.resetPath(contextId);
        if (ok) for (auto &n : nodes) if (n.id == contextId) {
          if (action == ContextAction::Favorite) n.favorite = !n.favorite;
          else {n.hops = -1;n.path.clear();}
        }
        contextOpen = false;
        notice = translate(settings.language, !ok ? "Aktion fehlgeschlagen" : status.simulation ? "Simuliert / kein Funk" : action == ContextAction::Favorite ? "Favorit gespeichert" : "Pfad zurückgesetzt");
      }
    }
    changed();
    return;
  }
  if (menuOpen) {
    if (k == Key::Next || k == Key::Previous)
      menuSelected = (menuSelected + (k == Key::Next ? 1 : 5)) % 6;
    else if (k == Key::Enter) {
      if (screen == Screen::Conversation) drafts_[activeChat] = draft;
      screen = static_cast<Screen>(menuSelected);
      if(screen==Screen::Settings){settingsCategory=-1;settingsEditing=false;}
      if (screen != Screen::Map) mapFullscreen = false;
      selected = 0;
      menuOpen = false;
    } else if (k == Key::Back) menuOpen = false;
    changed();
    return;
  }
  switch (k) {
  case Key::Context: break;
  case Key::Delete: break;
  case Key::Home: break;
  case Key::Confirm: break;
  case Key::Menu: break;
  case Key::Next:
  case Key::Previous: {
    if (screen == Screen::Map) {
      mapZoom = std::clamp(mapZoom + (k == Key::Next ? 1 : -1), 0, 18);
      break;
    }
    if (screen == Screen::Conversation) {
      size_t count = 0;
      for (auto &m : messages)
        if (m.chat == activeChat)
          ++count;
      if (count)
        historyOffset = k == Key::Previous ? std::min(historyOffset + 1, count - 1)
                                           : (historyOffset ? historyOffset - 1 : 0);
      break;
    }
    auto n = itemCount();
    if (n)
      selected = (std::min(selected, n - 1) + n + (k == Key::Next ? 1 : -1)) % n;
    break;
  }
  case Key::Enter:
    if(screen==Screen::Dashboard){screen=static_cast<Screen>(std::min(size_t(5),selected));selected=0;if(screen==Screen::Settings){settingsCategory=-1;settingsEditing=false;}}
    else if (screen == Screen::Chats) {auto visible=visibleChats();if(selected<visible.size())open(visible[selected]);else utilityOpen(Utility::ChatMenu);}
    else if (screen == Screen::Conversation)
      send();
    else if (screen == Screen::Map) { mapMenuOpen = true; mapMenuSelected = 0; }
    else if (screen == Screen::Signal)
      paused = !paused;
    else if ((screen == Screen::Contacts || screen == Screen::Radar) && selected < nodes.size()) {
      showContext();
    }
    break;
  case Key::Back:
    if (screen == Screen::Map && mapFullscreen) { mapFullscreen = false; break; }
    if (screen == Screen::Conversation) {
      drafts_[activeChat] = draft;
      screen = returnScreen;
      selected = returnSelected;
    } else { screen = Screen::Dashboard; selected = 0; }
    break;
  case Key::Mode:
    if (screen == Screen::Conversation) {
      screen = Screen::Chats;
      break;
    }
    screen = static_cast<Screen>((static_cast<int>(screen) + 1) % 6);
    selected = 0;
    break;
  case Key::ThemeNext:
    settings.theme = static_cast<Theme>((static_cast<int>(settings.theme) + 1) % 4);
    persist();
    break;
  case Key::Lock:
    screen = Screen::Lock;
    break;
  case Key::Advert:
  case Key::Flood:
    if(!settings.setupComplete){notice=translate(settings.language,"Senden gesperrt");break;}
    notice = backend_.advert(k == Key::Flood)
                 ? (status.simulation ? translate(settings.language, "Advert simuliert") : translate(settings.language, "Advert eingeplant"))
                 : translate(settings.language, "Advert fehlgeschlagen");
    break;
  case Key::ZoomIn:
    if(screen==Screen::Map)mapZoom=std::min(18,mapZoom+1);else radius = std::max(100.0, radius / 2);
    break;
  case Key::ZoomOut:
    if(screen==Screen::Map)mapZoom=std::max(0,mapZoom-1);else radius = std::min(51200.0, radius * 2);
    break;
  }
  changed();
}
} // namespace mc
