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

#ifdef USE_CMSIS

#define RX_SIZE 256

static uint8_t ep_in_n;
static uint8_t ep_out_n;
static uint8_t rx_buffer[RX_SIZE];
static uint8_t tx_buffer[256];

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
	ep_in_n  = 0;
	ep_out_n = 0;
}

/* -------------------------------------------------------------------------- */
/* --                             DAP commands                             -- */
/* -------------------------------------------------------------------------- */

static void dap_str(cmsis_pkt *pkt, char *str);

static char str_serial[]  = "12345678";
static char str_version[] = "1.0";

void dap_recv(uint8_t *rx, uint16_t len)
{
	cmsis_pkt req, rsp;
	int result = 1;
	int i;

	req.buffer = rx;
	req.len    = len;
	rsp.buffer = tx_buffer;
	rsp.len    = 0;


	/* DAP_Info */
	if (req.buffer[0] == 0x00)
	{
		rsp.buffer[0] = 0;

		/* Capabilities of the Debug Unit */
		if (req.buffer[1] == 0xF0)
		{
			log_puts("CMSIS: Get Capabilities\r\n");
			rsp.buffer[1] = 1;
			rsp.buffer[2] = (1 << 0);
			rsp.len = 3;
			result = 0;
		}
		/* Serial Number (string) */
		else if (req.buffer[1] == 0x03)
		{
			log_puts("CMSIS: Get serial number\r\n");
			dap_str(&rsp, str_serial);
			result = 0;
		}
		/* CMSIS-DAP Protocol Version */
		else if (req.buffer[1] == 0x04)
		{
			log_puts("CMSIS: Get DAP protocol version\r\n");
			dap_str(&rsp, str_version);
			result = 0;
		}
	}
	/* DAP_Connect */
	else if (req.buffer[0] == 0x02)
	{
		rsp.buffer[0] = 2;
		log_puts("CMSIS: Connect\r\n");
		/* If request port is SWD (or Default) */
		if ((req.buffer[1] == 1) || (req.buffer[1] == 0))
			rsp.buffer[1] = 0x01;
		/* For all other ports, Initialization Failed */
		else
			rsp.buffer[1] = 0x00;
		rsp.len = 2;
		result = 0;
	}

	if (result == 0)
	{
		if (rsp.len <= 0)
		{
			tx_buffer[0] = req.buffer[0]; // Copy command ID
			tx_buffer[1] = 0xFF;          // DAP_ERROR
			rsp.len = 2;
		}
		usbd_edpt_xfer(0, ep_in_n, tx_buffer, rsp.len);
	}
	else
	{
		log_puts("CMSIS: dap_recv() :\r\n");
		for (i = 0; i < len; i++)
		{
			log_puthex(rx[i], 8);
			log_puts(" ");
		}
		log_puts("\r\n");
	}

}

static void dap_str(cmsis_pkt *pkt, char *str)
{
	int len;

	if (str == 0)
	{
		pkt->buffer[1] = 0;
		pkt->len = 2;
		return;
	}

	len = strlen(str);
	/* Insert header */
	pkt->buffer[1] = len + 1;
	/* Copy string into response packet */
	strcpy((char *)(pkt->buffer + 2), str);
	pkt->buffer[2+len] = 0; // Add a nul char to finish string
	pkt->len = (2 + len);
}

/* -------------------------------------------------------------------------- */
/* --                         TinyUSB class driver                         -- */
/* -------------------------------------------------------------------------- */

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
	const tusb_desc_endpoint_t *p_desc_ep;
	uint8_t const *p_desc;
	uint16_t drv_len;
	uint8_t ep_n;

	(void)rhport;

	log_puts("cmsis_usb_open()\r\n");

	if (itf_desc->bInterfaceNumber != TUD_ITF_CMSIS)
		return(0);

	drv_len = sizeof(tusb_desc_interface_t);

	/* Search next descriptor (should be endpoint OUT) */
	p_desc = tu_desc_next(itf_desc);
	if (tu_desc_type(p_desc) == TUSB_DESC_ENDPOINT)
	{
		p_desc_ep = (const tusb_desc_endpoint_t *)p_desc;
		ep_n = p_desc_ep->bEndpointAddress;
		if (usbd_edpt_open(rhport, p_desc_ep) == 0)
		{
			log_puts("CMSIS: failed to activate endpoint.\r\n");
			goto err;
		}
		if (usbd_edpt_claim(rhport, ep_n) == 0)
		{
			log_puts("CMSIS: Open failed #1\r\n");
			goto err;
		}
		ep_out_n = ep_n;
		usbd_edpt_xfer(rhport, ep_out_n, rx_buffer, RX_SIZE);
		drv_len += tu_desc_len(p_desc);
	}
	else
		goto err;

	/* Search next descriptor (should be endpoint IN) */
	p_desc = tu_desc_next(p_desc);
	if (tu_desc_type(p_desc) == TUSB_DESC_ENDPOINT)
	{
		p_desc_ep = (const tusb_desc_endpoint_t *)p_desc;
		if (usbd_edpt_open(rhport, p_desc_ep) == 0)
		{
			log_puts("CMSIS: failed to activate endpoint.\r\n");
			return(0);
		}
		ep_in_n = p_desc_ep->bEndpointAddress;
		drv_len += tu_desc_len(p_desc);
	}
	else
		goto err;

	log_puts("CMSIS: Found\r\n");

	if (drv_len > max_len)
	{
		log_puts("CMSIS: Error into usb_open() : max_len\r\n");
		return(0);
	}

	return(drv_len);
err:
	ep_in_n  = 0;
	ep_out_n = 0;
	return(0);
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
	int i;

#ifdef DBG_XFER
	log_puts("cmsis_usb_xfer()");
	log_puts(" ep=");     log_puthex(ep, 8);
	log_puts(" result="); log_puthex(result, 32);
	log_puts(" len=");    log_puthex(xferred_bytes, 16);
	log_puts("\r\n");
#endif

	if (ep == ep_out_n)
	{
		/* Call DAP to process received command */
		dap_recv(rx_buffer, xferred_bytes);

		/* Prepare endpoint for next transfer */
		usbd_edpt_xfer(rhport, ep_out_n, rx_buffer, 256);
	}
	else if (ep == ep_in_n)
	{
		/* Do nothing :) */
	}
	else
		/* Unknown endpoint ?! */
		return(0);

	return(1);
}
#endif
/* EOF */
