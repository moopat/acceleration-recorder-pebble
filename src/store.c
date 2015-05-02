#include <pebble.h>
#include <store.h>
	
#define STORE_BATCH_SIZE 25
#define STORE_RETENTION 75

unsigned int batch[STORE_BATCH_SIZE];
unsigned int store[STORE_RETENTION];
int write_position = 0;

void add_to_store(unsigned int new_data[]){
	// Check size of store. Rely that new_data contains STORE_BATCH_SIZE samples.
	if((STORE_BATCH_SIZE + write_position) > STORE_RETENTION){
		// Subtract number of remaining spots from the batch size.
		remove_from_store(STORE_BATCH_SIZE - (STORE_RETENTION - write_position));
	}
	for(int i = 0; i < STORE_BATCH_SIZE; i++){
		store[write_position] = new_data[i];
		write_position++;
	}
}

void remove_from_store(int number_of_samples){
	for(int i = 0; i < number_of_samples; i++){
		store[i] = store[number_of_samples + i];
		write_position--;
	}
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