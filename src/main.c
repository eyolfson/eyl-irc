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
#include "irc.h"
#include "wayland.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>

void run_wayland_thread()
{
	pthread_t wayland_thread;

	if (pthread_create(&wayland_thread, NULL, &wayland_start, NULL) != 0) {
		set_exit_code(1);
		return;
	}

	/* Errors are not possible on this pthread_join */
	pthread_join(wayland_thread, NULL);
}

void run_threads()
{
	pthread_t irc_thread;

	if (pthread_create(&irc_thread, NULL, &irc_start, NULL) != 0) {
		set_exit_code(1);
		return;
	}

	run_wayland_thread();

	/* Errors are not possible on this pthread_join */
	pthread_join(irc_thread, NULL);

	irc_finish();
}

bool print_version(int argc, char **argv)
{
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--version") == 0) {
			printf("IRC Client 0.0.1-development\n");
			return true;
		}
	}
	return false;
}

int main(int argc, char **argv)
{
	if (print_version(argc, argv)) {
		return 0;
	}

	if (argc != 3) {
		return 1;
	}
	char *host = argv[1];
	char *nick = argv[2];

	irc_connect(host, nick);

	run_threads();

	return get_exit_code();
}
