/**
 * @file  swd.c
 * @brief Implement SWD protocol
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
#include "swd.h"

#define DEBUG_SWD
#define WAIT_DELAY 80

#define PIN_SWDIO PORT_D1_PIN
#define PIN_SWCLK PORT_D2_PIN

swd_param swd_config;

static inline uint _parity(uint32_t value);

/**
 * @brief Process one SWD transfer on the bus
 *
 * @param req Identifier of the SWD request
 * @param value Value to read or write during transaction
 * @return integer Value of the ACK bits (0 for success)
 */
int swd_transfer(uint8_t req, uint32_t *value)
{
	uint32_t data;
	int ack = 0;
	int i;

#ifdef DEBUG_SWD
	// Sanity check
	if (swd_config.retry_count == 0)
	{
		log_puts("SWD: transfer error : retry count is nul\r\n");
		return(ack);
	}
	if (value == 0)
		log_puts("SWD: transfer warning, no value specified\r\n");

	log_puts("swd_transfer ");
	log_puthex(req, 8);
	log_puts("\r\n");
#endif

	for (i = 0; i < swd_config.retry_count; i++)
	{
		data  = ((req & 0x0F) << 1);
		data |= (_parity(data) << 5);
		data |= 0x81;
		swd_wr(data, 8);
		swd_turna(0);
		ack = swd_rd(3);

		// If acknowledge is OK
		if (ack == 1)
		{
			data = swd_rd(32);
			// Read parity bit
			if (swd_rd(1) != _parity(data))
				log_puts("DAP: Parity error\r\n");
			else if (value)
				*value = data;

			// Trn cycle to revert initial state
			swd_turna(1);
			// Request finished, no retry needed
			break;
		}
		else
		{
			log_puts("DAP: Transfer failed ! ACK=");
			log_puthex(ack, 8);
			log_puts("\r\n");
			break;
		}
	}
	return(ack);
}

/**
 * @brief Read bits from SWD port
 *
 * @param len Number of bit(s) to write
 * @return integer Value of the readed bits
 */
u32 swd_rd(uint len)
{
	u32  result = 0;
	uint bit, wait;
	uint i;

	for (i = 0 ; i < len ; i++)
	{
		/* Falling edge to SWD-CLK */
		ios_pin_set(PIN_SWCLK, 0);
		/* Wait a bit TODO: improve delay */
		for (wait = 0; wait < WAIT_DELAY; wait++)
			asm volatile("nop");

		bit = ios_pin(PIN_SWDIO);

		/* Rising edge to SWD-CLK */
		ios_pin_set(PIN_SWCLK, 1);
		/* Wait a bit TODO: improve delay */
		for (wait = 0; wait < WAIT_DELAY; wait++)
			asm volatile("nop");

		result |= (bit << i);
	}
	return(result);
}

/**
 * @brief Execute a bus turnaround to change SWD-IO direction
 *
 * @param dir Direction to set (0=IN , 1=OUT)
 */
void swd_turna(int dir)
{
	int wait;

	if (dir)
	{
		/* Falling edge to SWD-CLK */
		ios_pin_set(PIN_SWCLK, 0);
		/* Wait a bit TODO: improve delay */
		for (wait = 0; wait < WAIT_DELAY; wait++)
			asm volatile("nop");

		ios_pin_mode(PIN_SWDIO, IO_DIR_OUT);

		/* Rising edge to SWD-CLK */
		ios_pin_set(PIN_SWCLK, 1);
		/* Wait a bit TODO: improve delay */
		for (wait = 0; wait < WAIT_DELAY; wait++)
			asm volatile("nop");
	}
	else
	{
		ios_pin_mode(PIN_SWDIO, IO_DIR_IN);
		/* Falling edge to SWD-CLK */
		ios_pin_set(PIN_SWCLK, 0);
		/* Wait a bit TODO: improve delay */
		for (wait = 0; wait < WAIT_DELAY; wait++)
			asm volatile("nop");
		/* Rising edge to SWD-CLK */
		ios_pin_set(PIN_SWCLK, 1);
		/* Wait a bit TODO: improve delay */
		for (wait = 0; wait < WAIT_DELAY; wait++)
			asm volatile("nop");
	}
}

/**
 * @brief Write bits to SWD port
 *
 * @param v   Value of the bits to write
 * @param len Number of bit(s) to write
 */
void swd_wr(uint32_t v, uint len)
{
	uint wait;

	for ( ; len ; len--)
	{
		/* Set next bit to SWD-DAT */
		if (v & 1) ios_pin_set(PIN_SWDIO, 1);
		else       ios_pin_set(PIN_SWDIO, 0);
		/* Falling edge to SWD-CLK */
		ios_pin_set(PIN_SWCLK, 0);
		/* Wait a bit TODO: improve delay */
		for (wait = 0; wait < WAIT_DELAY; wait++)
			asm volatile("nop");
		/* Rising edge to SWD-CLK */
		ios_pin_set(PIN_SWCLK, 1);
		/* Wait a bit TODO: improve delay */
		for (wait = 0; wait < WAIT_DELAY; wait++)
			asm volatile("nop");

		/* Shift byte to select next bit */
		v = (v >> 1);
	}
}

/**
 * @brief Compute a parity bit
 *
 * @param uint32_t Input value
 * @return integer Return 1 for an odd number of '1' into input value
 */
static inline uint _parity(uint32_t value)
{
	value ^= value >> 16;
	value ^= value >> 8;
	value ^= value >> 4;
	value &= 0x0f;

	return (0x6996 >> value) & 1;
}
/* EOF */
