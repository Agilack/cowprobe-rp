##
 # @file  Makefile
 # @brief Script to compile cmsis-usbmon tool using "make" command
 #
 # @author Saint-Genest Gwenael <gwen@cowlab.fr>
 # @copyright Cowlab (c) 2022
 #
 # @page License
 # This software is free software: you can redistribute it and/or modify it
 # under the terms of the GNU General Public License version 3 as published
 # by the Free Software Foundation. You should have received a copy of the
 # GNU General Public License along with this program, see LICENSE.md file
 # for more details.
 # This program is distributed WITHOUT ANY WARRANTY.
##
APP=cmsis-usbmon

CFLAGS = -O2 -Wall -Wextra
CFLAGS += -g

all: $(APP)

$(APP): main.o
	$(CC) $(CFLAGS) -o $(APP) main.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

clean:
	rm -f $(APP)
	rm -f *.o
	rm -f *~
