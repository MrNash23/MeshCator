#include "ui/View.h"
#include "core/UtilityText.h"
#include "core/Emoji.h"
#include "core/MessageTime.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
LV_FONT_DECLARE(meshcator_font_10);
LV_FONT_DECLARE(meshcator_font_12);
LV_FONT_DECLARE(meshcator_font_14);
LV_FONT_DECLARE(meshcator_font_16);
LV_FONT_DECLARE(meshcator_font_18);
LV_FONT_DECLARE(meshcator_font_24);
LV_FONT_DECLARE(meshcator_font_28);
LV_FONT_DECLARE(meshcator_font_36);
namespace mc {
namespace {
struct Palette {
  uint32_t bg, fg, a, b, c, d;
};
Palette palette(Theme t) {
  switch (t) {
  case Theme::Bands:
    return {0xF2F0E9, 0x161616, 0xF9EF55, 0x9BB9E7, 0xC5EE6A, 0xF0AFDE};
  case Theme::Orange:
    return {0x19130F, 0xFFF2E6, 0xFFAA5C, 0x79C9C0, 0x96D7A2, 0xF7CD71};
  case Theme::Purple:
    return {0x110E1B, 0xF6EFFF, 0xC58AFF, 0x77DCE8, 0x85E3B0, 0xFF8BD3};
  default:
    return {0x161616, 0xF2F0E9, 0xF9EF55, 0xAA7ED7, 0xC5EE6A, 0xF0AFDE};
  }
}
const lv_font_t *font(int n) {
  if (n >= 36)
    return &meshcator_font_36;
  if (n >= 28)
    return &meshcator_font_28;
  if (n >= 24)
    return &meshcator_font_24;
  if (n >= 18)
    return &meshcator_font_18;
  if (n >= 16)
    return &meshcator_font_16;
  if (n >= 14)
    return &meshcator_font_14;
  if (n >= 12)
    return &meshcator_font_12;
  return &meshcator_font_10;
}
std::string number(double n, int digits = 0) {
  char b[40];
  std::snprintf(b, sizeof b, "%.*f", digits, n);
  return b;
}
constexpr uint32_t ink = 0x161616;
} // namespace
void View::panel(int x, int y, int w, int h, uint32_t color, int radius) {
  auto o = lv_obj_create(root_);
  lv_obj_remove_style_all(o);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_set_style_bg_color(o, lv_color_hex(color), 0);
  lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(o, radius, 0);
  lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
}
void View::label(int x, int y, int w, const std::string &s, int z, uint32_t color) {
  auto o = lv_label_create(root_);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_width(o, w);
  lv_label_set_long_mode(o, LV_LABEL_LONG_WRAP);
  lv_label_set_text(o, s.c_str());
  lv_obj_set_style_text_font(o, font(z), 0);
  lv_obj_set_style_text_color(o, lv_color_hex(color), 0);
}
void View::segment(int x, int y, int a, int b, uint32_t color, int width) {
  auto o = lv_line_create(root_);
  lv_point_precise_t pts[2] = {
      {static_cast<lv_value_precise_t>(x), static_cast<lv_value_precise_t>(y)},
      {static_cast<lv_value_precise_t>(a), static_cast<lv_value_precise_t>(b)}};
  // LVGL 9.2 line points are borrowed: store a private copy until object deletion.
  auto owned = static_cast<lv_point_precise_t *>(lv_malloc(sizeof pts));
  std::copy(pts, pts + 2, owned);
  lv_line_set_points(o, owned, 2);
  lv_obj_add_event_cb(
      o, [](lv_event_t *e) { lv_free(lv_event_get_user_data(e)); }, LV_EVENT_DELETE, owned);
  lv_obj_set_style_line_color(o, lv_color_hex(color), 0);
  lv_obj_set_style_line_width(o, width, 0);
}
void View::splash() {
  auto tr = [&](const std::string &key) { return translate(app_.settings.language, key); };
  if (!root_) root_ = lv_screen_active();
  lv_obj_clean(root_);
  auto p = palette(app_.settings.theme);
  lv_obj_set_style_bg_color(root_, lv_color_hex(p.bg), 0);
  lv_obj_remove_flag(root_, LV_OBJ_FLAG_SCROLLABLE);
  panel(28, 57, 108, 108, p.a, 28);
  segment(50, 137, 50, 87, ink, 8);
  segment(50, 87, 82, 119, ink, 8);
  segment(82, 119, 114, 87, ink, 8);
  segment(114, 87, 114, 137, ink, 8);
  for (auto x : {42, 106}) {
    panel(x, 79, 16, 16, ink, 8);
    panel(x, 129, 16, 16, ink, 8);
  }
  panel(72, 109, 20, 20, p.b, 10);
  label(160, 69, 310, "MeshCator", 36, p.fg);
  label(163, 121, 300, tr("DEIN NETZ. OHNE UMWEGE."), 12, p.fg);
  panel(163, 150, 50, 6, p.a, 3);
  panel(221, 150, 28, 6, p.b, 3);
  panel(257, 150, 16, 6, p.c, 3);
  label(28, 192, 420, tr("SYSTEMSTART"), 14, p.fg);
}
void View::render() {
  auto tr = [&](const std::string &key) { return translate(app_.settings.language, key); };
  if (!app_.dirty)
    return;
  app_.dirty = false;
  if (!root_)
    root_ = lv_screen_active();
  lv_obj_clean(root_);
  auto p = palette(app_.settings.theme);
  lv_obj_set_style_bg_color(root_, lv_color_hex(p.bg), 0);
  lv_obj_remove_flag(root_, LV_OBJ_FLAG_SCROLLABLE);
  if (app_.helpOpen) {
    label(12, 5, 360, tr("Kurze Einführung"), 14, p.fg);
    label(420, 5, 55, std::to_string(app_.helpPage + 1) + "/4", 14, p.fg);
    const char *titles[] = {"1. Drehen", "2. Klicken / Enter", "3. Rad halten", "4. Zurück / Löschen"};
    const char *bodies[] = {
      "Rad: auswählen.\nIm Chat: Verlauf. Auf der Karte: Zoom.",
      "Klick: bestätigen oder Menü öffnen.\nEnter: bestätigen; im Chat senden.",
      "Rad halten: Hauptmenü.\nIn jeder Ansicht dieselbe Funktion.",
      "Kurz: zurück; im Chat: löschen.\nHalten >2 s: Start. Entwurf bleibt."};
    panel(12, 32, 456, 157, app_.helpPage % 2 ? p.b : p.a, 22);
    label(26, 48, 425, tr(titles[app_.helpPage]), 24, ink);
    label(26, 93, 425, tr(bodies[app_.helpPage]), 18, ink);
    for (size_t i = 0; i < 4; ++i) panel(26 + int(i)*24, 170, 16, 5, i == app_.helpPage ? ink : p.bg, 2);
    panel(12, 198, 204, 24, p.a, 12);
    label(22, 201, 184, tr("Klick: ") + tr(app_.helpPage == 3 ? "Los geht's" : "Weiter"), 14, ink);
    label(228, 201, 246, app_.notice.empty() ? tr("Zurück") : app_.notice, 14, p.fg);
    return;
  }
  if (app_.screen == Screen::Map && app_.mapFullscreen && !app_.menuOpen && !app_.mapMenuOpen && !app_.waypointNaming && !app_.waypointList) {
    drawMap(0, 0, 480, 222, p.b, ink, p.a);
    panel(10, 8, 460, 25, p.bg, 12);
    label(22, 13, 436, app_.mapCaption + " / Z" + std::to_string(app_.mapZoom), 12, p.fg);
    panel(10, 190, 460, 25, p.bg, 12);
    label(22, 195, 436, tr("Zurück: Vollbild verlassen"), 14, p.fg);
    return;
  }
  const std::string names[] = {tr("CHATS"),  tr("KONTAKTE"),      tr("RADAR"),     tr("KARTE"),
                                tr("SIGNAL"), tr("EINSTELLUNGEN"), tr("NACHRICHT"), tr("GESPERRT"), "MESHCATOR"};
  label(12, 3, 230,
        std::string(names[static_cast<int>(app_.screen)]) +
            (app_.status.simulation ? " / DEMO" : ""),
        14, p.fg);
  time_t epoch = app_.status.epoch;
  char clock[12] = "--:--";
  if (epoch > 1700000000) {
    auto tm = std::gmtime(&epoch);
    if (tm)
      std::strftime(clock, sizeof clock, "%H:%M", tm);
  }
  label(254, 3, 220,
        std::string(clock) + "  " + (app_.status.fix ? "GPS" : "--") + "  " +
            (app_.status.battery < 0 ? "--" : std::to_string(app_.status.battery)) + "%",
        16, p.fg);
  auto screen = app_.screen;
  if(app_.menuOpen||app_.mapMenuOpen||app_.contextOpen||app_.notificationMenu||app_.utility!=Utility::None||app_.waypointNaming||app_.waypointList){
    // Opaque overlays replace the content: do not draw expensive hidden views.
  } else if(screen==Screen::Dashboard){
    const char *titles[]={"Gespraeche","Kontakte","Radar","Karte","Signal","Einstellungen"};
    unsigned unread=0;for(auto &chat:app_.chats)unread+=chat.unread;
    const std::string values[]={std::to_string(unread)+tr(" NEU"),std::to_string(app_.nodes.size())+" nodes",app_.status.fix?"GPS":"GPS --",app_.status.fix?number(app_.status.position.lat,3):"Natural Earth",app_.noise.empty()?"--":std::to_string(app_.noise.back())+" dBm",app_.settings.setupComplete?tr("Bestätigt"):tr("Senden gesperrt")};
    for(size_t i=0;i<6;++i){
      int x=12+int(i%3)*156,y=29+int(i/3)*84;auto color=i==app_.selected?p.a:p.b;
      panel(x,y,148,78,color,18);
      label(x+10,y+11,128,std::string(i==app_.selected?"> ":"")+tr(titles[i]),16,ink);
      label(x+10,y+44,128,values[i],14,ink);
    }
  } else if (screen == Screen::Chats) {
    label(12, 27, 300, tr("Gespraeche"), 24, p.fg);
    auto visible=app_.visibleChats();
    const size_t begin = (app_.selected / 3) * 3;
    for (size_t j = begin; j < std::min(begin + 3, visible.size()+1); ++j) {
      if(j==visible.size()){int x=12+int(j-begin)*156;panel(x,60,148,137,j==app_.selected?0xFFAA5C:0x9BB9E7);label(x+10,73,127,tr("Verwalten"),18,ink);label(x+10,112,127,tr("Filter / Neu / Import"),14,ink);continue;}
      auto &c = app_.chats[visible[j]];
      int x = 12 + static_cast<int>(j - begin) * 156;
      auto col = j == app_.selected ? 0xFFAA5Cu : 0x9BB9E7u;
      panel(x, 60, 148, 137, col);
      label(x + 10, 70, 127,
            c.unread    ? std::to_string(c.unread) + tr(" NEU")
            : c.channel ? tr("KANAL")
                        : tr("DIREKT"),
            12, ink);
      label(x + 10, 90, 127, (app_.notificationMode(c.id)==0?"[M] ":"")+c.name, 18, ink);
      std::string preview = tr("Noch keine Nachricht");
      for (auto m = app_.messages.rbegin(); m != app_.messages.rend(); ++m)
        if (m->chat == c.id) {
          preview = m->text;
          break;
        }
      label(x + 10, 120, 127, preview.substr(0, 38), 12, ink);
      if (j == app_.selected) {
        panel(x + 77, 174, 61, 18, ink, 9);
        label(x + 84, 177, 52, tr("FOKUS"), 10, col);
      }
    }
  } else if (screen == Screen::Conversation) {
    auto c = std::find_if(app_.chats.begin(), app_.chats.end(),
                          [&](const Chat &v) { return v.id == app_.activeChat; });
    label(12, 26, 455, c == app_.chats.end() ? tr("Chat") : c->name, 24, p.fg);
    std::vector<const Message *> ms;
    for (auto &m : app_.messages)
      if (m.chat == app_.activeChat)
        ms.push_back(&m);
    if (!ms.empty()) {
      size_t offset = std::min(app_.historyOffset, ms.size() - 1);
      auto &m = *ms[ms.size() - 1 - offset];
      panel(12, 59, 456, 98, m.own ? p.b : p.a, 16);
      label(24, 66, 190,
            (m.own ? tr("DU / ") : tr("EMPFANGEN / ")) + std::to_string(ms.size() - offset) + tr(" von ") +
                std::to_string(ms.size()),
            12, ink);
      auto stamp=lv_label_create(root_);lv_label_set_text(stamp,messageTime(m.timestamp).c_str());
      lv_obj_set_pos(stamp,220,64);lv_obj_set_size(stamp,236,18);
      lv_label_set_long_mode(stamp,LV_LABEL_LONG_CLIP);
      lv_obj_set_style_text_font(stamp,font(14),0);lv_obj_set_style_text_color(stamp,lv_color_hex(ink),0);
      lv_obj_set_style_text_align(stamp,LV_TEXT_ALIGN_RIGHT,0);
      label(24, 83, 431, m.text, 14, ink);
      label(24, 141, 431,
            m.delivery == Delivery::Delivered ? tr("ZUGESTELLT")
            : m.delivery == Delivery::Unconfirmed ? tr("NICHT BESTÄTIGT / ERNEUT SENDEN")
            : m.delivery == Delivery::Failed      ? tr("FEHLER / ENTWURF ERHALTEN")
            : m.delivery == Delivery::Queued    ? tr("IN WARTESCHLANGE / KEIN ACK")
            : m.delivery == Delivery::Simulated ? tr("SIMULIERT")
                                                : tr("Rad: Verlauf"),
            12, ink);
    }
    panel(12, 165, 456, 35, p.c, 10);
    label(22, 172, 438, app_.draft.empty() ? tr("Tippen ... (Enter sendet)") : app_.draft, 14, ink);
  } else if (screen == Screen::Contacts || screen == Screen::Radar) {
    bool radar = screen == Screen::Radar;
    int startX = radar ? 230 : 12, width = radar ? 238 : 456;
    if (radar) {
      panel(12, 29, 206, 169, p.c, 20);
      for (int r : {23, 46, 66}) {
        auto o = lv_obj_create(root_);
        lv_obj_remove_style_all(o);
        lv_obj_set_pos(o, 115 - r, 113 - r);
        lv_obj_set_size(o, r * 2, r * 2);
        lv_obj_set_style_radius(o, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_color(o, lv_color_hex(ink), 0);
        lv_obj_set_style_border_width(o, 1, 0);
        lv_obj_set_style_border_opa(o, 70, 0);
      }
      segment(49, 113, 181, 113, ink);
      segment(115, 47, 115, 179, ink);
      label(110, 33, 20, "N", 12, ink);
      label(90, 181, 120, "Zoom " + std::to_string(app_.mapZoom), 12, ink);
      if (!app_.status.position.valid)
        label(34, 85, 165, tr("Warte auf GPS"), 14, ink);
      else
        for (size_t i = 0; i < app_.nodes.size(); ++i) {
          auto &n = app_.nodes[i];
          if (!n.position.valid)
            continue;
          auto d = locate(app_.status.position, n.position);
          if (d.meters > app_.radius)
            continue;
          double rad = d.degrees * 3.14159265 / 180;
          int x = 115 + static_cast<int>(std::sin(rad) * d.meters / app_.radius * 65),
              y = 113 - static_cast<int>(std::cos(rad) * d.meters / app_.radius * 65);
          panel(x - 3, y - 3, 6, 6, ink, 3);
        }
    }
    if (app_.nodes.empty())
      label(startX, 55, width, tr("Keine Nodes gehoert"), 16, p.fg);
    size_t begin = app_.selected / 4 * 4;
    for (size_t i = begin; i < std::min(begin + 4, app_.nodes.size()); ++i) {
      auto &n = app_.nodes[i];
      int y = 30 + static_cast<int>(i - begin) * 42;
      panel(startX, y, width, 37, i == app_.selected ? p.a : p.b, 12);
      label(startX + 10, y + 3, width - 20, std::string(n.favorite ? "* " : "") + n.name, 14, ink);
      label(startX + 10, y + 21, width - 20,
            (n.hasSnr ? number(n.snr, 1) + " dB" : "SNR --") + " / " +
                (n.position.valid ? "GPS" : tr("ohne Position")),
            12, ink);
    }
  } else if (screen == Screen::Signal) {
    panel(12, 29, 136, 170, p.a, 20);
    label(24, 43, 115, tr("NOISE / dBm"), 12, ink);
    label(20, 68, 125, app_.noise.empty() ? "--" : std::to_string(app_.noise.back()), 36, ink);
    if (!app_.noise.empty()) {
      auto mm = std::minmax_element(app_.noise.begin(), app_.noise.end());
      label(24, 124, 115,
            "MIN " + std::to_string(*mm.first) + "\nMAX " + std::to_string(*mm.second), 12, ink);
    }
    label(24, 177, 115, app_.paused ? tr("PAUSIERT") : tr("LIVE / 1 Hz"), 12, ink);
    panel(158, 29, 310, 170, p.b, 20);
    for (int y : {70, 118, 166})
      segment(206, y, 454, y, ink);
    label(169, 65, 40, "-90", 12, ink);
    label(169, 113, 40, "-105", 12, ink);
    label(169, 161, 40, "-120", 12, ink);
    for (size_t i = 1; i < app_.noise.size(); ++i) {
      auto yy = [&](int n) { return std::clamp(166 - (n + 120) * 96 / 30, 65, 172); };
      segment(206 + static_cast<int>(i - 1) * 248 / 59, yy(app_.noise[i - 1]),
              206 + static_cast<int>(i) * 248 / 59, yy(app_.noise[i]), ink, 2);
    }
    label(207, 182, 245, tr("-60 s                         jetzt"), 12, ink);
  } else if (screen == Screen::Map && !app_.menuOpen && !app_.mapMenuOpen && !app_.waypointNaming && !app_.waypointList && app_.utility==Utility::None) {
    drawMap(12, 29, 294, 170, p.b, ink, p.a);
    panel(316, 29, 152, 170, p.a, 20);
    label(328, 44, 128, tr("ANSICHT"), 12, ink);
    label(328, 68, 128, "Zoom " + std::to_string(app_.mapZoom), 24, ink);
    label(328, 113, 128,
          app_.status.position.valid
              ? number(app_.status.position.lat, 5) + "\n" + number(app_.status.position.lon, 5)
              : "GPS --",
          12, ink);
    label(328, 167, 128, app_.mapCaption, 14, ink);
    if(app_.navigationTarget>=0 && size_t(app_.navigationTarget)<app_.settings.waypoints.size()){
      auto &target=app_.settings.waypoints[app_.navigationTarget];
      panel(18,140,282,54,p.a,12);
      label(26,145,265,target.name,16,ink);
      auto bearing=locate(app_.status.position,target.position);
      label(26,168,265,app_.status.position.valid?number(bearing.meters/1000,2)+" km / "+number(bearing.degrees,0)+" deg":tr("Warte auf GPS-Fix"),16,ink);
    }
  } else if (screen == Screen::Settings) {
    if (app_.settingsEditing) {
      label(12, 29, 456, tr(settingLabel(app_.editField)), 24, p.fg);
      auto options = settingOptions(app_.editField);
      if(app_.editField==SettingField::Setup){
        panel(12,65,456,125,p.a,18);
        label(26,75,425,settingValue(app_.settings,SettingField::Frequency)+" MHz / "+settingValue(app_.settings,SettingField::Bandwidth)+" kHz",18,ink);
        label(26,104,425,"SF "+std::to_string(app_.settings.sf)+" / CR "+std::to_string(app_.settings.cr)+" / "+std::to_string(app_.settings.power)+" dBm",18,ink);
        label(26,142,425,tr("Funkkonfiguration bestätigen"),18,ink);
      } else if (options.empty()) {
        panel(12, 70, 456, 74, p.a, 18);
        label(26, 94, 425, app_.editValue + "_", 24, ink);
        label(12, 158, 456, app_.editField == SettingField::Setup ? tr("Funkkonfiguration bestätigen") : tr("Tippen: ersetzen / Del: löschen"), 16, p.fg);
      } else {
        int begin = app_.settingChoice / 3 * 3;
        for(int i=begin;i<std::min(begin+3,int(options.size()));++i){
          int y=64+(i-begin)*43;
          panel(12,y,456,39,i==app_.settingChoice?p.a:p.b,14);
          label(26,y+9,425,std::string(i==app_.settingChoice?"> ":"")+tr(options[i]),18,ink);
        }
      }
    } else {
      const char *categories[]={"Sprache","Farbe","Funk / Region","Gerät / GPS","Bluetooth","Display","Karte","Kanäle / Filter","Benachrichtigungen","Einrichtung","Version","Hilfe"};
      auto fields=settingFields(app_.settingsCategory);
      size_t count=app_.settingsCategory<0?12:app_.settingsCategory==7?2:fields.size();
      size_t begin=app_.selected/4*4;
      for(size_t i=begin;i<std::min(begin+4,count);++i){
        int y=29+int(i-begin)*42;
        panel(12,y,456,38,i==app_.selected?p.a:p.b,14);
        std::string text=app_.settingsCategory==7?tr(i==0?"Gesprächsfilter":"Kanal erstellen"):app_.settingsCategory<0?tr(categories[i]):tr(settingLabel(fields[i]))+": "+tr(settingValue(app_.settings,fields[i]));
        label(26,y+9,425,std::string(i==app_.selected?"> ":"")+text,18,ink);
      }
    }
  } else if (screen == Screen::Lock) {
    label(20, 45, 450, clock, 36, p.fg);
    panel(12, 112, 456, 85, p.a, 20);
    label(26, 129, 425, tr("GESPERRT"), 18, ink);
    label(26, 165, 425, tr("Enter zum Entsperren"), 14, ink);
  }
  if(app_.channelCreating){
    panel(0,24,480,173,p.bg,0);
    label(12,29,456,tr(app_.channelEditing.empty()?"Kanal erstellen":"Kanal bearbeiten")+" / "+std::to_string(app_.channelStage+1)+"/4",24,p.fg);
    panel(12,64,456,128,p.a,18);
    if(app_.channelStage==0){
      label(26,83,425,std::string(app_.channelKind==0?"> ":"  ")+tr("Öffentlicher Hashtag"),18,ink);
      label(26,131,425,std::string(app_.channelKind==1?"> ":"  ")+tr("Privater Schlüssel"),18,ink);
    }else if(app_.channelStage==1){label(26,77,425,tr("Name"),16,ink);label(26,112,425,app_.channelName+"_",24,ink);}
    else if(app_.channelStage==2){label(26,75,425,tr("128/256 Bit: Hex oder Base64"),16,ink);label(26,104,425,app_.channelSecret.substr(0,16)+"\n"+app_.channelSecret.substr(std::min(size_t(16),app_.channelSecret.size())),18,ink);label(26,166,425,tr("Tippen: ersetzen / Del: löschen"),14,ink);}
    else {label(26,82,425,app_.channelName,24,ink);label(26,125,425,tr(app_.channelKind?"Privater Schlüssel":"Öffentlicher Hashtag"),18,ink);label(26,161,425,tr("Klick: Speichern"),16,ink);}
  }
  if (app_.menuOpen) {
    panel(0, 24, 480, 178, p.bg, 0);
    const std::string menu[] = {tr("Gespraeche"), tr("Kontakte"), tr("Radar"), tr("Karte"), tr("Signal"), tr("Einstellungen")};
    for (size_t i = 0; i < 6; ++i) {
      int x = 12 + (i % 3) * 156, y = 32 + (i / 3) * 82;
      bool focus = i == app_.menuSelected;
      panel(x, y, 144, 72, focus ? p.a : p.b, 16);
      label(x + 10, y + 12, 124, focus ? tr("> AUSWAHL") : std::to_string(i + 1), 12, ink);
      label(x + 10, y + 35, 124, menu[i], 14, ink);
    }
  }
  if (app_.mapMenuOpen) {
    panel(12, 27, 456, 171, p.bg, 16);
    const char *items[]={app_.mapFullscreen?"Vollbild verlassen":"Vollbild","Eigene Position","Kontakte anzeigen","Standort merken","Gemerkte Orte","Navigation beenden","Schliessen"};
    size_t begin=app_.mapMenuSelected/4*4;
    for(size_t i=begin;i<std::min(begin+4,size_t(7));++i){
      int y=29+int(i-begin)*42;
      panel(12,y,456,38,app_.mapMenuSelected==i?p.a:p.b,14);
      label(26,y+9,425,std::string(app_.mapMenuSelected==i?"> ":"")+tr(items[i])+(i==2?": "+tr(app_.settings.showContacts?"Ein":"Aus"):""),18,ink);
    }
  }
  if(app_.waypointNaming||app_.waypointList){
    panel(12,27,456,171,p.bg,16);
    label(24,32,430,tr(app_.waypointNaming?"Standort merken":"Gemerkte Orte"),24,p.fg);
    if(app_.waypointNaming){panel(12,76,456,72,p.a,18);label(26,98,425,app_.waypointName+"_",24,ink);}
    else if(app_.settings.waypoints.empty())label(24,92,430,tr("Keine Orte gespeichert"),18,p.fg);
    else {
      size_t begin=app_.waypointSelected/3*3;
      for(size_t i=begin;i<std::min(begin+3,app_.settings.waypoints.size());++i){int y=65+int(i-begin)*43;panel(12,y,456,39,i==app_.waypointSelected?p.a:p.b,14);label(26,y+9,425,app_.settings.waypoints[i].name,18,ink);}
    }
  }
  if (app_.contextOpen) {
    auto node = std::find_if(app_.nodes.begin(), app_.nodes.end(), [&](const Node &n) { return n.id == app_.contextId; });
    auto chat = std::find_if(app_.chats.begin(), app_.chats.end(), [&](const Chat &c) { return c.id == app_.contextId; });
    std::string title = node != app_.nodes.end() ? node->name : chat != app_.chats.end() ? chat->name : app_.contextId;
    panel(0, 24, 480, 173, p.bg, 0);
    label(12, 29, 456, title, 18, p.fg);
    if (app_.contextDetails) {
      panel(12, 54, 456, 140, p.a, 16);
      if (app_.contextChannel) {
        label(24, 66, 430, tr("KANAL") + " / " + app_.contextId, 16, ink);
      } else {
        label(24, 64, 430, app_.contextId.substr(0,32) + "\n" + app_.contextId.substr(std::min<size_t>(32, app_.contextId.size())), 12, ink);
        if (node != app_.nodes.end()) {
          label(24, 106, 430, node->hops < 0 ? tr("Pfad unbekannt") : std::to_string(node->hops) + " hops", 16, ink);
          label(24, 133, 430, node->position.valid ? number(node->position.lat,5) + " / " + number(node->position.lon,5) : tr("ohne Position"), 14, ink);
          label(24, 160, 430, node->favorite ? tr("Favorit entfernen") : tr("Favorit setzen"), 12, ink);
        }
      }
    } else {
      auto actions = app_.contextActions();
      size_t begin = (app_.contextSelected / 4) * 4;
      for (size_t i = begin; i < std::min(begin + 4, actions.size()); ++i) {
        std::string text;
        switch (actions[i]) {
        case ContextAction::OpenChat: text = tr("Chat öffnen"); break;
        case ContextAction::Details: text = tr("Details"); break;
        case ContextAction::Favorite: text = tr(node != app_.nodes.end() && node->favorite ? "Favorit entfernen" : "Favorit setzen"); break;
        case ContextAction::ResetPath: text = tr("Pfad zurücksetzen"); break;
        case ContextAction::Latest: text = tr("Neueste Nachricht"); break;
        case ContextAction::Modes: text = tr("Hauptmenü"); break;
        case ContextAction::Notifications: text=tr("Benachrichtigungen"); break;
        case ContextAction::Retry: text=tr("Letzte unbestätigte erneut senden");break;
        case ContextAction::RetryFlood: text=tr("Erneut per Flood senden");break;
        case ContextAction::ClearHistory:text=tr("Verlauf löschen");break;
        case ContextAction::Emoji:text=tr("Emoji einfügen");break;
        case ContextAction::EditChannel:text=tr("Kanal bearbeiten");break;
        case ContextAction::DeleteEntry:text=tr("Eintrag löschen");break;
        case ContextAction::Share:text=tr("Kontakt senden");break;
        case ContextAction::Export:text=tr("Export / Teilen");break;
        case ContextAction::Import:text=tr("Importieren");break;
        case ContextAction::Rename:text=tr("Umbenennen");break;
        case ContextAction::QR:text=tr("Kontakt als QR-Code");break;
        case ContextAction::SetPath:text=tr("Pfad bearbeiten");break;
        case ContextAction::Ping:text="Ping (Zero Hop)";break;
        case ContextAction::Trace:text=tr("Pfad verfolgen");break;
        case ContextAction::Telemetry:text=tr("Telemetrie abfragen");break;
        case ContextAction::Permissions:text=tr("Telemetrie freigeben");break;
        case ContextAction::ShowMap:text=tr("Auf Karte ansehen");break;
        case ContextAction::Remote:text=tr("Remote-Administration");break;
        case ContextAction::Filter:text=tr("Gesprächsfilter");break;
        case ContextAction::NewChannel:text=tr("Kanal erstellen");break;
        case ContextAction::Close: text = tr("Schliessen"); break;
        }
        int y = 54 + int(i - begin) * 35;
        panel(12, y, 456, 32, i == app_.contextSelected ? p.a : p.b, 12);
        label(24, y + 7, 420, (i == app_.contextSelected ? "> " : "") + text, 16, ink);
      }
      label(410, 30, 62, std::to_string(app_.contextSelected+1) + "/" + std::to_string(actions.size()), 14, p.fg);
    }
  }
  if(app_.utility!=Utility::None){
    auto u=app_.utility;panel(0,24,480,173,p.bg,0);
    bool confirm=u==Utility::DeleteEntry||u==Utility::ClearHistory||u==Utility::TelemetryPolicy;
    std::string title=u==Utility::NodeDetails?tr("Details"):u==Utility::QR?tr("Kontakt als QR-Code"):u==Utility::TelemetryPolicy?tr("Regeln für alle Kontakte?"):u==Utility::SetPath?tr("Pfad: Hex / leer = direkt"):u==Utility::Permissions?tr("Telemetrie freigeben"):confirm?tr("Löschen bestätigen"):u==Utility::Emoji?tr("Emoji einfügen"):u==Utility::RemoteMenu?tr("Remote-Administration"):u==Utility::RemoteLogin?tr("Passwort"):u==Utility::RemoteCommand?tr("Remote-Befehl"):u==Utility::Rename?tr("Umbenennen"):u==Utility::ChatMenu?tr("Gespräche verwalten"):u==Utility::Import?tr("Importieren"):u==Utility::Export?tr("Export / Schlüssel sichtbar"):tr("Antwort");
    label(12,29,390,title,18,p.fg);
    std::vector<std::string> rows;
    if(confirm){rows={tr("Abbrechen"),tr(u==Utility::TelemetryPolicy?"Kontaktregeln aktivieren":"Löschen")};std::string target=app_.utilityTarget;for(auto&c:app_.chats)if(c.id==target)target=c.name;label(12,54,450,target,14,p.fg);}
    else if(u==Utility::Permissions){uint8_t bits=0;for(auto&n:app_.nodes)if(n.id==app_.utilityTarget)bits=n.permissions;const char* names[]={"Basis / Batterie","Position","Umweltsensoren"};for(int i=0;i<3;++i){int policy=(app_.telemetryPolicy()>>(i*2))&3;rows.push_back(std::string(bits&(1<<i)?"[+] ":"[-] ")+tr(names[i])+(policy==1?"":policy==2?" (ALL)":" (OFF)"));}rows.push_back(tr("Kontaktregeln aktivieren"));}
    else if(u==Utility::Emoji)for(int i=0;i<emojiCount;++i)rows.push_back(std::string(emojis[i])+" "+tr(emojiNames[i]));
    else if(u==Utility::RemoteMenu)rows={tr("Anmelden"),tr("Geräteinfo abfragen"),tr("Status abfragen"),tr("Remote-Befehl"),tr("Funkparameter lesen"),tr("Sendeleistung lesen"),tr("Advert-Intervall lesen"),tr("Weiterleitung lesen"),tr("Name remote ändern"),tr("Sendeleistung ändern"),tr("Advert-Intervall ändern"),tr("Weiterleitung ändern")};
    else if(u==Utility::ChatMenu)rows={tr("Alle"),tr("Kanäle"),tr("Direktnachrichten"),tr("Kanal erstellen"),tr("Importieren")};
    if(u==Utility::QR){panel(170,54,140,140,0xffffff,0);auto qr=lv_qrcode_create(root_);lv_qrcode_set_size(qr,124);lv_obj_set_pos(qr,178,62);lv_qrcode_set_dark_color(qr,lv_color_black());lv_qrcode_set_light_color(qr,lv_color_white());if(lv_qrcode_update(qr,app_.utilityText.data(),app_.utilityText.size())!=LV_RESULT_OK){lv_obj_delete(qr);label(12,65,456,tr("Aktion fehlgeschlagen"),16,p.fg);}}
    else if(!rows.empty()){size_t begin=app_.utilityChoice/3*3;for(size_t i=begin;i<std::min(begin+3,rows.size());++i){int yy=(confirm?80:58)+int(i-begin)*40;panel(12,yy,456,36,i==app_.utilityChoice?p.a:p.b,12);label(24,yy+7,430,(i==app_.utilityChoice?"> ":"")+rows[i],16,ink);}label(415,31,60,std::to_string(app_.utilityChoice+1)+"/"+std::to_string(rows.size()),12,p.fg);}
    else {
      bool input=u==Utility::SetPath||u==Utility::Import||u==Utility::RemoteCommand||u==Utility::Rename||u==Utility::RemoteLogin;
      std::string value=u==Utility::RemoteLogin?std::string(app_.utilityText.size(),'*'):app_.utilityText;
      auto pages=utilityPages(value);
      size_t page=input?pages.size()-1:std::min(app_.utilityChoice,pages.size()-1);
      label(415,31,60,std::to_string(page+1)+"/"+std::to_string(pages.size()),12,p.fg);
      label(12,65,456,pages[page]+(input?"_":""),16,p.fg);
    }
  }
  if(app_.notificationMenu){
    panel(0,24,480,173,p.bg,0);
    label(12,29,390,tr("Benachrichtigungen"),24,p.fg);
    label(414,34,60,std::to_string(app_.notificationChoice+1)+"/5",14,p.fg);
    const char *choices[]={"Wie global","Stumm","Ton","Vibration","Ton + Vibration"};
    int begin=app_.notificationChoice/3*3;
    for(int i=begin;i<std::min(begin+3,5);++i){int y=64+(i-begin)*43;panel(12,y,456,39,i==app_.notificationChoice?p.a:p.b,14);label(26,y+9,425,std::string(i==app_.notificationChoice?"> ":"")+tr(choices[i]),18,ink);}
  }
  std::string action = tr("Oeffnen");
  if(app_.utility!=Utility::None)action=tr("Auswaehlen");
  else if(app_.notificationMenu)action=tr("Speichern");
  else if (app_.contextOpen) action = app_.contextDetails ? tr("Zurück") : tr("Auswaehlen");
  else if (app_.menuOpen || app_.mapMenuOpen) action = tr("Auswaehlen");
  else if (screen == Screen::Conversation) action = tr("Senden");
  else if (screen == Screen::Signal) action = app_.paused ? tr("Fortsetzen") : tr("Pause");
  else if (app_.channelCreating) action = app_.channelStage==3?tr("Speichern"):tr("Weiter");
  else if (app_.waypointNaming) action = tr("Speichern");
  else if (screen == Screen::Settings) action = app_.settingsEditing ? tr("Speichern") : tr("Auswaehlen");
  else if (screen == Screen::Map) action = tr("Menue");
  else if (screen == Screen::Lock) action = tr("Entsperren");
  panel(0, 197, 480, 25, p.bg, 0);
  panel(12, 198, 204, 24, p.a, 12);
  label(22, 201, 184, (screen == Screen::Conversation && !app_.menuOpen && !app_.contextOpen && !app_.notificationMenu && app_.utility==Utility::None ? tr("Enter: ") : tr("Klick: ")) + action, 14, ink);
  label(228, 201, 246, app_.utility==Utility::Export||app_.utility==Utility::RemoteOutput||app_.utility==Utility::NodeDetails ? tr("Rad: Seiten / Back: Zurück") : app_.utility!=Utility::None ? tr("Zurück: Abbrechen") : app_.notificationMenu ? tr("Zurück: Abbrechen") : app_.contextOpen ? tr("Rad: Wahl / Back: Zurück") : app_.settingsEditing && !app_.menuOpen ? tr("Zurück: Abbrechen") : app_.menuOpen ? tr("Rad: Wahl / Back: Zurück") :
        screen == Screen::Conversation ? tr("Radklick: Chatmenü") :
        screen == Screen::Map ? tr("Rad: Zoom") :
        screen == Screen::Lock ? tr("Back halten: Start") : tr("Rad halten: Menü"), 14, p.fg);
  if (!app_.notice.empty()) {
    panel(12, 153, 456, 42, p.d, 10);
    label(22, 157, 436, app_.notice, 14, ink);
  }
}
} // namespace mc
