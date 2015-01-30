#include <pebble.h>

  
static Window *s_window;
static GFont font_s;
static GFont font_b;
static TextLayer *general_info;
static TextLayer *accel_info;
  
static void initialise_ui(void) {
  s_window = window_create();
  window_set_fullscreen(s_window, true);
  
  font_s = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  font_b = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  
  general_info = text_layer_create(GRect(0, 0, 144, 80));
  text_layer_set_text_alignment(general_info, GTextAlignmentCenter);
  text_layer_set_font(general_info, font_s);
  layer_add_child(window_get_root_layer(s_window), (Layer *)general_info);
  
  accel_info = text_layer_create(GRect(0, 80, 144, 92));
  text_layer_set_text_alignment(accel_info, GTextAlignmentCenter);
  text_layer_set_font(accel_info, font_b);
  layer_add_child(window_get_root_layer(s_window), (Layer *)accel_info);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(general_info);
  text_layer_destroy(accel_info);
}

static void handle_window_unload(Window* window) {
  destroy_ui();
}

static void flash(TextLayer * layer) {
  static bool layer_bg = GColorWhite;
  text_layer_set_text_color(layer, layer_bg);
  layer_bg = layer_bg == GColorBlack ? GColorWhite : GColorBlack;
  text_layer_set_background_color(layer, layer_bg);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  flash(general_info);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  vibes_long_pulse();
}

static AccelSamplingRate sampling_rate = ACCEL_SAMPLING_25HZ;
static char * sampling_rate_str() {
  static char str[5] = "100Hz";
  snprintf(str, 5, "%dHz", sampling_rate);
  return str;
}
static int num_samples = 25;

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (num_samples < 25) {
    num_samples++;
  } else {
    num_samples = 0;
  }
  accel_service_set_samples_per_update(num_samples);
}

static void down_long_click_handler_down(ClickRecognizerRef recognizer, void *context) {
  switch(sampling_rate) {
    case ACCEL_SAMPLING_10HZ:  sampling_rate = ACCEL_SAMPLING_25HZ ; break;
    case ACCEL_SAMPLING_25HZ:  sampling_rate = ACCEL_SAMPLING_50HZ ; break;
    case ACCEL_SAMPLING_50HZ:  sampling_rate = ACCEL_SAMPLING_100HZ; break;
    case ACCEL_SAMPLING_100HZ: sampling_rate = ACCEL_SAMPLING_10HZ ; break;
  }
  accel_service_set_sampling_rate(sampling_rate);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_long_click_subscribe(BUTTON_ID_DOWN, 500, down_long_click_handler_down, NULL);
}
  
void tap_service(AccelAxisType axis, int32_t direction) {

  static char label[64];
  char * axis_str = "_";
  switch(axis) {
    case ACCEL_AXIS_X: axis_str = "X"; break;
    case ACCEL_AXIS_Y: axis_str = "Y"; break;
    case ACCEL_AXIS_Z: axis_str = "Z"; break;
    default: axis_str = "?"; break;
  }
  snprintf(label, 64, "Axis: %s,\n direction: \n %li", axis_str, direction);
  text_layer_set_text(general_info, label);
}

void accel_service(AccelData *data, uint32_t num_samples) {
  static uint32_t last_timestamp = 0;
  
  uint32_t period = data[num_samples-1].timestamp - last_timestamp;
  
  int32_t x=0, y=0, z = 0;
  int samples = 0;
  for (uint32_t i = 0; i<num_samples; i++) {
    // Discard data measured when watch vibrated.
    if (!data[i].did_vibrate) {
      x += data[i].x;
      y += data[i].y;
      z += data[i].z;
      samples++;
    }
  }
  if (samples > 0) {
    x = x/samples;
    y = y/samples;
    z = z/samples;
  }
  
  static char label1[128];
  static char label2[128];
  snprintf(label1, 128, "Time/Period:\n%lu/%dms\nSamples: %d/%d\nSampl. rate: %s", (unsigned long)data[0].timestamp, (int16_t)period, samples, (int)num_samples, sampling_rate_str());
  snprintf(label2, 128, "X: %i mG\nY: %i mG\nZ: %i mG", (int16_t)x, (int16_t)y, (int16_t)z);
  text_layer_set_text(general_info, label1);
  text_layer_set_text(accel_info, label2);
  
  last_timestamp = data[num_samples-1].timestamp;
}

void show_screen_main(void) {
  initialise_ui();

  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_set_click_config_provider(s_window, click_config_provider);
  window_stack_push(s_window, true);
  
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
  accel_data_service_subscribe(25, accel_service);
//   accel_tap_service_subscribe(tap_service);
}

void hide_screen_main(void) {
  window_stack_remove(s_window, true);
}

static void init(void) {
  show_screen_main();
}

static void deinit(void) {
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
