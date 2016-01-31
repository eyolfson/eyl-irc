/*
 * Copyright 2016 Jonathan Eyolfson
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

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
