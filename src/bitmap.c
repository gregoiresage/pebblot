#include <pebble.h>
#include "bitmap.h"

typedef enum {
  TOP_LEFT,
  LEFT,
  BOTTOM_LEFT,
  BOTTOM,
  BOTTOM_RIGHT,
  RIGHT,
  TOP_RIGHT,
  TOP
} DirectionId;

static GColor getPixelColorAt(GBitmap* bitmap, int x, int y, DirectionId direction){
  switch(direction){
    case TOP_LEFT : return getPixel(bitmap, x-1, y-1); break;
    case LEFT :     return getPixel(bitmap, x-1, y); break;
    case BOTTOM_LEFT : return getPixel(bitmap, x-1, y+1); break;
    case BOTTOM : return getPixel(bitmap, x, y+1);  break;
    case BOTTOM_RIGHT : return getPixel(bitmap, x+1, y+1);  break;
    case RIGHT : return getPixel(bitmap, x+1, y);  break;
    case TOP_RIGHT : return getPixel(bitmap, x+1, y-1);  break;
    case TOP : return getPixel(bitmap, x, y-1);  break;
  }
  return -1;
}

static void addPoint(GPathInfo *pathinfo, uint32_t* maxsize, GPoint point){
  if(pathinfo->num_points == (*maxsize)){
    GPoint *newpoints = malloc(((*maxsize) + 10) * sizeof(GPoint));
    memcpy(newpoints, pathinfo->points, (*maxsize) * sizeof(GPoint));
    free(pathinfo->points);
    pathinfo->points = newpoints;
    *maxsize += 10;
  }
  pathinfo->points[pathinfo->num_points].x = point.x;
  pathinfo->points[pathinfo->num_points].y = point.y;
  pathinfo->num_points++;
}

static uint32_t distance(GPoint p1, GPoint p2){
  return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}

MyGPath* gpath_create_from_bitmap(GBitmap* bitmap){
  int offset_x = bitmap->bounds.origin.x;
  int offset_y = bitmap->bounds.origin.y;
  int h = bitmap->bounds.size.h;
  int w = bitmap->bounds.size.w;

  int initX = 0;
  int initY = 0;
  bool startFound = false;

  for(int y=0; y<h && !startFound; y++){
    for(int x=0; x<w && !startFound; x++){
      if(getPixel(bitmap, x + offset_x, y + offset_y) == GColorBlack)
        startFound = true;
        initX = x + offset_x;
        initY = y + offset_y;
    }
  }

  if(!startFound){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "not start found");
    return NULL;
  }

  uint32_t maxsize = 100;
  GPathInfo pathinfo = {
    0,
    (GPoint*)malloc(maxsize *sizeof(GPoint))
  };

  DirectionId direction = BOTTOM_LEFT;
  GPoint p = (GPoint){initX, initY};
  addPoint(&pathinfo, &maxsize, p);

  DirectionId previousDirection = direction;

  GPoint corners[4];
  corners[0] =  (GPoint){bitmap->bounds.origin.x, bitmap->bounds.origin.y};
  corners[1] =  (GPoint){bitmap->bounds.origin.x, bitmap->bounds.origin.y + bitmap->bounds.size.h};
  corners[2] =  (GPoint){bitmap->bounds.origin.x + bitmap->bounds.size.w, bitmap->bounds.origin.y + bitmap->bounds.size.h};
  corners[3] =  (GPoint){bitmap->bounds.origin.x + bitmap->bounds.size.w, bitmap->bounds.origin.y};
  uint16_t corners_ids[4];
  uint32_t distances[4];
  memset(distances, 0xFF, 4 * sizeof(uint32_t));

  do {
    for(int i=0; i<8; i++){
      DirectionId searchdirection = (direction + i) % 8;
      if(getPixelColorAt(bitmap, p.x, p.y, searchdirection) == GColorBlack){
        if(searchdirection != previousDirection)
        {
          addPoint(&pathinfo, &maxsize, p);
          previousDirection = searchdirection;
          for(int i=0; i<4; i++){
            uint32_t dist = distance(corners[i], p);
            if(dist < distances[i]){
              distances[i] = dist;
              corners_ids[i] = pathinfo.num_points;
            }
          }
        }
        switch(searchdirection){
          case TOP_LEFT : 
            p.x--; p.y--;
            direction = TOP_RIGHT; break;
          case LEFT : 
            p.x--;
            direction = TOP_LEFT; break;
          case BOTTOM_LEFT : 
            p.x--; p.y++;
            direction = TOP_LEFT; break;
          case BOTTOM : 
            p.y++;
            direction = BOTTOM_LEFT; break;
          case BOTTOM_RIGHT : 
            p.x++; p.y++;
            direction = BOTTOM_LEFT; break;
          case RIGHT : 
            p.x++;
            direction = BOTTOM_RIGHT; break;
          case TOP_RIGHT : 
            p.x++; p.y--;
            direction = BOTTOM_RIGHT; break;
          case TOP : 
            p.y--;
            direction = TOP_RIGHT; break;
        }
        break;
      }
    }
  }
  while(initX != p.x || initY != p.y);

  MyGPath* res = malloc(sizeof(MyGPath));
  res->path = gpath_create(&pathinfo);
  memcpy(res->corners, corners_ids, 4 * sizeof(uint16_t));

  return res;
}


GPath* gpath2gpath(MyGPath* path1, MyGPath* path2, uint8_t percent){
  uint32_t max_num_points = 100;
  GPathInfo pathinfo = {
    0,
    (GPoint*)malloc(max_num_points *sizeof(GPoint))
  };

  for(int i=0; i<4; i++){
    uint16_t numpoints_p1 = 0;
    if(path1->corners[i] > path1->corners[(i+1) % 4]){
      numpoints_p1 = path1->corners[(i+1) % 4] + path1->path->num_points - path1->corners[i];
    }
    else {
      numpoints_p1 = path1->corners[(i+1) % 4] - path1->corners[i];
    }
    uint16_t numpoints_p2 = 0;
    if(path2->corners[i] > path2->corners[(i+1) % 4]){
      numpoints_p2 = path2->corners[(i+1) % 4] + path2->path->num_points - path2->corners[i];
    }
    else {
      numpoints_p2 = path2->corners[(i+1) % 4] - path2->corners[i];
    }

    uint16_t numpoints = numpoints_p1 > numpoints_p2 ? numpoints_p1 : numpoints_p2;
    GPoint p;
    GPoint p1;
    GPoint p2;
    for(uint32_t j=0; j<numpoints; j++){
      if(numpoints_p1 > numpoints_p2){
        p1 =  path1->path->points[(path1->corners[i] + j) % path1->path->num_points];
        p2 =  path2->path->points[(path2->corners[i] + j * numpoints_p2 / numpoints_p1) % path2->path->num_points];
      }
      else {
        p1 =  path1->path->points[(path1->corners[i] + j * numpoints_p1 / numpoints_p2) % path1->path->num_points];
        p2 =  path2->path->points[(path2->corners[i] + j) % path2->path->num_points];
      }
      p.x = (p2.x - p1.x) * percent / 100 + p1.x;
      p.y = (p2.y - p1.y) * percent / 100 + p1.y;
      addPoint(&pathinfo, &max_num_points, p);
    }
  }
  
  return gpath_create(&pathinfo);
}

void drawBitmapInBitmap(GBitmap* context, GPoint offset, GBitmap* image2draw){
  GRect bounds = image2draw->bounds;
  GRect context_bounds = context->bounds;
  for(int y=0; y<bounds.size.h; y++){
    if(y + offset.y >= context_bounds.origin.y && y + offset.y < context_bounds.origin.y + context_bounds.size.h){
      for(int x=0; x<bounds.size.w; x++){
        if(x + offset.x >= context_bounds.origin.x && x + offset.x < context_bounds.origin.x + context_bounds.size.w){
          if(getPixel(image2draw, x + bounds.origin.x, y + bounds.origin.y) == GColorBlack){
            setBlackPixel(context, offset.x + x, offset.y + y);
          }
        }
      }
    }
  }
}

