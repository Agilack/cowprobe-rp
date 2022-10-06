/**
 * @file  cmsis.h
 * @brief Headers and definitions for the CMSIS module (USB and DAP)
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
#ifndef CMSIS_H
#define CMSIS_H
#include "pico/stdlib.h"
#include <device/usbd_pvt.h>

/* Macro used to insert a CMSIS interface into a usb config descriptor */
#define TUD_CMSIS_DESCRIPTOR(itf, str, ep_out, ep_in, ep_size) \
	9, TUSB_DESC_INTERFACE, itf, 0, 2, TUSB_CLASS_VENDOR_SPECIFIC, 0, 0, str, \
	/* EP_OUT must be before EP_IN for openocd */                             \
	7, TUSB_DESC_ENDPOINT, ep_out, TUSB_XFER_BULK, U16_TO_U8S_LE(ep_size), 1, \
	7, TUSB_DESC_ENDPOINT, ep_in,  TUSB_XFER_BULK, U16_TO_U8S_LE(ep_size), 1

typedef struct s_cmsis_pkt
{
	uint8_t  *buffer;
	uint16_t  len;
} cmsis_pkt;

void cmsis_init (void);

/* TinyUSB class driver functions */
void     cmsis_usb_init (void);
uint16_t cmsis_usb_open (uint8_t rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len);
void     cmsis_usb_reset(uint8_t rhport);
bool     cmsis_usb_ctl  (uint8_t rhport, uint8_t stage, tusb_control_request_t const* req);
bool     cmsis_usb_xfer (uint8_t rhport, uint8_t ep, xfer_result_t result, uint32_t xferred_bytes);

#endif
