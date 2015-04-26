#include <pebble.h>
#include <inttypes.h>

#define KEY_COMMAND 0 // the key tells the smartphone what kind of data to expect
#define COMMAND_DATA 1
#define NUMBER_SAMPLES 25 // samples are reported every 25 samples, so every second
#define NUMBER_PARAMETERS 3 // number of parameters for every sample

static Window *s_main_window;
static TextLayer *s_status_layer;

// A new batch of acceleration data was received.
static void data_handler(AccelData *data, uint32_t num_samples) {	
	DictionaryIterator *iterator;
	app_message_outbox_begin(&iterator);

	dict_write_int8(iterator, KEY_COMMAND, COMMAND_DATA);
	
	for(uint sample = 0; sample < num_samples; sample++){
		
		/*
		X = 0
		Y = 1
		Z = 2
		+1 is added to ensure that 0 is always the command.
		*/

		// Add acceleration data
		Tuplet t3 = TupletInteger(NUMBER_PARAMETERS * sample + 0 + 1, data[sample].x);
		dict_write_tuplet(iterator, &t3);
		
		Tuplet t4 = TupletInteger(NUMBER_PARAMETERS * sample + 1 + 1, data[sample].y);
		dict_write_tuplet(iterator, &t4);
		
		Tuplet t5 = TupletInteger(NUMBER_PARAMETERS * sample + 2 + 1, data[sample].z);
		dict_write_tuplet(iterator, &t5);
	}
	
	app_message_outbox_send();
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
	GRect window_bounds = layer_get_bounds(window_layer);
	
	s_status_layer = text_layer_create(GRect(0, 20, window_bounds.size.w, window_bounds.size.h-40));
	text_layer_set_background_color(s_status_layer, GColorClear);
	text_layer_set_text_color(s_status_layer, GColorBlack);
	text_layer_set_font(s_status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(s_status_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_status_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_status_layer);
}

static void updateTextLayer(bool success){
	time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
	
	static char buffer[] = "SUCCESS\n@\n00:00:00";
	
	if(success){
		strftime(buffer, sizeof("SUCCESS\n@\n00:00:00"), "SUCCESS\n@\n%H:%M:%S", tick_time);
	} else {
		strftime(buffer, sizeof("ERROR\n@\n00:00:00"), "ERROR\n@\n%H:%M:%S", tick_time);
	}
	
  text_layer_set_text(s_status_layer, buffer);
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
	updateTextLayer(false);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
	updateTextLayer(true);
}

static void init() {
  // Create main Window
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  // Subscribe to the accelerometer data service
	accel_data_service_subscribe(NUMBER_SAMPLES, data_handler);

	// Choose update rate
	accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);
	
	// Register callbacks
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy main Window
  window_destroy(s_main_window);
	accel_data_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}