#include <algorithm>

#include "core/App.h"
#include "core/BackGesture.h"
#include "mesh/SimBackend.h"
#include "ui/View.h"
#include <SDL.h>
#include <chrono>
#include <cstring>
#include <ctime>
#include <iostream>
#include <vector>
namespace {
std::vector<uint16_t> pixels(480 * 222);
void flush(lv_display_t *d, const lv_area_t *a, uint8_t *bytes) {
  auto src = reinterpret_cast<uint16_t *>(bytes);
  for (int y = a->y1; y <= a->y2; ++y) {
    std::memcpy(&pixels[y * 480 + a->x1], src, (a->x2 - a->x1 + 1) * 2);
    src += a->x2 - a->x1 + 1;
  }
  lv_display_flush_ready(d);
}
} // namespace
int main(int argc, char **argv) {
  bool smoke = false, menu = false, splash = false, mapPerf=false;
  int mapZoom = -1;
  std::string utilityPreview;
  std::string screenshot, settings = "meshcator-settings.txt";
  int view = 8, language = -1, helpPage = 0, category = -1, channelStep = -1, setupStep = -1;
  bool notificationPreview=false;
  bool editSettings = false, fullscreenMap = false, context = false, help = false;
  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    if(a=="--map-perf")mapPerf=true;
    else if(a=="--utility"&&i+1<argc)utilityPreview=argv[++i];
    else if(a=="--notifications") notificationPreview=true;
    else if (a == "--settings-category" && i + 1 < argc) category=std::atoi(argv[++i]);
    else if (a == "--channel-step" && i + 1 < argc) channelStep=std::clamp(std::atoi(argv[++i]),0,3);
    else if (a == "--setup-step" && i + 1 < argc) setupStep=std::clamp(std::atoi(argv[++i]),0,5);
    else if (a == "--help-page" && i + 1 < argc)
      helpPage = std::clamp(std::atoi(argv[++i]), 0, 3);
    else if (a == "--help-screen")
      help = true;
    else if (a == "--context")
      context = true;
    else if (a == "--map-zoom" && i + 1 < argc)
      mapZoom = std::clamp(std::atoi(argv[++i]), 0, 18);
    else if (a == "--map-fullscreen")
      fullscreenMap = true;
    else if (a == "--language" && i + 1 < argc)
      language = std::atoi(argv[++i]);
    else if (a == "--edit-settings")
      editSettings = true;
    else if (a == "--splash")
      splash = true;
    else if (a == "--menu")
      menu = true;
    else if (a == "--smoke")
      smoke = true;
    else if (a == "--screenshot" && i + 1 < argc)
      screenshot = argv[++i];
    else if (a == "--settings" && i + 1 < argc)
      settings = argv[++i];
    else if (a == "--screen" && i + 1 < argc)
      view = std::atoi(argv[++i]);
  }
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    std::cerr << SDL_GetError();
    return 1;
  }
  auto window = SDL_CreateWindow(
      "MeshCator / LVGL / DEMO", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 444,
      smoke ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (!window) {
    std::cerr << SDL_GetError();
    return 1;
  }
  auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  if (!renderer) {
    std::cerr << SDL_GetError();
    return 1;
  }
  SDL_RenderSetLogicalSize(renderer, 480, 222);
  auto texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, 480, 222);
  lv_init();
  auto display = lv_display_create(480, 222);
  lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
  std::vector<uint8_t> buffer(480 * 40 * 2);
  lv_display_set_buffers(display, buffer.data(), nullptr, buffer.size(),
                         LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(display, flush);
  mc::SimBackend backend(settings);
  mc::App app(backend);
  backend.seed(app);
  if (language >= 0 && language < 4) app.settings.language = static_cast<mc::Language>(language);
  app.screen = static_cast<mc::Screen>(std::clamp(view, 0, 8));
  if (app.screen == mc::Screen::Conversation) app.open(0);
  if (mapZoom >= 0) app.mapZoom = mapZoom;
  if (fullscreenMap) { app.screen = mc::Screen::Map; app.mapFullscreen = true; }
  if(category>=0&&category<=8){app.screen=mc::Screen::Settings;app.settingsCategory=category;app.selected=0;}
  if(channelStep>=0){app.screen=mc::Screen::Settings;app.beginChannel();app.channelStage=channelStep;app.channelKind=1;app.channelName="Field team";app.channelSecret="0123456789abcdef0123456789abcdef";}
  if(setupStep>=0){app.beginSetup();for(int i=0;i<setupStep;++i)app.key(mc::Key::Confirm);}
  if(notificationPreview){app.open(0);app.notificationMenu=true;app.notificationTarget=app.activeChat;app.notificationChoice=0;}
  if(utilityPreview=="node"||utilityPreview=="path"||utilityPreview=="permissions"||utilityPreview=="qr"||utilityPreview=="node-menu"||utilityPreview=="node-remote"){
    app.screen=mc::Screen::Contacts;app.contextId=app.nodes[0].id;app.contextChannel=false;
    if(utilityPreview=="qr"){app.nodes[0].id=std::string(64,'a');app.contextId=app.nodes[0].id;}
    if(utilityPreview=="node-menu")app.showContext();else app.action(utilityPreview=="node"?mc::ContextAction::Details:utilityPreview=="path"?mc::ContextAction::SetPath:utilityPreview=="permissions"?mc::ContextAction::Permissions:utilityPreview=="qr"?mc::ContextAction::QR:mc::ContextAction::Remote);
    utilityPreview.clear();
  }
  if(!utilityPreview.empty()){app.open(0);app.contextId=app.activeChat;app.contextChannel=true;if(utilityPreview=="emoji")app.action(mc::ContextAction::Emoji);else if(utilityPreview=="delete")app.action(mc::ContextAction::DeleteEntry);else if(utilityPreview=="remote")app.action(mc::ContextAction::Remote);else if(utilityPreview=="manage")app.utilityOpen(mc::Utility::ChatMenu);else if(utilityPreview=="chat"){app.receive(app.activeChat,"Demo","🙂 ❤️ 👍 ✅",true);app.draft="Danke! 🙏";}}
  if (context) app.key(mc::Key::Context);
  if (menu) app.key(mc::Key::Menu);
  if (editSettings && app.screen == mc::Screen::Settings) app.key(mc::Key::Confirm);
  if (help) { app.openHelp(); app.helpPage = helpPage; }
  else if (!smoke) app.startIntroduction();
  mc::View ui(app);
  if(mapPerf){
    auto check=[](bool ok,const char*msg){if(!ok){std::cerr<<msg<<"\n";std::exit(2);}};
    app.screen=mc::Screen::Map;app.mapZoom=6;app.changed();auto start=std::chrono::steady_clock::now();ui.render();auto first=std::chrono::steady_clock::now();
    unsigned builds=ui.mapBuilds();
    for(int i=0;i<100;++i){app.changed();ui.render();}check(ui.mapBuilds()==builds,"Unchanged map rebuilt");
    app.mapMenuOpen=true;for(int i=0;i<100;++i){app.key(mc::Key::Next);ui.render();}check(ui.mapBuilds()==builds,"Map menu rebuilt hidden map");
    app.mapMenuOpen=false;app.menuOpen=true;for(int i=0;i<100;++i){app.key(mc::Key::Next);ui.render();}check(ui.mapBuilds()==builds,"Global menu rebuilt hidden map");
    app.menuOpen=false;app.mapZoom++;app.changed();ui.render();check(ui.mapBuilds()==builds+1,"Zoom did not invalidate map");
    app.settings.showContacts=!app.settings.showContacts;app.changed();ui.render();check(ui.mapBuilds()==builds+2,"Contact visibility did not invalidate map");
    std::cout<<"First map ms: "<<std::chrono::duration<double,std::milli>(first-start).count()<<"; 300 redraws: no redundant map builds; zoom/contacts invalidate correctly\n";SDL_Quit();return 0;
  }
  SDL_StartTextInput();
  bool running = true;
  bool backDown = false;
  mc::BackGesture backGesture;
  auto processBack = [&]() {
    auto action = backGesture.update(backDown, SDL_GetTicks());
    if (action == mc::BackGesture::Action::Short) app.key(mc::Key::Delete);
    else if (action == mc::BackGesture::Action::Home) app.key(mc::Key::Home);
  };
  uint32_t last = SDL_GetTicks(), sample = last;
  int frames = 0;
  while (running) {
    app.tick(SDL_GetTicks());
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT)
        running = false;
      if (e.type == SDL_TEXTINPUT)
        app.type(e.text.text);
      if (e.type == SDL_MOUSEWHEEL)
        app.key(e.wheel.y > 0 ? mc::Key::Previous : mc::Key::Next);
      if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_BACKSPACE) { backDown = false; processBack(); }
      if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
        case SDLK_TAB:
          app.key(mc::Key::Menu);
          break;
        case SDLK_RIGHT:
        case SDLK_DOWN:
          app.key(mc::Key::Next);
          break;
        case SDLK_LEFT:
        case SDLK_UP:
          app.key(mc::Key::Previous);
          break;
        case SDLK_RETURN:
          app.key(mc::Key::Enter);
          break;
        case SDLK_ESCAPE:
          app.key(mc::Key::Back);
          break;
        case SDLK_BACKSPACE:
          backDown = true;
          processBack();
          break;
        case SDLK_F6:
          app.key(mc::Key::Context);
          break;
        case SDLK_F2:
          app.key(mc::Key::ThemeNext);
          break;
        case SDLK_F3:
          app.key(mc::Key::Advert);
          break;
        case SDLK_F4:
          app.key(mc::Key::Flood);
          break;
        case SDLK_F5:
          app.key(mc::Key::Lock);
          break;
        case SDLK_PLUS:
        case SDLK_EQUALS:
          if (app.screen != mc::Screen::Conversation)
            app.key(mc::Key::ZoomIn);
          break;
        case SDLK_MINUS:
          if (app.screen != mc::Screen::Conversation)
            app.key(mc::Key::ZoomOut);
          break;
        case SDLK_SPACE:
          if (app.screen == mc::Screen::Signal)
            app.key(mc::Key::Enter);
          else if (app.screen != mc::Screen::Conversation)
            app.key(mc::Key::Menu);
          break;
        default:
          break;
        }
      }
    }
    processBack();
    uint32_t now = SDL_GetTicks();
    lv_tick_inc(now - last);
    last = now;
    if (now - sample >= 1000) {
      sample = now;
      app.status.epoch = static_cast<uint32_t>(std::time(nullptr));
      app.sample(-116 + int(now / 1000 % 15));
      app.changed();
    }
    if (splash) ui.splash();
    else ui.render();
    lv_timer_handler();
    SDL_UpdateTexture(texture, nullptr, pixels.data(), 480 * 2);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
    SDL_Delay(5);
    if (smoke && ++frames == 20)
      running = false;
  }
  if (!screenshot.empty()) {
    auto surface = SDL_CreateRGBSurfaceWithFormatFrom(pixels.data(), 480, 222, 16, 960,
                                                      SDL_PIXELFORMAT_RGB565);
    if (!surface || SDL_SaveBMP(surface, screenshot.c_str())) {
      std::cerr << SDL_GetError();
      return 2;
    }
    SDL_FreeSurface(surface);
  }
  app.saveArchive();
  SDL_StopTextInput();
  lv_deinit();
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
