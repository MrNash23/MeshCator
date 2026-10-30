#pragma once
#include "AdminData.h"
#include "Projection.h"
namespace mc {
// Coarse overview containment, not a cadastral or political authority.
inline bool adminContains(const AdminArea &area,double lat,double lon){
 bool inside=false;
 for(unsigned r=area.first;r<unsigned(area.first)+area.count;++r){
  const auto &ring=adminRings[r];bool hit=false;
  for(unsigned i=0,j=ring.count-1;i<ring.count;j=i++){
   const auto &a=adminPoints[ring.first+i],&b=adminPoints[ring.first+j];
   double ax=a.lon*.01,bx=b.lon*.01,ay=a.lat*.01,by=b.lat*.01;
   // Dateline-spanning rings are represented on the source longitude side.
   if((ay>lat)!=(by>lat)&&lon<(bx-ax)*(lat-ay)/(by-ay)+ax)hit=!hit;
  }
  inside^=hit;
 }
 return inside;
}
inline const AdminArea *adminAt(double lat,double lon,int level){
 for(const auto &a:adminAreas)if(a.level==level&&adminContains(a,lat,lon))return &a;
 return nullptr;
}
}
