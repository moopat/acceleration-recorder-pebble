#include <pebble.h>
#include <store.h>
	
#define STORE_BATCH_SIZE 25
#define STORE_RETENTION 75

unsigned int batch[STORE_BATCH_SIZE];
unsigned int store[STORE_RETENTION];
int write_position = 0;

void add_to_store(unsigned int new_data[]){
	//APP_LOG(APP_LOG_LEVEL_INFO, "Adding to store. Writing position is %d.", write_position);
	// Check size of store. Rely that new_data contains STORE_BATCH_SIZE samples.
	if((STORE_BATCH_SIZE + write_position) > STORE_RETENTION){
		// Subtract number of remaining spots from the batch size.
		remove_from_store(write_position + STORE_BATCH_SIZE - STORE_RETENTION);
	}
	for(int i = 0; i < STORE_BATCH_SIZE; i++){
		store[write_position] = new_data[i];
		write_position++;
	}
	//APP_LOG(APP_LOG_LEVEL_INFO, "Added data. Next writing position is %d.", write_position);
}

void remove_from_store(int number_of_samples){
	for(int i = 0; i < number_of_samples; i++){
		store[i] = store[number_of_samples + i];
		write_position--;
	}
	//APP_LOG(APP_LOG_LEVEL_INFO, "Removed %d samples. Next writing position is %d.", number_of_samples, write_position);
}

unsigned int* get_batch_from_store(){
	//memcpy(batch, &store[0], ??) TODO: Check out how that can be used here.
	for(int i = 0; i < STORE_BATCH_SIZE; i++){
		batch[i] = store[i];
	}
	return batch;
}

int get_store_size(){
	return write_position;
}