// Created with TexturePacker (http://www.codeandweb.com/texturepacker)
//
// Sprite sheet: spritesheet.png (448 x 114)
//
// $TexturePacker:SmartUpdate:f1c7a246f89f3d84588e55a3fd47b58d:9fa511aacc948cd1c4a092cc0c8ff232:f255187afaed5e6299dc3d2f34bbb167$

#include <pebble.h>

typedef enum {
  INVALID_SPRITE = 0,
  SPRITE_ID_DIGIT0_0,
  SPRITE_ID_DIGIT0_1,
  SPRITE_ID_DIGIT0_2,
  SPRITE_ID_DIGIT1_0,
  SPRITE_ID_DIGIT1_1,
  SPRITE_ID_DIGIT1_2,
  SPRITE_ID_DIGIT1_3,
  SPRITE_ID_DIGIT1_4,
  SPRITE_ID_DIGIT1_5,
  SPRITE_ID_DIGIT1_6,
  SPRITE_ID_DIGIT1_7,
  SPRITE_ID_DIGIT1_8,
  SPRITE_ID_DIGIT1_9,
  SPRITE_ID_DIGIT2_0,
  SPRITE_ID_DIGIT2_1,
  SPRITE_ID_DIGIT2_2,
  SPRITE_ID_DIGIT2_3,
  SPRITE_ID_DIGIT2_4,
  SPRITE_ID_DIGIT2_5,
  SPRITE_ID_DIGIT3_0,
  SPRITE_ID_DIGIT3_1,
  SPRITE_ID_DIGIT3_2,
  SPRITE_ID_DIGIT3_3,
  SPRITE_ID_DIGIT3_4,
  SPRITE_ID_DIGIT3_5,
  SPRITE_ID_DIGIT3_6,
  SPRITE_ID_DIGIT3_7,
  SPRITE_ID_DIGIT3_8,
  SPRITE_ID_DIGIT3_9,
  SPRITE_ID_MELTED_EXT0_LEFT_TOP,
  SPRITE_ID_MELTED_EXT0_TOP_RIGHT,
  SPRITE_ID_MELTED_EXT1_RIGHT_BOTTOM,
  SPRITE_ID_MELTED_EXT1_RIGHT_CENTER,
  SPRITE_ID_MELTED_EXT1_RIGHT_TOP,
  SPRITE_ID_MELTED_EXT1_TOP_LEFT,
  SPRITE_ID_MELTED_EXT2_BOTTOM_RIGHT,
  SPRITE_ID_MELTED_EXT2_LEFT_BOTTOM,
  SPRITE_ID_MELTED_EXT2_LEFT_CENTER,
  SPRITE_ID_MELTED_EXT2_LEFT_TOP,
  SPRITE_ID_MELTED_EXT3_BOTTOM_RIGHT,
  SPRITE_ID_MELTED_EXT3_RIGHT_BOTTOM,
  SPRITE_ID_MELTED_EXT3_RIGHT_CENTER,
  SPRITE_ID_MELTED_EXT3_RIGHT_TOP,
} SpriteId;

GBitmap* gbitmap_create_with_sprite(SpriteId sprite_id);
void destroy_sprite();
