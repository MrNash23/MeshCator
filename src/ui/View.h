#pragma once
#include "core/App.h"
#include <lvgl.h>
namespace mc {
class View {
  App &app_;
  lv_obj_t *root_ = nullptr;
  std::vector<uint16_t> mapPixels_;
  lv_image_dsc_t mapImage_{};
  std::map<std::string, std::vector<uint16_t>> tiles_;
  uint32_t tileEpoch_ = 0;
  std::string mapKey_, cachedCaption_;
  struct MapLabel {int x,y,w;std::string text;};
  std::vector<MapLabel> mapLabels_;
  unsigned mapBuilds_=0;
  bool mapPending_=false;
  std::map<int,std::vector<uint8_t>> worldCache_;
  void drawMap(int x, int y, int w, int h, uint32_t background, uint32_t coast, uint32_t accent);
  void panel(int, int, int, int, uint32_t, int radius = 16);
  void label(int, int, int, const std::string &, int, uint32_t);
  void segment(int, int, int, int, uint32_t, int width = 1);

public:
  explicit View(App &a) : app_(a) {}
  void render();
  void splash();
  unsigned mapBuilds()const{return mapBuilds_;}
};
} // namespace mc
