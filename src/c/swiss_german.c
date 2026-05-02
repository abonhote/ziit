#include "pebble.h"

// Debugging (man weiss ja nie ...)
#define DEBUG_TIME 1
#define DEBUG_H 23
#define DEBUG_M 24
#define DEBUG_S 40

#ifdef DEBUG_TIME
static time_t get_debug_start() {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  t->tm_hour = DEBUG_H;
  t->tm_min  = DEBUG_M;
  t->tm_sec  = DEBUG_S;
  
  return mktime(t);
}

static time_t debug_offset = 0;

static void init_debug_time() {
  debug_offset = get_debug_start() -time(NULL);
}

static time_t get_time() {
  return time(NULL) + debug_offset;
}
#else
static void get_debug_start() {}
static time_t get_time() { return time(NULL) }
#endif

static Window *window;
static TextLayer *minuteLayer; // The Minutes
static TextLayer *hourLayer; // The hours

static void init_text_layers(GRect bounds) {
  int paddingLeft = PBL_IF_ROUND_ELSE(0, 10);
  int top = (bounds.size.h - 3 * 42)/2 - 8;

  GRect minuteFrame = (GRect) {
    .origin = {
      paddingLeft,
      top
    },
    .size = {
      bounds.size.w - paddingLeft /* width */,
      3 * 42  /* height */
    }
  };
  minuteLayer = text_layer_create(minuteFrame);
  text_layer_set_text_color(minuteLayer, GColorWhite);
  text_layer_set_background_color(minuteLayer, GColorClear);
  text_layer_set_text_alignment(minuteLayer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
  GFont bitham = fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT);
  text_layer_set_font(minuteLayer, bitham);

  GRect hourFrame = (GRect) {
    .origin = {
      paddingLeft,
      top + (2 * 42)
    },
    .size = {
      bounds.size.w - paddingLeft /* width */,
      42 + 10 /* height */
    }
  };
  hourLayer = text_layer_create(hourFrame);
  text_layer_set_text_color(hourLayer, GColorWhite);
  text_layer_set_background_color(hourLayer, GColorClear);
  text_layer_set_text_alignment(hourLayer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
  GFont bithamBold = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  text_layer_set_font(hourLayer, bithamBold);
  
}

static void prv_unobstructed_change(AnimationProgress progress, void *context) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_unobstructed_bounds(window_layer);
  int top = (bounds.size.h - 3 * 42)/2 - 8;

  GRect minuteFrame  = layer_get_frame(text_layer_get_layer(minuteLayer));
  minuteFrame.origin.y = top;
  layer_set_frame(text_layer_get_layer(minuteLayer), minuteFrame);

  GRect hourFrame = layer_get_frame(text_layer_get_layer(hourLayer));
  hourFrame.origin.y = top + (42 * 2);
  layer_set_frame(text_layer_get_layer(hourLayer), hourFrame);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_unobstructed_bounds(window_layer);
  init_text_layers(bounds);
  layer_add_child(window_layer, text_layer_get_layer(minuteLayer));
  text_layer_enable_screen_text_flow_and_paging(minuteLayer, 0);
  layer_add_child(window_layer, text_layer_get_layer(hourLayer));
  text_layer_enable_screen_text_flow_and_paging(hourLayer, 2);

  UnobstructedAreaHandlers handler = {
    .change = prv_unobstructed_change
  };
  unobstructed_area_service_subscribe(handler, NULL);
}

static void window_unload(Window *window) {
  text_layer_destroy(minuteLayer);
  text_layer_destroy(hourLayer);
}


static void display_time(struct tm *time) {

  const char *hrs[12] = { "zwölfi", "eis","zwei", "drü", "vieri", "foifi", "sächsi",
		 "sibni", "achti", "nüni", "zäni", "elfi" };
  const char *hour_string[25];
  for (int i = 0 ; i < 12; i++) hour_string[i] = hrs[i];
  for (int i = 0 ; i < 12; i++) hour_string[i+12] = hrs[i];
  hour_string[25] = hrs[0];

  int hour = time->tm_hour;
  int min = time->tm_min;
  char minute_text[50];
  char hour_text[50];
  if (min < 25) {
    strcpy(hour_text , hour_string[hour]);
  } else {
  	strcpy(hour_text , hour_string[hour + 1]);
  }
  if (-1 < min && min < 5) {
  	 strcpy(minute_text , "");
  }
  if (4 < min && min < 10) {
  	 strcpy(minute_text , "\nföif ab");
  }
  if (9 < min && min < 15) {
  	 strcpy(minute_text , "\nzäh ab");
  }
  if (14 < min && min < 20) {
  	 strcpy(minute_text , "viertel ab");
  }
  if (19 < min && min < 25) {
  	 strcpy(minute_text , "zwänzg ab");
  }
  if (24 < min && min < 30) {
  	 strcpy(minute_text , "föif vor halbi");
  }
  if (29 < min && min < 35) {
  	 strcpy(minute_text , "\nhalbi");
  }
  if (34 < min && min < 40) {
  	 strcpy(minute_text , "föif ab halbi");
  }
  if (39 < min && min < 45) {
  	 strcpy(minute_text , "zwänzg vor");
  }
  if (44 < min && min < 50) {
  	 strcpy(minute_text , "viertel vor");
  }
  if (49 < min && min < 55) {
  	 strcpy(minute_text , "\nzäh vor");
  }
  if (54 < min && min < 60) {
  	 strcpy(minute_text , "\nföif vor");
  }
  
  static char staticTimeText[50] = ""; // Needs to be static because it's used by the system later.
  strcpy(staticTimeText , "");
  strcat(staticTimeText , minute_text);
  text_layer_set_text(minuteLayer, staticTimeText);

  static char staticHourText[50] = ""; // Needs to be static because it's used by the system later.
  strcpy(staticHourText , "");
  strcat(staticHourText , hour_text);
  text_layer_set_text(hourLayer, staticHourText);
}


static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  display_time(tick_time);
}

static void init(void) {
  init_debug_time();
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
  window_set_background_color(window, GColorBlack);
  
  time_t now = get_time();
  struct tm *tick_time = localtime(&now);
  display_time(tick_time);
  
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}


static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
