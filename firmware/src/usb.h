/**
 * @file  usb.h
 * @brief Headers and definitions for usb module
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
#ifndef USB_H
#define USB_H

#define USE_CMSIS

void usb_init(void);
void usb_task(void);

enum USB_INTERFACES
{
	TUD_ITF_CDC = 0,
	TUD_ITF_CDC_DATA,
	TUD_ITF_LOG,
	TUD_ITF_LOG_DATA,
#ifdef USE_CMSIS
	TUD_ITF_CMSIS
#endif
};

#endif
