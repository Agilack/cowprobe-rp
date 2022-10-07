/**
 * @file  test.h
 * @brief Global headers and definitions used by all tests
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
#ifndef TEST_H
#define TEST_H
#include <libusb-1.0/libusb.h>

typedef struct cmsis_env_s
{
	libusb_device_handle *dev;
	unsigned char tx[1024];
	int tx_len;
	unsigned char rx[1024];
	int rx_len;
} cmsis_env;

void color(int x);
int cmsis_txrx(cmsis_env *env);

int err_header (cmsis_env *env, int n);
int err_request(void);

#endif
