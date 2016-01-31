#include "exit_code.h"

#include <pthread.h>

static pthread_mutex_t exit_code_lock = PTHREAD_MUTEX_INITIALIZER;
static uint8_t exit_code = 0;

void set_exit_code(uint8_t value)
{
	pthread_mutex_lock(&exit_code_lock);
	exit_code = value;	
	pthread_mutex_unlock(&exit_code_lock);
}

bool is_exiting()
{
	bool is_exit_code_set = false;
	pthread_mutex_lock(&exit_code_lock);
	if (exit_code != 0) {
		is_exit_code_set = true;
	}
	pthread_mutex_unlock(&exit_code_lock);
	return is_exit_code_set;
}

uint8_t get_exit_code() {
	return exit_code;
}
