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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "xdg_shell.h"

static const int16_t IRC_MAX_MESSAGE_LENGTH = 512;

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

int32_t irc_handle_message(uint8_t *data, size_t size)
{
	if (size > 4 && strncmp(data, "PING", 4) == 0) {
		data[1] = 'O';
		if (write(irc_fd, data, size) < 0) {
			return 1;
		}
	}

	return 0;
}

int32_t irc_receive_message_loop()
{
	int32_t ret = 0;

	uint8_t message[IRC_MAX_MESSAGE_LENGTH];
	uint16_t message_position = 0;

	uint8_t buffer[4096];

	ssize_t bytes_read = read(irc_fd, buffer, sizeof(buffer));
	if (bytes_read <= 0) {
		return 1;
	}

	while (bytes_read > 0) {
		for (size_t i = 0; i < (size_t) bytes_read; ++i) {
			/* Copy a byte from the buffer to this message */
			message[message_position] = buffer[i];
			++message_position;

			/* Message too large */
			if (message_position == IRC_MAX_MESSAGE_LENGTH) {
				return 2;
			}

			/* End of message condition (ends in \r\n) */
			if (message_position > 2
			    && message[message_position - 2] == '\r'
			    && message[message_position - 1] == '\n') {
				ret = irc_handle_message(
					message, message_position);
				if (ret != 0) {
					return ret;
				}

				message_position = 0;
			}
		}

		bytes_read = read(irc_fd, buffer, sizeof(buffer));
	}

	if (bytes_read < 0) {
		return 1;
	}

	return ret;
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

	int32_t ret;

	ret = irc_nick(nick);
	if (ret != 0) {
		goto irc_main_error;
	}

	ret = irc_receive_message_loop();
	if (ret != 0) {
		goto irc_main_error;
	}

irc_main_error:
	close(irc_fd);
	return ret;
}
