/**
 * @file  serial.h
 * @brief Headers and definitions for serial module
 *
 * @author Saint-Genest Gwenael <gwen@cowlab.fr>
 * @copyright Cowlab (c) 2022
 *
 * @page License
 * This firmware is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as published
 * by the Free Software Foundation. You should have received a copy of the
 * GNU General Public License along with this program, see LICENSE.md file
 * for more details.
 * This program is distributed WITHOUT ANY WARRANTY.
 */
#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_RX_SZ 1024
#define SERIAL_TX_SZ 128

void serial_init (void);
void serial_write(unsigned char *data, int len);
int  serial_read (char *buffer, int len);
int  serial_rx_avail(void);
void serial_set_format(int bits, int stop, int parity, int speed);

#endif
