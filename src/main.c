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
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
		return -1;
	}
	if (write(fd, buffer, strlen(buffer)) == -1) {
		return 1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	printf("IRC Client 0.0.1-development\n");

	if (argc != 3) {
		return 1;
	}

	char *host = argv[1];
	char *nick = argv[2];

	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
	};
	struct addrinfo *res;

	getaddrinfo(host, "6667", &hints, &res);

	printf("Attempting to connect to '%s' as '%s'\n", host, nick);

	int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (fd == -1) {
		return 1;
	}
	if (connect(fd, res->ai_addr, res->ai_addrlen) == -1) {
		return 1;
	}

	char buffer[4096];

	if (irc_nick(fd, nick) != 0) {
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

		bytes_read = read(fd, buffer, sizeof(buffer));
	}

	if (bytes_read == -1) {
		return 1;
	}

	return 0;
}
