#include <pebble.h>

// This is a custom defined key for saving our count field
#define P1_SCORE_PKEY 1
#define P2_SCORE_PKEY 2
#define UNDO_PKEY 3

// You can define defaults for values in persistent storage
#define P1_SCORE_DEFAULT 0
#define P2_SCORE_DEFAULT 0
#define UNDO_DEFAULT 0

static Window *window;

static GBitmap *action_icon_plus;

static ActionBarLayer *action_bar;

static TextLayer *p1_text_layer, *p2_text_layer;
static TextLayer *score_text_layer;

static char *player_name_1 = "Joel";
static char *player_name_2 = "Luis";
static char *score;

// We'll save the counters and the undo buffer in memory from persistent storage
static int p1_score = P1_SCORE_DEFAULT;
static int p2_score = P2_SCORE_DEFAULT;
static int undo = UNDO_DEFAULT;

static void update_text() {
  static char score[10];
  snprintf(score, sizeof(score), "%u - %u", p1_score, p2_score);
  text_layer_set_text(score_text_layer, score);
}

static void p1_increment_click_handler(ClickRecognizerRef recognizer, void *context) {
  p1_score++;
  undo = undo << 1;
  update_text();
}

static void p2_increment_click_handler(ClickRecognizerRef recognizer, void *context) {
  p2_score++;
  undo = undo << 1;
  undo += 1;
  update_text();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  
    
  // UNDO functionality
  if (p1_score + p2_score <= 0)
    return;

  if (undo % 2 == 0){
    p1_score--;
  }else{
    p2_score--;
  }

  undo = undo >> 1;  
  update_text();
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  
  // RESET functionality
  p1_score = p2_score = undo = 0;
  
  update_text();
}

static void empty_click_handler(ClickRecognizerRef recognizer, void *context) {
  return;
}

static void click_config_provider(void *context) {
  const uint16_t click_length_ms = 500;
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) p1_increment_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) p2_increment_click_handler);
    
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) select_click_handler);

  window_long_click_subscribe(BUTTON_ID_SELECT, click_length_ms,
                              (ClickHandler) empty_click_handler,
                              (ClickHandler) select_long_click_handler);
}

static void window_load(Window *me) {
  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, me);
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_plus);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_plus);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_plus);

  Layer *layer = window_get_root_layer(me);
  const int16_t width = layer_get_frame(layer).size.w - ACTION_BAR_WIDTH - 3;

  p1_text_layer = text_layer_create(GRect(4, 10, width, 60));
  text_layer_set_font(p1_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_background_color(p1_text_layer, GColorClear);
  text_layer_set_text(p1_text_layer, player_name_1);
  layer_add_child(layer, text_layer_get_layer(p1_text_layer));

  score_text_layer = text_layer_create(GRect(4, 60, width, 60));
  text_layer_set_font(score_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_background_color(score_text_layer, GColorClear);
  text_layer_set_text(score_text_layer, score);
  layer_add_child(layer, text_layer_get_layer(score_text_layer));
  
  p2_text_layer = text_layer_create(GRect(4, 110, width, 60));
  text_layer_set_font(p2_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_background_color(p2_text_layer, GColorClear);
  text_layer_set_text(p2_text_layer, player_name_2);
  layer_add_child(layer, text_layer_get_layer(p2_text_layer));

  update_text();
}

static void window_unload(Window *window) {
  text_layer_destroy(p1_text_layer);
  text_layer_destroy(p2_text_layer);

  action_bar_layer_destroy(action_bar);
}

static void init(void) {
  action_icon_plus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_PLUS);

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  // Get the count from persistent storage for use if it exists, otherwise use the default
  p1_score = persist_exists(P1_SCORE_PKEY) ? persist_read_int(P1_SCORE_PKEY) : P1_SCORE_DEFAULT;
  p2_score = persist_exists(P2_SCORE_PKEY) ? persist_read_int(P2_SCORE_PKEY) : P2_SCORE_DEFAULT;

  window_stack_push(window, true /* Animated */);
}

static void deinit(void) {
  // Save the count into persistent storage on app exit
  persist_write_int(P1_SCORE_PKEY, p1_score);
  persist_write_int(P2_SCORE_PKEY, p2_score);

  window_destroy(window);

  gbitmap_destroy(action_icon_plus);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
