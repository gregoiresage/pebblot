#include <pebble.h>
#include "settings.h"
#include "digits.h"
#include "digit_bitmaps.h"
#include "melted_bitmaps.h"
#include "symmetry.h"
#include "positions.h"
#include "spritesheet.h"
#include "bitmap.h"

typedef struct DisplayState {
  uint8_t digits[4];
  bool steel_offset;
  bool bt_connected;
  bool inverted;
  bool symmetry;
  bool melted;
  bool pm_dot;
} DisplayState;

static DisplayState* state;
static Settings *settings;
static Window *window;
static Layer *canvas;
static InverterLayer *inverter;
static uint8_t loaded_digits[4];

static MyGPath* digit_from_bitmaps[4];
static MyGPath* digit_to_bitmaps[4];
static GPath* animated_bitmaps[4];
static bool animated[4];

static GBitmap* tmp_bitmap;
static GSize tmp_bitmap_size = {144, 168};

static AppTimer* timer;
static int percent = 0;
static int interval = 10;

void update_inverter(bool bt_connected) {
  state->bt_connected = bt_connected;
  state->inverted = (settings->bgcolor == GColorWhite) ^ (!state->bt_connected && settings->bt_invert);
  layer_set_hidden(inverter_layer_get_layer(inverter), state->inverted);
  layer_mark_dirty(inverter_layer_get_layer(inverter));
}

static void timer_callback(void *data) {
  timer = NULL;

  if(percent == 100){
    for(int i=0; i<4; i++){
      animated[i] = false;
      if(animated_bitmaps[i]){
        free(animated_bitmaps[i]->points);
        gpath_destroy(animated_bitmaps[i]);
        animated_bitmaps[i] = NULL;
      }
    }
    percent = 0;
  }
  else {
    for(int i=0; i<4; i++){
      if(animated[i]){
        if(animated_bitmaps[i]){
          free(animated_bitmaps[i]->points);
          gpath_destroy(animated_bitmaps[i]);
          animated_bitmaps[i] = NULL;
        }
        if(digit_from_bitmaps[i] && digit_to_bitmaps[i])
          animated_bitmaps[i] = gpath2gpath(digit_from_bitmaps[i], digit_to_bitmaps[i], percent);
      }
    }
    timer = app_timer_register(100, timer_callback, NULL);
    percent += interval;
  }

  layer_mark_dirty(canvas);
  update_inverter(bluetooth_connection_service_peek());
}

void update_screen() {
 bool hour24;

  if (settings->time_display == TimeDispModeAuto) {
    hour24 = clock_is_24h_style();
  } else {
    hour24 = settings->time_display == TimeDispMode24H;
  }

  time_t now = time(NULL);
  struct tm * tm_now = localtime(&now);

  state->symmetry = settings->screen_mode != ScreenModeSimple;
  state->melted = settings->screen_mode == ScreenModeInsane;
  if (settings->steel_offset == SteelOffsetAuto) {
    state->steel_offset = watch_info_get_model() == WATCH_INFO_MODEL_PEBBLE_STEEL;
  } else {
    state->steel_offset = settings->steel_offset;
  }

  // Create 'from' pathes (copy of the 'to' pathes)
  for (int i = 0; i < 4; i++) {
    if(digit_from_bitmaps[i]){
      free(digit_from_bitmaps[i]->path->points);
      gpath_destroy(digit_from_bitmaps[i]->path);
      free(digit_from_bitmaps[i]);
    }
    digit_from_bitmaps[i] = digit_to_bitmaps[i];
  }

  for (int i = 0; i < 4; i++) {
    state->digits[i] = get_time_digit(i, tm_now, hour24);
    if (loaded_digits[i] != state->digits[i]) {
      loaded_digits[i] = state->digits[i];
      animated[i] = true;
    }
    else {
      // if(digit_from_bitmaps[i]){
      //   free(digit_from_bitmaps[i]->points);
      //   gpath_destroy(digit_from_bitmaps[i]);
      //   digit_from_bitmaps[i] = NULL;
      // }
      animated[i] = false;
    }
  }

  // Clear bitmap
  memset(tmp_bitmap->addr, 0xFF, tmp_bitmap->row_size_bytes * tmp_bitmap->bounds.size.h);

  // Create 'to' bitmaps
  for (int i = 0; i < 4; i++) {
    GBitmap* tmp = get_digit_bitmap(i, state->digits[i]);
    GPoint offset = get_digit_position(i, state->steel_offset).origin;
    drawBitmapInBitmap(tmp_bitmap, offset, tmp);
    // if (state->melted) 
    {
      draw_digit_external_melted_parts(i, state->digits[i], tmp_bitmap, state->steel_offset);
    }
    gbitmap_destroy(tmp);
  }

  // Create 'to' pathes
  for (int i = 0; i < 4; i++) {
    GRect bounds = get_zone_position(i, state->steel_offset);
    GBitmap* sub = gbitmap_create_as_sub_bitmap(tmp_bitmap, bounds);
    digit_to_bitmaps[i] = gpath_create_from_bitmap(sub);
    gbitmap_destroy(sub);
  }

  if(timer){
    app_timer_cancel(timer);
  }
  timer = app_timer_register(10, timer_callback, NULL);
}

static void update_canvas(struct Layer *layer, GContext *ctx) {
  for (int i = 0; i < 4; i++) {
    if(animated_bitmaps[i]){
      gpath_draw_filled(ctx, animated_bitmaps[i]);
    }
    else if(digit_to_bitmaps[i]) {
      gpath_draw_filled(ctx, digit_to_bitmaps[i]->path);
      gpath_draw_outline(ctx, digit_to_bitmaps[i]->path);
    }
  }
  
  // if (state->symmetry) 
  {
    GBitmap* buffer = graphics_capture_frame_buffer(ctx);
    pebblot_symmetry(buffer, state->steel_offset);
    graphics_release_frame_buffer(ctx, buffer);
  }
}

static void window_load(Window *window) {
  // window_set_background_color(window, GColorBlack);
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  canvas = layer_create(bounds);
  layer_set_update_proc(canvas, update_canvas);
  layer_add_child(window_layer, canvas);

  inverter = inverter_layer_create(bounds);
  layer_add_child(window_layer, inverter_layer_get_layer(inverter));

  tmp_bitmap = gbitmap_create_blank(tmp_bitmap_size);
  memset(tmp_bitmap->addr, 0xFF, tmp_bitmap->row_size_bytes * tmp_bitmap->bounds.size.h);

  memset(state->digits, 0, 4);

  // init bitmap
  for (int i = 0; i < 4; i++) {
    GBitmap* tmp = get_digit_bitmap(i, state->digits[i]);
    GPoint offset = get_digit_position(i, state->steel_offset).origin;
    drawBitmapInBitmap(tmp_bitmap, offset, tmp);
    gbitmap_destroy(tmp);
  }

  // Create 'to' pathes
  for (int i = 0; i < 4; i++) {
    GRect bounds = get_zone_position(i, state->steel_offset);
    GBitmap* sub = gbitmap_create_as_sub_bitmap(tmp_bitmap, bounds);
    digit_to_bitmaps[i] = gpath_create_from_bitmap(sub);
    gbitmap_destroy(sub);
  }
}

static void window_unload(Window *window) {
  layer_destroy(canvas);
  inverter_layer_destroy(inverter);
  for (int i = 0; i < 4; i++) {
    if(animated_bitmaps[i]) {
      free(animated_bitmaps[i]->points);
      gpath_destroy(animated_bitmaps[i]);
    }
    if(digit_from_bitmaps[i]) {
      free(digit_from_bitmaps[i]->path->points);
      gpath_destroy(digit_from_bitmaps[i]->path);
      free(digit_from_bitmaps[i]);
    }
    if(digit_to_bitmaps[i]) {
      free(digit_to_bitmaps[i]->path->points);
      gpath_destroy(digit_to_bitmaps[i]->path);
      free(digit_to_bitmaps[i]);
    }
  }
  gbitmap_destroy(tmp_bitmap);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_screen();
  if (units_changed & HOUR_UNIT && settings->hour_vibes) vibes_double_pulse();
}

static void in_recv_handler(DictionaryIterator *received, void *context) {
  settings->bgcolor = (GColor) dict_find(received, APPKEY_BGCOLOR)->value->int32;
  settings->screen_mode = (ScreenMode) dict_find(received, APPKEY_DISPLAY)->value->int32;
  settings->time_display = (TimeDisplayMode) dict_find(received, APPKEY_HOUR24)->value->int32;
  settings->steel_offset = (SteelOffset) dict_find(received, APPKEY_STEEL_OFFSET)->value->int32;
  settings->bt_invert = (bool) dict_find(received, APPKEY_BT_INVERT)->value->int32;
  settings->bt_vibes = (bool) dict_find(received, APPKEY_BT_VIBES)->value->int32;
  settings->hour_vibes = (bool) dict_find(received, APPKEY_HOUR_VIBES)->value->int32;

  update_screen();
}

static void bt_handler(bool connected) {
  update_inverter(connected);
  if (!connected && settings->bt_vibes) vibes_long_pulse();
}

static void init(void) {
  settings = settings_create();
  persist_read_settings(settings);
  state = malloc(sizeof(DisplayState));

  // React to settings message from phone
  app_message_register_inbox_received(in_recv_handler);
  app_message_open(1 + (7+sizeof(uint32_t)) * APPKEY_COUNT, 0);

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  bluetooth_connection_service_subscribe(bt_handler);
}

static void deinit(void) {
  window_destroy(window);

  texture_destroy_all();
  persist_write_settings(settings);
  settings_destroy(settings);
  free(state);
  bluetooth_connection_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
