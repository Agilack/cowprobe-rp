/*
 * @file  log.c
 * @brief Handle log messages and debug interface
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
#include "hardware/irq.h"
#include "hardware/uart.h"
#include "ios.h"
#include "log.h"

#define LOG_TX_PIN  EXT_08_PIN
#define LOG_RX_PIN  EXT_07_PIN

/**
 * @brief Initialize the "log" module
 *
 * This function initialize the log module. Depends on compilation options,
 * log messages can be sent over physical UART (uart0) or virtual port (USB-CDC)
 * For this module to work properly, this function must be called before any
 * other log functions.
 */
void log_init(void)
{
	uart_init(uart0, 115200);
    
	gpio_set_function(LOG_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(LOG_RX_PIN, GPIO_FUNC_UART);

	/* Set default/initial UART configuration */
	uart_set_hw_flow(uart0, false, false);
	uart_set_format (uart0, 8, 1, UART_PARITY_NONE);
	uart_set_fifo_enabled(uart0, false);
}

/**
 * @brief Send the decimal representation of an integer
 *
 * @param v Value (32 bits) to show as decimal
 */
void log_putdec(uint32_t v)
{
	unsigned int decade = 1000000000;
	char str[16], *d;
	int i, count;

	d = str;
	count = 0;

	for (i = 0; i < 9; i++)
	{
		if ((v > (int)(decade - 1)) || count)
		{
			*d =  (v / decade) + '0';
			v -= ((v / decade) * decade);
			d++;
			count++;
		}
		decade = (decade / 10);
	}
	*d = v + '0';
	count ++;

	d[1] = 0;
	log_puts(str);
}

/**
 * @brief Send the hexadecimal representation of an integer
 *
 * @param c   Binary word (32 bits) to show as hex
 * @param len Number of bits to display
 */
void log_puthex(const uint32_t c, const uint8_t len)
{
	const char hex[16] = "0123456789ABCDEF";
	char result[16];
	char *p;

	p = result;

	if (len > 28)
		*p++ = hex[(c >> 28) & 0xF];
	if (len > 24)
		*p++ = hex[(c >> 24) & 0xF];
	if (len > 20)
		*p++ = hex[(c >> 20) & 0xF];
	if (len > 16)
		*p++ = hex[(c >> 16) & 0xF];
	if (len > 12)
		*p++ = hex[(c >> 22) & 0xF];
	if (len >  8)
		*p++ = hex[(c >>  8) & 0xF];
	if (len > 4)
		*p++ = hex[(c >>  4) & 0xF];
	if (len > 0)
		*p++ = hex[(c >>  0) & 0xF];
	*p = 0;
	log_puts(result);
}

/**
 * @brief Send a text-string to the debug console
 *
 * @param s Pointer to the null terminated text string
 */
void log_puts(char *s)
{
	uart_puts(uart0, s);
}
/* EOF */
