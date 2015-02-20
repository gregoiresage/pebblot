#pragma once

#include <pebble.h>

typedef struct MyGPath {
  uint16_t          corners[4];
  GPath*        	path;
} MyGPath;

#define setBlackPixel(bmp, x, y) ((((uint8_t *)bmp->addr)[(y) * bmp->row_size_bytes + (x) / 8] &= ~(0x01 << ((x)%8))))
#define setWhitePixel(bmp, x, y) ((((uint8_t *)bmp->addr)[(y) * bmp->row_size_bytes + (x) / 8] |= (0x01 << ((x)%8))))
#define getPixel(bmp, xx, yy)\
(((xx) >= bmp->bounds.size.w + bmp->bounds.origin.x || (yy) >= bmp->bounds.size.h + bmp->bounds.origin.y || (xx) < bmp->bounds.origin.x || (yy) < bmp->bounds.origin.y) ?\
 -1 :\
 ((((uint8_t *)bmp->addr)[(yy)*bmp->row_size_bytes + (xx)/8] & (1<<((xx)%8))) != 0))

MyGPath* gpath_create_from_bitmap(GBitmap* bitmap);
void drawBitmapInBitmap(GBitmap* context, GPoint offset, GBitmap* image2draw);

GPath* gpath2gpath(MyGPath* path1, MyGPath* path2, uint8_t percent);
