#include "View.h"
#include "maps/Projection.h"
#include "maps/AdminMap.h"
#include "maps/WorldTileDecoder.h"
#include <algorithm>
#include <sstream>
LV_FONT_DECLARE(meshcator_font_14);
namespace mc {
void View::drawMap(int x,int y,int w,int h,uint32_t background,uint32_t coast,uint32_t accent){
 Position center=app_.mapCenter.valid?app_.mapCenter:app_.status.position.valid?app_.status.position:Position{20,0,true};
 double world=256.*std::pow(2.,app_.mapZoom),cx=mapX(center.lon,world),cy=mapY(center.lat,world);
 std::ostringstream key;key.precision(15);
 key<<w<<','<<h<<','<<app_.mapZoom<<','<<std::floor(cx)<<','<<std::floor(cy)<<','<<background<<','<<coast<<','<<accent<<','<<app_.settings.mapSource<<','<<int(app_.settings.language)<<','<<app_.settings.showContacts;
 auto markKey=[&](Position pos){key<<','<<pos.valid;if(pos.valid)key<<','<<std::lround(mapX(pos.lon,world))<<','<<std::lround(mapY(pos.lat,world));};
 markKey(app_.status.position);if(app_.settings.showContacts)for(const auto&n:app_.nodes)markKey(n.position);
 key<<','<<app_.navigationTarget;if(app_.navigationTarget>=0&&size_t(app_.navigationTarget)<app_.settings.waypoints.size())markKey(app_.settings.waypoints[app_.navigationTarget].position);
 // SD misses are retried every ten seconds, but never on each menu detent.
 if(app_.settings.mapSource!=0)key<<','<<lv_tick_get()/10000;
 auto present=[&](){auto image=lv_image_create(root_);lv_image_set_src(image,&mapImage_);lv_obj_set_pos(image,x,y);
  for(const auto&n:mapLabels_){panel(x+n.x,y+n.y,n.w,20,background,6);auto text=lv_label_create(root_);lv_label_set_text(text,n.text.c_str());lv_obj_set_style_text_font(text,&meshcator_font_14,0);lv_obj_set_style_text_color(text,lv_color_hex(coast),0);lv_label_set_long_mode(text,LV_LABEL_LONG_CLIP);lv_obj_set_size(text,n.w-12,18);lv_obj_set_pos(text,x+n.x+6,y+n.y+2);}
 };
 if(mapKey_==key.str()&&!mapPending_){app_.mapCaption=cachedCaption_;present();return;}
 mapKey_=key.str();mapPending_=false;mapLabels_.clear();++mapBuilds_;
 auto rgb=[](uint32_t c){return uint16_t(((c>>19)&31)<<11|((c>>10)&63)<<5|((c>>3)&31));};
 lv_image_cache_drop(&mapImage_);
 mapPixels_.assign(w*h,rgb(background));
 auto pixel=[&](int px,int py,uint16_t c){if(px>=0&&px<w&&py>=0&&py<h)mapPixels_[py*w+px]=c;};
 auto line=[&](double ax,double ay,double bx,double by,uint16_t c){
  if(!clipMapLine(ax,ay,bx,by,w-1,h-1))return;
  int a=std::lround(ax),b=std::lround(ay),endX=std::lround(bx),endY=std::lround(by);
  int dx=std::abs(endX-a),sx=a<endX?1:-1,dy=-std::abs(endY-b),sy=b<endY?1:-1,err=dx+dy;
  for(;;){pixel(a,b,c);if(a==endX&&b==endY)break;int e=2*err;if(e>=dy){err+=dy;a+=sx;}if(e<=dx){err+=dx;b+=sy;}}
 };

 int z=std::min(app_.mapZoom,4),scale=1<<(app_.mapZoom-z),tiles=1<<z;
 double baseWorld=256.*tiles;
 auto blend=[](uint32_t a,uint32_t b,int weight){uint32_t c=0;for(int shift:{0,8,16})c|=((((a>>shift)&255)*weight+((b>>shift)&255)*(100-weight))/100)<<shift;return c;};
 uint16_t ocean=rgb(blend(background,coast,35)),land=rgb(blend(background,0xffffff,78));
 std::fill(mapPixels_.begin(),mapPixels_.end(),ocean);
 // Decode a compact semantic atlas, then color it using the active theme.
 if(app_.settings.mapSource!=1){
 std::vector<int> columns(w);for(int px=0;px<w;++px)columns[px]=(int(std::floor((cx-w*.5+px)/scale))%int(baseWorld)+int(baseWorld))%int(baseWorld);
 int lastKey=-1;const std::vector<uint8_t>* current=nullptr;
 for(int py=0;py<h;++py){
  int wy=int(std::floor((cy-h*.5+py)/scale));if(wy<0||wy>=int(baseWorld))continue;
  for(int px=0;px<w;++px){int wx=columns[px],tx=wx/256,ty=wy/256,key=z*4096+ty*tiles+tx;
   if(key!=lastKey){auto found=worldCache_.find(key);if(found==worldCache_.end()){
    if(worldCache_.size()>=12)worldCache_.clear();std::vector<uint8_t> tile;
    if(loadWorldTile(z,tx,ty,tile))found=worldCache_.emplace(key,std::move(tile)).first;
   }current=found==worldCache_.end()?nullptr:&found->second;lastKey=key;}
   if(current&&worldLand(*current,wx%256,wy%256))mapPixels_[py*w+px]=land;
  }
 }
 }

 // Vector borders stay thin as the user zooms; regional borders are subdued.
 if(app_.settings.mapSource!=1){
  for(const auto &area:adminAreas){
   if(area.level&&app_.mapZoom<5)continue;
   uint16_t color=rgb(area.level?blend(coast,background,55):coast);
   for(unsigned r=area.first;r<unsigned(area.first)+area.count;++r){auto ring=adminRings[r];
    struct Bounds{int minX=18000,maxX=-18000,minY=9000,maxY=-9000;};
    static const auto bounds=[](){std::vector<Bounds> out;for(auto ring:adminRings){Bounds b;for(unsigned j=0;j<ring.count;++j){auto v=adminPoints[ring.first+j];b.minX=std::min(b.minX,int(v.lon));b.maxX=std::max(b.maxX,int(v.lon));b.minY=std::min(b.minY,int(v.lat));b.maxY=std::max(b.maxY,int(v.lat));}out.push_back(b);}return out;}();
    const auto &bb=bounds[r];
    double top=h*.5+mapY(bb.maxY*.01,world)-cy,bottom=h*.5+mapY(bb.minY*.01,world)-cy;
    if(bottom<0||top>h)continue;
    if(bb.maxX-bb.minX<18000){double mid=(bb.minX+bb.maxX)*.005,half=(bb.maxX-bb.minX)*.005/360.*world;double rx=w*.5+wrapMapX(mapX(mid,world)-cx,world);if(rx+half<0||rx-half>w)continue;}
    for(unsigned i=1;i<ring.count;++i){auto a=adminPoints[ring.first+i-1],b=adminPoints[ring.first+i];
     double ax=w*.5+wrapMapX(mapX(a.lon*.01,world)-cx,world),ay=h*.5+mapY(a.lat*.01,world)-cy;
     double bx=ax+wrapMapX(mapX(b.lon*.01,world)-mapX(a.lon*.01,world),world),by=h*.5+mapY(b.lat*.01,world)-cy;
     line(ax,ay,bx,by,color);
    }
   }
  }
 }
 int available=0,missing=0;
 if(lv_tick_get()/10000!=tileEpoch_){for(auto it=tiles_.begin();it!=tiles_.end();)if(it->second.empty())it=tiles_.erase(it);else ++it;tileEpoch_=lv_tick_get()/10000;}
 bool tileRead=false;
 if(app_.settings.mapSource!=0){
  if(app_.settings.mapSource==1)std::fill(mapPixels_.begin(),mapPixels_.end(),rgb(background));
  int minX=int(std::floor((cx-w*.5)/256)),maxX=int(std::floor((cx+w*.5-1)/256));
  int minY=int(std::floor((cy-h*.5)/256)),maxY=int(std::floor((cy+h*.5-1)/256));
  int count=1<<app_.mapZoom;
  for(int ty=minY;ty<=maxY;++ty)for(int tx=minX;tx<=maxX;++tx){
   if(ty<0||ty>=count)continue;
   int wrapped=(tx%count+count)%count;
   std::string key=std::to_string(app_.mapZoom)+"/"+std::to_string(wrapped)+"/"+std::to_string(ty);
   auto found=tiles_.find(key);
   if(found==tiles_.end()){
    if(tileRead){++missing;mapPending_=true;continue;}tileRead=true;
    if(tiles_.size()>=8)tiles_.erase(tiles_.begin());
    std::vector<uint16_t> pixels;if(!app_.readTile(app_.mapZoom,wrapped,ty,pixels))pixels.clear();
    found=tiles_.emplace(key,std::move(pixels)).first;
   }
   if(found->second.size()!=65536){++missing;continue;}++available;
   int left=std::lround(tx*256-cx+w*.5),top=std::lround(ty*256-cy+h*.5);
   for(int py=std::max(0,top);py<std::min(h,top+256);++py)
    for(int px=std::max(0,left);px<std::min(w,left+256);++px)mapPixels_[py*w+px]=found->second[(py-top)*256+px-left];
  }
 }
 app_.mapCaption=app_.settings.mapSource==0?"World":missing?(available?"SD / partial":app_.settings.mapSource==2?"World / SD --":"SD --"):"SD";
 const bool adminVisible=app_.settings.mapSource==0||(app_.settings.mapSource==2&&available==0);
 if(adminVisible&&app_.mapZoom>=6){
  const auto *country=adminAt(center.lat,center.lon,0),*region=adminAt(center.lat,center.lon,1);
  int lang=static_cast<int>(app_.settings.language);
  if(country)app_.mapCaption=std::string(region?region->names[lang]:country->names[lang])+(region?std::string(" / ")+country->names[lang]:"");
 }
 auto marker=[&](Position pos,bool own){
  if(!pos.valid)return;
  int px=std::lround(w*.5+wrapMapX(mapX(pos.lon,world)-cx,world)),py=std::lround(h*.5+mapY(pos.lat,world)-cy);
  for(int dy=-4;dy<=4;++dy)for(int dx=-4;dx<=4;++dx)if(dx*dx+dy*dy<=16)pixel(px+dx,py+dy,rgb(own?accent:coast));
  if(own){line(px-6,py,px+6,py,rgb(coast));line(px,py-6,px,py+6,rgb(coast));}
 };
 if(app_.settings.showContacts)for(auto &node:app_.nodes)marker(node.position,false);
 marker(app_.status.position,true);
 if(app_.navigationTarget>=0&&size_t(app_.navigationTarget)<app_.settings.waypoints.size())marker(app_.settings.waypoints[app_.navigationTarget].position,true);
 mapImage_={};mapImage_.header.magic=LV_IMAGE_HEADER_MAGIC;mapImage_.header.cf=LV_COLOR_FORMAT_RGB565;
 mapImage_.header.w=w;mapImage_.header.h=h;mapImage_.header.stride=w*2;
 mapImage_.data_size=mapPixels_.size()*2;mapImage_.data=reinterpret_cast<const uint8_t*>(mapPixels_.data());
 // Geographic names use the same type and rounded backing as the rest of the UI.
 if(adminVisible){
  struct Candidate {const AdminArea *area;double px,py,dist;};
  std::vector<Candidate> names;
  for(const auto &a:adminAreas){
   if((a.level&&app_.mapZoom<5)||(!a.level&&app_.mapZoom>=6))continue;
   double px=w*.5+wrapMapX(mapX(a.lon*.01,world)-cx,world),py=h*.5+mapY(a.lat*.01,world)-cy;
   if(px<0||px>=w||py<28||py>h-30)continue;
   names.push_back({&a,px,py,std::hypot(px-w*.5,py-h*.5)});
  }
  std::sort(names.begin(),names.end(),[](const Candidate&a,const Candidate&b){return a.dist<b.dist;});
  struct Box {int x,y,w;};std::vector<Box> boxes;
  for(auto &n:names){
   std::string name=n.area->names[static_cast<int>(app_.settings.language)];
   lv_point_t measured;lv_text_get_size(&measured,name.c_str(),&meshcator_font_14,0,0,2000,LV_TEXT_FLAG_NONE);
   int width=std::min(w-8,int(measured.x)+16),left=std::clamp(int(n.px)-width/2,4,w-width-4),top=int(n.py)-9;
   if(std::abs(n.px-w*.5)<18&&std::abs(n.py-h*.5)<18)top+=19;
   bool overlap=false;for(auto b:boxes)if(left<b.x+b.w+5&&left+width+5>b.x&&top<b.y+24&&top+24>b.y){overlap=true;break;}
   if(overlap||top>h-32||top<(app_.mapFullscreen?36:4))continue;
   mapLabels_.push_back({left,top,width,name});boxes.push_back({left,top,width});
   if(boxes.size()>=7)break;
  }
 }
 cachedCaption_=app_.mapCaption;present();
}
}
