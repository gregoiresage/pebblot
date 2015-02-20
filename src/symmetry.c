#include <pebble.h>
#include "symmetry.h"
#include "bitmap.h"

void pebblot_symmetry(GBitmap* buffer, bool steel_offset) {
  uint16_t lines = buffer->bounds.size.h;
  uint16_t cols = buffer->bounds.size.w;
  uint16_t l = 0;
  uint16_t c = 0;
  while(l < lines){
    c = 0;
    while(c < cols){
      if(getPixel(buffer, c, l) == GColorBlack)
        setBlackPixel(buffer, cols - 1 - c, l);
      c++;
    }
    l++;
  }
}
