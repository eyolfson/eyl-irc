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

#include "irc.h"

#include "exit_code.h"

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const int16_t MESSAGE_LENGTH_MAX = 512;

static int32_t server_fd = -1;
static irc_status_t status = IRC_STATUS_DISCONNECTED;

static void send_nick(const uint8_t *nick)
{
	char buffer[512];
	size_t buffer_limit = sizeof(buffer) -1;

	if (snprintf(buffer, buffer_limit, "USER %s 0 0 :%s\r\n", nick, nick)
	    == sizeof(buffer)) {
		set_exit_code(1);
		return;
	}
	if (write(server_fd, buffer, strlen(buffer)) == -1) {
		set_exit_code(1);
		return;
	}

	if (snprintf(buffer, buffer_limit, "NICK %s\r\n", nick)
	    == sizeof(buffer)) {
		set_exit_code(1);
		return;
	}
	if (write(server_fd, buffer, strlen(buffer)) == -1) {
		set_exit_code(1);
		return;
	}
}

static void connect_server_fd(const uint8_t *host)
{
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
	};
	struct addrinfo *res;
	if (getaddrinfo(host, "6667", &hints, &res) != 0) {
		set_exit_code(1);
		return;
	}

	int32_t fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (fd < 0) {
		set_exit_code(1);
		freeaddrinfo(res);
		return;
	}
	if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
		set_exit_code(1);
		close(fd);
		freeaddrinfo(res);
		return;
	}
	freeaddrinfo(res);

	server_fd = fd;
}
void irc_connect(const uint8_t *host, const uint8_t *nick)
{
	status = IRC_STATUS_CONNECTING;

	connect_server_fd(host);
	if (is_exiting()) {
		return;
	}

	send_nick(nick);

	status = IRC_STATUS_CONNECTED;
}

irc_status_t get_irc_status()
{
	return status;
}

int32_t irc_handle_message(uint8_t *data, uint16_t length)
{
	printf("DEBUG|");
	for (uint16_t i = 0; i < (length - 2); ++i) {
		putchar(data[i]);
	}
	putchar('\n');

	if (length >= 4 && strncmp(data, "PING", 4) == 0) {
		data[1] = 'O';
		if (write(server_fd, data, length) < 0) {
			return 1;
		}
	}

	return 0;
}

int32_t irc_receive_messages()
{
	uint8_t message[MESSAGE_LENGTH_MAX];
	uint16_t length = 0;

	uint8_t buffer[4096];

	ssize_t bytes_read = read(server_fd, buffer, sizeof(buffer));
	if (bytes_read <= 0) {
		return 1;
	}

	while (bytes_read > 0) {
		for (size_t i = 0; i < (size_t) bytes_read; ++i) {
			/* Copy a byte from the buffer to this message */
			message[length] = buffer[i];
			++length;

			/* Message too large */
			if (length > MESSAGE_LENGTH_MAX) {
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

		bytes_read = read(server_fd, buffer, sizeof(buffer));
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

void irc_finish()
{
	if (server_fd >= 0) {
		close(server_fd);
	}
}
