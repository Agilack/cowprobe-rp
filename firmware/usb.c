/**
 * @file  usb.c
 * @brief Handle communication with USB interface
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
#include <tusb.h>
#include <device/usbd_pvt.h>
#include "cmsis.h"
#include "serial.h"
#include "log.h"
#include "usb.h"

static void cdc_task(void);

/**
 * @brief Initialize the "USB" module
 *
 * This function initialize USB module and TinyUSB library. The initialization
 * of USB bus if not finished after calling this function, some steps remains
 * and will be made by tinyusb itself into periodic call of usb stack. This
 * function must be called before any other USB functions.
 */
void usb_init(void)
{
	log_puts("USB initialization\r\n");
	tusb_init();
}

/**
 * @brief Process periodic stuff of USB stack
 *
 * This function must be called periodically (typically from main loop of a
 * thread) to process USB events and other stuff.
 */
void usb_task(void)
{
	/* Call TinyUSB stack to process events */
	tud_task();
	cdc_task();
}

/* -------------------------------------------------------------------------- */
/* --                            CDC interfaces                            -- */
/* -------------------------------------------------------------------------- */

/**
 * @brief Process periodic events of CDC interface
 *
 * When the main CDC interface is opened by a client (interface 0) this function
 * copy to CDC data received from UART ... and copy to UART data received from
 * CDC.
 */
static void cdc_task(void)
{
	uint8_t buffer[64];
	int count;

	/* If bytes received from UART (direction uart -> CDC) */
	if (serial_rx_avail() > 0)
	{
		int i;

		/* Get data from serial module ... */
		count = serial_read(buffer, 64);
		/* ... and send them to CDC */
		for (i = 0; i < count; i++)
			tud_cdc_n_write_char(0, buffer[i]);
		tud_cdc_n_write_flush(0);
	}
}

/**
 * @brief TinuUSB callback: CDC line coding configuration has been modified
 *
 * This function is called by TinyUSB when a new configuration has been defined
 * for a CDC virtual communication port. After decoding, this new config is set
 * to the UART interface.
 *
 * @param itf Identifier of the modified interface
 * @param p_line_coding Pointer to a structure with new config
 */
void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding)
{
	if (itf == 0)
	{
		serial_set_format(p_line_coding->data_bits,
		                  p_line_coding->stop_bits,
		                  p_line_coding->parity,
		                  p_line_coding->bit_rate);
	}
}

/**
 * @brief TinuUSB callback: Data have been received from CDC
 *
 * @param itf Identifier of the CDC interface
 */
void tud_cdc_rx_cb(uint8_t itf)
{
	uint8_t buffer[64];
	int count;

	if (itf == 0)
	{
		count = tud_cdc_n_read(0, buffer, 64);
		serial_write(buffer, count);
	}
}

/* -------------------------------------------------------------------------- */
/* --                                                                      -- */
/* --                       TinyUSB and  descriptors                       -- */
/* --                                                                      -- */
/* -------------------------------------------------------------------------- */

#define USBD_VID 0x2E8A /* Raspberry Pi */
#define _PID_MAP(itf, n) ((CFG_TUD_##itf) << (n))
#define USBD_PID (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                  _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) | _PID_MAP(ECM_RNDIS, 5) | _PID_MAP(NCM, 5))
/* IDs of strings */
#define USBD_STR_MANUF   0x01
#define USBD_STR_PRODUCT 0x02
#define USBD_STR_SERIAL  0x03

static const uint8_t usb_desc_config[] =
{
	TUD_CONFIG_DESCRIPTOR(1, 5, 0, (TUD_CONFIG_DESC_LEN + CFG_TUD_CDC * TUD_CDC_DESC_LEN + 23), 0x00, 100),
	TUD_CDC_DESCRIPTOR(0, 4, 0x81, 8, 0x02, 0x83, 64),
	TUD_CDC_DESCRIPTOR(2, 4, 0x84, 8, 0x05, 0x86, 64),
	/* CMSIS v2 Descriptor */
	TUD_CMSIS_DESCRIPTOR(USB_CMSIS_IF, 0, 0x07, 0x88, 64),
};

const uint8_t  str_lang[]    = {0x04,0x03,0x09,0x04};
const uint16_t str_manuf[]   = {0x030E, 'C','o','w','l','a','b'};
const uint16_t str_product[] = {0x0326, 'C','o','w','p','r','o','b','e', ' ', 'C','M','S','I','S','-','D','A','P'};
const uint16_t str_serial[]  = {0x030A, '0','1','2','3'};
const uint16_t str_extra[]   = {0x030A, 'p','l','o','p'};

static const tusb_desc_device_t usbd_desc_device = {
	.bLength = sizeof(tusb_desc_device_t),
	.bDescriptorType = TUSB_DESC_DEVICE,
	.bcdUSB          = 0x0200,
	.bDeviceClass    = TUSB_CLASS_MISC,
	.bDeviceSubClass = MISC_SUBCLASS_COMMON,
	.bDeviceProtocol = MISC_PROTOCOL_IAD,
	.bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
	.idVendor  = USBD_VID,
	.idProduct = USBD_PID,
	.bcdDevice = 0x0100,
	.iManufacturer = USBD_STR_MANUF,
	.iProduct      = USBD_STR_PRODUCT,
	.iSerialNumber = USBD_STR_SERIAL,
	.bNumConfigurations = 1,
};

static const usbd_class_driver_t usbd_custom_class_drv = {
#if CFG_TUSB_DEBUG >= 2
	.name = "cmsis-dap",
#endif
	.init            = cmsis_usb_init,
	.open            = cmsis_usb_open,
	.reset           = cmsis_usb_reset,
	.control_xfer_cb = cmsis_usb_ctl,
	.xfer_cb         = cmsis_usb_xfer,
	.sof             = 0,
};

/**
 * @brief Callback function for providing the USB configuration descriptor.
 *
 * @param index Index of the requested configuration descriptor.
 * @return Pointer to the USB configuration descriptor.
 */
const uint8_t *tud_descriptor_configuration_cb(uint8_t index)
{
	if (index >= 1)
		return(NULL);

	return(usb_desc_config); 
}

/**
 * @brief Callback function for providing the USB device descriptor.
 *
 * @return Pointer to the USB device descriptor.
 */
const uint8_t *tud_descriptor_device_cb(void)
{
	return (const uint8_t *)&usbd_desc_device;
}

usbd_class_driver_t const *usbd_app_driver_get_cb(uint8_t *driver_count)
{
	int count = 0;

	log_puts("usbd_app_driver_get_cb()\r\n");

	count ++; // CMSIS

	if (driver_count != 0)
	{
		*driver_count = count;
	}
	return(&usbd_custom_class_drv);
}

/**
 * @brief Callback function for providing USB string descriptors.
 *
 * @param index  Index of the requested string descriptor.
 * @param langid Language ID for the string.
 * @return Pointer to the requested string descriptor.
 */
const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
	if (index == 0)
		return ((uint16_t *)str_lang);
	else if (index == 1)
		return(str_manuf);
	else if (index == 2)
		return(str_product);
	else if (index == 3)
		return(str_serial);
	else
		return(str_extra);
}
/* EOF */
