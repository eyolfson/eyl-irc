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

#pragma once

#include <stdint.h>

#ifdef __cpluscplus
extern "C" {
#endif

typedef enum {
	IRC_STATUS_DISCONNECTED,
	IRC_STATUS_CONNECTING,
	IRC_STATUS_CONNECTED,
} irc_status_t;

void *irc_start(void *arg);
void irc_finish();

irc_status_t get_irc_status();

void irc_connect(const uint8_t *host, const uint8_t *nick);

#ifdef __cpluscplus
}
#endif
