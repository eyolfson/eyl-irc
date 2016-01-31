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

#include <netdb.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "exit_code.h"
#include "xdg_shell.h"

static const int16_t IRC_MESSAGE_LENGTH_MAX = 512;
static int32_t irc_fd;

int irc_nick(char *nick)
{
	char buffer[512];
	size_t buffer_limit = sizeof(buffer) -1;

	if (snprintf(buffer, buffer_limit, "USER %s 0 0 :%s\r\n", nick, nick)
	    == sizeof(buffer)) {
		return -1;
	}
	if (write(irc_fd, buffer, strlen(buffer)) == -1) {
		return 1;
	}

	if (snprintf(buffer, buffer_limit, "NICK %s\r\n", nick)
	    == sizeof(buffer)) {
		return 1;
	}
	if (write(irc_fd, buffer, strlen(buffer)) == -1) {
		return 1;
	}

	return 0;
}

int32_t irc_connect(const char *host)
{
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
	};
	struct addrinfo *res;
	if (getaddrinfo(host, "6667", &hints, &res) != 0) {
		return -1;
	}

	int32_t fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (fd < 0) {
		freeaddrinfo(res);
		return fd;
	}
	if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
		close(fd);
		freeaddrinfo(res);
		return -1;
	}
	freeaddrinfo(res);
	return fd;
}

int32_t irc_handle_message(uint8_t *data, uint16_t length)
{
	if (length >= 4 && strncmp(data, "PING", 4) == 0) {
		data[1] = 'O';
		if (write(irc_fd, data, length) < 0) {
			return 1;
		}
	}

	printf("Got message length %d\n", length);

	return 0;
}

int32_t irc_receive_messages()
{
	uint8_t message[IRC_MESSAGE_LENGTH_MAX];
	uint16_t length = 0;

	uint8_t buffer[4096];

	ssize_t bytes_read = read(irc_fd, buffer, sizeof(buffer));
	if (bytes_read <= 0) {
		return 1;
	}

	while (bytes_read > 0) {
		for (size_t i = 0; i < (size_t) bytes_read; ++i) {
			/* Copy a byte from the buffer to this message */
			message[length] = buffer[i];
			++length;

			/* Message too large */
			if (length > IRC_MESSAGE_LENGTH_MAX) {
				return 2;
			}

			/* End of message condition (ends in \r\n) */
			if (length >= 2
			    && message[length - 2] == '\r'
			    && message[length - 1] == '\n') {
				uint8_t ret = irc_handle_message(
					message, length);
				if (ret != 0) {
					return ret;
				}

				/* Reset message length for next message */
				length = 0;
			}
		}

		if (is_exiting()) {
			return 0;
		}

		bytes_read = read(irc_fd, buffer, sizeof(buffer));
	}

	if (bytes_read < 0) {
		return 1;
	}

	return 0;
}

void *irc_start(void *arg)
{
	uint8_t ret = irc_receive_messages();
	if (ret != 0) {
		set_exit_code(ret);
	}
	return NULL;
}

void *wayland_start(void *);

uint8_t run_threads()
{
	pthread_t irc_thread;
	pthread_t wayland_thread;

	if (pthread_create(&irc_thread, NULL, &irc_start, NULL) != 0) {
		return 1;
	}
	if (pthread_create(&wayland_thread, NULL, &wayland_start, NULL) != 0) {
		return 1;
	}

	if (pthread_join(irc_thread, NULL) != 0) {
		return 1;
	}

	if (pthread_join(wayland_thread, NULL) != 0) {
		return 1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--version") == 0) {
			printf("IRC Client 0.0.1-development\n");
			return 0;
		}
	}

	if (argc != 3) {
		return 1;
	}
	char *host = argv[1];
	char *nick = argv[2];

	irc_fd = irc_connect(host);
	if (irc_fd < 0) {
		return 1;
	}
	uint32_t ret = irc_nick(nick);
	if (ret != 0) {
		goto irc_main_error;
	}

	ret = run_threads();

irc_main_error:
	close(irc_fd);

	if (ret != 0) {
		return ret;
	}
	else {
		return get_exit_code();
	}
}
