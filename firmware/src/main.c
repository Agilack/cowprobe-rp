/**
 * @file  main.c
 * @brief Entry point and main function of the firmware
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
#include "pico/stdlib.h"
#include "ios.h"
#include "log.h"
#include "serial.h"
#include "usb.h"

/**
 * @brief Entry point of C code
 *
 * This function is called by low-level reset handler. This can be after a
 * power-on, an hardware or software reboot, and after some critical errors.
 * This function should never returns.
 */
int main()
{
	/* Initialize all modules */
	ios_init();
	log_init();
	serial_init();
	usb_init();

	while(1)
	{
		usb_task();
	}
}
/* EOF */
