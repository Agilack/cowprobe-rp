##
 # @file  dap_info.c
 # @brief Collection of functions to test all DAP_Info commands
 #
 # @author Saint-Genest Gwenael <gwen@cowlab.fr>
 # @copyright Cowlab (c) 2022
 #
 # @page License
 # This firmware is free software: you can redistribute it and/or modify it
 # under the terms of the GNU General Public License version 3 as published
 # by the Free Software Foundation. You should have received a copy of the
 # GNU General Public License along with this program, see LICENSE.md file
 # for more details.
 # This program is distributed WITHOUT ANY WARRANTY.
##
APP=ut_cmsis
CFLAGS = -g -Wall -Wextra
LDFLAGS = -lusb-1.0

all:
	cc $(CFLAGS) -c main.c        -o main.o
	cc $(CFLAGS) -c dap_general.c -o dap_general.o
	cc $(CFLAGS) -c dap_info.c    -o dap_info.o
	cc $(CFLAGS) -c swd.c         -o swd.o
	cc -o $(APP) $(LDFLAGS) main.o dap_general.o dap_info.o swd.o

clean:
	rm -f $(APP) *.o *~