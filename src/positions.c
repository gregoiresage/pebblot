#include <pebble.h>
#include "positions.h"

#define DigitBigSize {27, 59}
#define DigitSmallSize {25, 55}

static const GRect DigitPosition[4] = {
  { {  75, 21 }, DigitBigSize },
  { { 109, 25 }, DigitSmallSize },
  { {  10, 88 }, DigitSmallSize },
  { {  42, 88 }, DigitBigSize },
};

GRect get_digit_position(uint8_t position, bool steel_offset) {
  GRect result = DigitPosition[position];
  if (steel_offset) result.origin.y += SteelOffsetPixels;
  return result;
}

#define BitmapSize {144/4, 168/2}

static const GRect BitmapPosition[4] = {
  { {  72, 0 }, BitmapSize },
  { { 3*144/4, 0 }, BitmapSize },
  { {  0, 84 }, BitmapSize },
  { {  36, 84}, BitmapSize },
};

GRect get_zone_position(uint8_t position, bool steel_offset) {
  GRect result = BitmapPosition[position];
  if (steel_offset) result.origin.y += SteelOffsetPixels;
  return result;
}

