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

int irc_nick(int fd, char *nick)
{
	char buffer[512];
	size_t buffer_limit = sizeof(buffer) -1;

	if (snprintf(buffer, buffer_limit, "USER %s 0 0 :%s\r\n", nick, nick)
	    == sizeof(buffer)) {
		return -1;
	}
	if (write(fd, buffer, strlen(buffer)) == -1) {
		return 1;
	}

	if (snprintf(buffer, buffer_limit, "NICK %s\r\n", nick)
	    == sizeof(buffer)) {
		return 1;
	}
	if (write(fd, buffer, strlen(buffer)) == -1) {
		return 1;
	}

	return 0;
}

int join_channel(int fd, char *channel)
{
	char buffer[512];
	size_t buffer_limit = sizeof(buffer) -1;

	if (snprintf(buffer, buffer_limit, "JOIN %s\r\n", channel)
	    == sizeof(buffer)) {
		return -1;
	}
	if (write(fd, buffer, strlen(buffer)) == -1) {
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

	getaddrinfo(host, "6667", &hints, &res);
	int32_t fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (fd < 0) {
		return -1;
	}
	if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
		close(fd);
		return -1;
	}
	return fd;
}

int main(int argc, char **argv)
{
	printf("IRC Client 0.0.1-development\n");

	if (argc != 4) {
		return 1;
	}

	char *host = argv[1];
	char *nick = argv[2];
	char *channel = argv[3];

	printf("Attempting to connect to '%s' as '%s'\n", host, nick);
	int32_t fd = irc_connect(host);
	if (fd < 0) {
		return 1;
	}

	char buffer[4096];

	if (irc_nick(fd, nick) != 0) {
		return 1;
	}

	if (join_channel(fd, channel) != 0) {
		return 1;
	}

	ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
	if (bytes_read <= 0) {
		return 1;
	}

	while (bytes_read > 0) {
		for (size_t i = 0; i < (size_t) bytes_read; ++i) {
			putchar(buffer[i]);
		}

		if (bytes_read > 4 && strncmp(buffer, "PING", 4) == 0) {
			buffer[1] = 'O';
			if (write(fd, buffer, bytes_read) == -1) {
				return 1;
			}
		}
		bytes_read = read(fd, buffer, sizeof(buffer));
	}

	if (bytes_read == -1) {
		return 1;
	}

	return 0;
}
