# Copyright 2016 Jonathan Eyolfson
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program. If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.1.3)

project(IRC_CLIENT C)

find_package(PkgConfig)

pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
pkg_check_modules(WAYLAND_SCANNER REQUIRED wayland-scanner)

find_package(Threads)

include_directories(
	${CMAKE_BINARY_DIR}
	${CAIRO_INCLUDE_DIRS}
	${WAYLAND_CLIENT_INCLUDE_DIRS}
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wextra)

add_custom_command(
	OUTPUT ${CMAKE_BINARY_DIR}/xdg_shell.h
	COMMAND wayland-scanner
	ARGS client-header
	     ${CMAKE_SOURCE_DIR}/../wayland-protocol/xdg-shell-unstable-v5.xml
	     ${CMAKE_BINARY_DIR}/xdg_shell.h
)
add_custom_command(
	OUTPUT ${CMAKE_BINARY_DIR}/xdg_shell.c
	COMMAND wayland-scanner
	ARGS code
	     ${CMAKE_SOURCE_DIR}/../wayland-protocol/xdg-shell-unstable-v5.xml
	     ${CMAKE_BINARY_DIR}/xdg_shell.c
)

add_executable(irc-client
	main.c
	${CMAKE_BINARY_DIR}/xdg_shell.c
	${CMAKE_BINARY_DIR}/xdg_shell.h
)
target_link_libraries(irc-client
	${CMAKE_THREAD_LIBS_INIT}
	${CAIRO_LIBRARIES}
	${WAYLAND_CLIENT_LIBRARIES}
)
