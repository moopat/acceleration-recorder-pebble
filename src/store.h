#ifndef H_STORE
#define H_STORE

void add_to_store(unsigned int new_data[]);
void remove_from_store(int number_of_samples);
void get_batch_from_store(unsigned int *batch);
bool has_stored_data();

#endif