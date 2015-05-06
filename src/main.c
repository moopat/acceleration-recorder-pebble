#include <pebble.h>
#include <inttypes.h>
#include "store.h"
#include "util.h"
#include "main.h"
#include "config.h"

#define KEY_COMMAND 0 // the key tells the smartphone what kind of data to expect
#define COMMAND_DATA 1
#define NUMBER_PARAMETERS 1 // number of parameters for every sample
	
const AccelSamplingRate CFG_SAMPLING_RATE_PBL = ACCEL_SAMPLING_50HZ;

static Window *s_main_window;
static TextLayer *s_status_layer;
static TextLayer *s_clock_layer;
static GFont s_clock_font;
static bool sending = false;

static void send_batch(){
	if(!has_stored_data()){
		sending = false;
		return;
	}
	sending = true;

	DictionaryIterator *iterator;
	app_message_outbox_begin(&iterator);
	dict_write_int8(iterator, KEY_COMMAND, COMMAND_DATA);
	
	unsigned int batch[CFG_BATCH_SIZE];
	get_batch_from_store(batch);
	for(uint sample = 0; sample < CFG_BATCH_SIZE; sample++){
		/*
		VERTICAL ACCELERATION = 0
		+1 is added to ensure that 0 is always the command.
		*/
		Tuplet t = TupletInteger(NUMBER_PARAMETERS * sample + 0 + 1, batch[sample]);
		dict_write_tuplet(iterator, &t);
	}
	app_message_outbox_send();
}

//static uint count = 0;
// A new batch of acceleration data was received.
static void data_handler(AccelData *data, uint32_t num_samples) {	
	unsigned int batch[CFG_BATCH_SIZE];
	
	for(uint sample = 0; sample < num_samples; sample++){
		//batch[sample] = ++count;
		batch[sample] = get_vertical_acceleration(data[sample].x, data[sample].y, data[sample].z);
	}
	
	add_to_store(batch);
	
	if(!sending){
		send_batch();
	}
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
	GRect window_bounds = layer_get_bounds(window_layer);
	
	window_set_background_color(window, GColorBlack);
	
	s_status_layer = text_layer_create(GRect(0, window_bounds.size.h-22, window_bounds.size.w, 20));
	text_layer_set_background_color(s_status_layer, GColorClear);
	text_layer_set_text_color(s_status_layer, GColorWhite);
	text_layer_set_font(s_status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(s_status_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_status_layer));
	
	s_clock_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_SEVEN_50));
	
	s_clock_layer = text_layer_create(GRect(0, 40, window_bounds.size.w, window_bounds.size.h - 62));
	text_layer_set_background_color(s_clock_layer, GColorClear);
	text_layer_set_text_color(s_clock_layer, GColorWhite);
	text_layer_set_font(s_clock_layer, s_clock_font);
	text_layer_set_text_alignment(s_clock_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_clock_layer));
	
	update_clock();
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_status_layer);
	text_layer_destroy(s_clock_layer);
	fonts_unload_custom_font(s_clock_font);
}

static void update_status(bool success){
	time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
	
	static char buffer[] = "OK@00:00:00";
	
	if(success){
		strftime(buffer, sizeof(buffer), "OK@%H:%M:%S", tick_time);
	} else {
		strftime(buffer, sizeof(buffer), "ER@%H:%M:%S", tick_time);
	}
	
  text_layer_set_text(s_status_layer, buffer);
}

static void update_clock(){
	time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
	
	static char buffer[] = "00:00";
	strftime(buffer, sizeof(buffer), "%H:%M", tick_time);
	text_layer_set_text(s_clock_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_clock();	
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed: %i - %s", reason, translate_error(reason));
	update_status(false);
	send_batch();
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
	update_status(true);
	remove_from_store(CFG_BATCH_SIZE);
	send_batch();
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
	accel_data_service_subscribe(CFG_BATCH_SIZE, data_handler);

	// Choose update rate
	accel_service_set_sampling_rate(CFG_SAMPLING_RATE_PBL);
	
	// Register callbacks
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
	// Sign up for time updates.
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
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
