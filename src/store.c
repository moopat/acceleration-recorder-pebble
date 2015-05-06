#include <pebble.h>
#include "store.h"
#include "config.h"

unsigned int store[CFG_RETAINED_SAMPLES];
int write_position = 0;

void add_to_store(unsigned int new_data[]){
	// Check size of store. Rely that new_data contains CFG_BATCH_SIZE samples.
	if(CFG_BATCH_SIZE + write_position > CFG_RETAINED_SAMPLES){
		// Subtract number of remaining spots from the batch size.
		remove_from_store(write_position + CFG_BATCH_SIZE - CFG_RETAINED_SAMPLES);
	}
	for(int i = 0; i < CFG_BATCH_SIZE; i++){
		store[write_position] = new_data[i];
		write_position++;
	}
	//APP_LOG(APP_LOG_LEVEL_INFO, "Added data. Next writing position is %d.", write_position);
}

void remove_from_store(int number_of_samples){
	for(int i = 0; i + number_of_samples < CFG_RETAINED_SAMPLES; i++){
		store[i] = store[number_of_samples + i];
		// For debug reasons, we flag empty fields:
		store[number_of_samples + i] = 0;
	}
	write_position = write_position - number_of_samples;
}

void get_batch_from_store(unsigned int *batch){
	//memcpy(batch, &store[0], ??) TODO: Check out how that can be used here.
	for(int i = 0; i < CFG_BATCH_SIZE; i++){
		batch[i] = store[i];
	}
}

bool has_stored_data(){
	return write_position > 0;
}
