#include <pebble.h>
#include "main.h"

static Window *s_window;
static TextLayer *s_label_secondtick, *s_label_animations;

// A struct for our specific settings (see main.h)
ClaySettings settings;

// Initialize the default settings
static void prv_default_settings() {
  settings.BackgroundColor = GColorBlack;
  settings.ForegroundColor = GColorWhite;
  settings.SecondTick = false;
  settings.Animations = false;
}

// Read settings from persistent storage
static void prv_load_settings() {
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  // Update the display based on new settings
  prv_update_display();
}

// Update the display elements
static void prv_update_display() {
  // Background color
  window_set_background_color(s_window, settings.BackgroundColor);
  text_layer_set_background_color(s_label_secondtick, settings.BackgroundColor);
  text_layer_set_background_color(s_label_animations, settings.BackgroundColor);

  // Foreground Color
  text_layer_set_text_color(s_label_secondtick, settings.ForegroundColor);
  text_layer_set_text_color(s_label_animations, settings.ForegroundColor);

  // Seconds
  if(settings.SecondTick) {
    text_layer_set_text(s_label_secondtick, "seconds: enabled");
  } else {
    text_layer_set_text(s_label_secondtick, "seconds: disabled");
  }

  // Animations
  if(settings.Animations) {
    text_layer_set_text(s_label_animations, "animations: enabled");
  } else {
    text_layer_set_text(s_label_animations, "animations: disabled");
  }
}

// Handle the response from AppMessage
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Background Color
  Tuple *bg_color_t = dict_find(iter, MESSAGE_KEY_BackgroundColor);
  if(bg_color_t) {
    settings.BackgroundColor = GColorFromHEX(bg_color_t->value->int32);
  }

  // Foreground Color
  Tuple *fg_color_t = dict_find(iter, MESSAGE_KEY_ForegroundColor);
  if(fg_color_t) {
    settings.ForegroundColor = GColorFromHEX(fg_color_t->value->int32);
  }

  // Second Tick
  Tuple *second_tick_t = dict_find(iter, MESSAGE_KEY_SecondTick);
  if(second_tick_t) {
    settings.SecondTick = second_tick_t->value->int32 == 1;
  }

  // Animations
  Tuple *animations_t = dict_find(iter, MESSAGE_KEY_Animations);
  if(animations_t) {
    settings.Animations = animations_t->value->int32 == 1;
  }

  // Save the new settings to persistent storage
  prv_save_settings();
}

// Window Load event
static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_label_secondtick = text_layer_create(GRect(0, bounds.size.h/4, bounds.size.w, 30));
  text_layer_set_text_alignment(s_label_secondtick, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_label_secondtick));

  s_label_animations = text_layer_create(GRect(0, bounds.size.h/2, bounds.size.w, 30));
  text_layer_set_text_alignment(s_label_animations, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_label_animations));

  prv_update_display();
}

// Window Unload event
static void prv_window_unload(Window *window) {
  layer_destroy(text_layer_get_layer(s_label_secondtick));
  layer_destroy(text_layer_get_layer(s_label_animations));
}

static void prv_init(void) {
  prv_load_settings();

  // Listen for AppMessages
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(128, 128);

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });

  window_stack_push(s_window, true);
}

static void prv_deinit(void) {

}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
