/**
 * @file  cmsis.c
 * @brief This module contains a CMSIS compatible probe (USB interface and DAP)
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
#include "cmsis.h"
#include "usb.h"

/**
 * @brief Initialize the "cmsis" module
 *
 * This function initialize the cmsis module and configure IOs of the debug
 * port for SWD signals. For this module to work properly, this function must
 * be called before any other cmsis functions.
 */
void cmsis_init(void)
{
	log_puts("cmsis_init()\r\n");
}

/**
 * @brief TinyUSB class driver init
 *
 * This function is called by TinyUSB stack when the cmsis class driver is
 * registered into the list of custom drivers (see usb.c for descriptors)
 */
void cmsis_usb_init(void)
{
	log_puts("cmsis_usb_init()\r\n");
}

/**
 * @brief TinyUSB class driver "open" function
 *
 * This function is called by TinyUSB during the SET_CONFIGURATION step of the
 * enumeration when trying to find a valid class driver for each available
 * interfaces. If a cmsis interface is defined, this function will match.
 */
uint16_t cmsis_usb_open(uint8_t rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len)
{
	uint16_t drv_len;

	(void)rhport;

	log_puts("cmsis_usb_open()\r\n");

	if (itf_desc->bInterfaceNumber == USB_CMSIS_IF)
	{
		drv_len = sizeof(tusb_desc_interface_t);
		drv_len += 14;
		log_puts("CMSIS: Found\r\n");
	}
	else
		drv_len = 0;

	if (drv_len > max_len)
	{
		log_puts("CMSIS: Error into usb_open() : max_len\r\n");
		return(0);
	}
	
	return(drv_len);
}

/**
 * @brief TinyUSB class driver "reset" function
 *
 * This function is called by TinyUSB when a class driver is no more used
 * and can be revert to a "reset" state.
 */
void cmsis_usb_reset(uint8_t rhport)
{
	log_puts("cmsis_usb_reset()\r\n");
}

/**
 * @brief TinyUSB class driver "control transfer" function
 *
 * This function is called by TinyUSB when a control transfer is requested on
 * an interface attached to the cmsis class driver.
 */
bool cmsis_usb_ctl(uint8_t rhport, uint8_t stage, tusb_control_request_t const* req)
{
	log_puts("cmsis_usb_ctl()\r\n");
}

/**
 * @brief TinyUSB class driver "transfer" function
 *
 * This function is called by TinyUSB when a transfer is made on an endpoint
 * managed by cmsis class driver.
 */
bool cmsis_usb_xfer(uint8_t rhport, uint8_t ep, xfer_result_t result, uint32_t xferred_bytes)
{
	log_puts("cmsis_usb_xfer()\r\n");
}
/* EOF */
