#include "App.h"
#include "maps/Projection.h"
namespace mc {
void App::prepareMap(){
 if(settings.mapSource==0||mapHomeLoaded)return;
 mapHomeLoaded=true;Position center;int zoom=2;
 if(backend_.mapHome(center,zoom)&&center.valid){
  offlineMapCenter=center;
  if(!mapCenter.valid&&mapZoom==2)mapZoom=zoom;
 }
}

bool App::panMap(char key){
 if(screen!=Screen::Map||menuOpen||helpOpen||contextOpen||mapMenuOpen||waypointNaming||waypointList||notificationMenu||utility!=Utility::None)return false;
 int dx=0,dy=0;
 switch(key){case 'a':case 'A':dx=-64;break;case 's':case 'S':dx=64;break;case 'w':case 'W':dy=-64;break;case 'z':case 'Z':dy=64;break;default:return false;}
 prepareMap();
 Position center=mapCenter.valid?mapCenter:status.position.valid?status.position:settings.mapSource!=0&&offlineMapCenter.valid?offlineMapCenter:Position{20,0,true};
 double world=256.*std::pow(2.,mapZoom),x=mapX(center.lon,world)+dx,y=std::clamp(mapY(center.lat,world)+dy,0.,world);
 x-=std::floor(x/world)*world;
 mapCenter={std::atan(std::sinh(3.141592653589793*(1.-2.*y/world)))*180./3.141592653589793,x/world*360.-180.,true};
 changed();return true;
}

bool App::handleMapKey(Key k){
 if(screen!=Screen::Map||menuOpen||helpOpen)return false;
 if(k==Key::Home||k==Key::Menu)return false;
 if(waypointNaming){
  if(k==Key::Delete)eraseUtf8(waypointName);
  else if(k==Key::Back)waypointNaming=false;
  else if(k==Key::Confirm||k==Key::Enter){
   if(!validText(waypointName)||waypointName.empty()){notice=translate(settings.language,"Ungültiger Wert");}
   else {auto next=settings;next.waypoints.push_back({waypointName,waypointPosition});
    if(backend_.saveSettings(next)){settings=next;waypointNaming=false;notice.clear();}
    else notice=translate(settings.language,"Einstellungen nicht gespeichert");}
  }
 }else if(waypointList){
  auto count=settings.waypoints.size();
  if((k==Key::Next||k==Key::Previous)&&count)waypointSelected=(waypointSelected+count+(k==Key::Next?1:-1))%count;
  else if(k==Key::Back||k==Key::Delete)waypointList=false;
  else if((k==Key::Confirm||k==Key::Enter)&&waypointSelected<count){navigationTarget=int(waypointSelected);mapCenter=settings.waypoints[waypointSelected].position;waypointList=false;}
 }else if(mapMenuOpen){
  if(k==Key::Next||k==Key::Previous)mapMenuSelected=(mapMenuSelected+7+(k==Key::Next?1:-1))%7;
  else if(k==Key::Back||k==Key::Delete)mapMenuOpen=false;
  else if(k==Key::Confirm||k==Key::Enter){
   mapMenuOpen=false;notice.clear();
   switch(mapMenuSelected){
    case 0:mapFullscreen=!mapFullscreen;break;
    case 1:if(status.position.valid)mapCenter={};else notice=translate(settings.language,"Warte auf GPS-Fix");break;
    case 2:{auto next=settings;next.showContacts=!next.showContacts;if(backend_.saveSettings(next))settings=next;else notice=translate(settings.language,"Einstellungen nicht gespeichert");break;}
    case 3:if(!status.position.valid)notice=translate(settings.language,"Warte auf GPS-Fix");else if(settings.waypoints.size()>=16)notice=translate(settings.language,"Maximal 16 Orte");else{waypointNaming=true;waypointName.clear();waypointPosition=status.position;}break;
    case 4:waypointList=true;waypointSelected=0;break;
    case 5:navigationTarget=-1;mapCenter={};break;
    default:break;
   }
  }
 }else return false;
 changed();return true;
}
}
