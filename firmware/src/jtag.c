/**
 * @file  jtag.c
 * @brief Implement JTAG state machine
 *
 * @authors Saint-Genest Gwenael <gwen@cowlab.fr>
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
#include "ios.h"
#include "jtag.h"
#include "types.h"

static uint jtag_bit_delay;

/**
 * @brief Activate the debug port in JTAG mode
 *
 * @result integer Zero is returuned on success
 */
int jtag_connect(void)
{
	/* Set default bit delay */
	jtag_bit_delay = 80;

	ios_mode(PORT_MODE_JTAG);
	return(0);
}

/**
 * @brief Terminate a JTAG session and disconnect port
 *
 * @return integer Zero is returned on success
 */
int jtag_disconnect(void)
{
	ios_mode(PORT_MODE_HIZ);
	return(0);
}

/**
 * @brief Execute one or multiple jtag transition
 *
 * @brief seq List of TMS values (one bit per transition)
 * @brief len Number of transitions to execute (up to 32)
 */
void jtag_tms_sequence(u32 seq, uint len)
{
	unsigned int wait;
	unsigned int i;

	for (i = 0; i < len ; i++)
	{
		/* Set next bit to TMS */
		if (seq & 1) ios_pin_set(PORT_D1_PIN, 1);
		else         ios_pin_set(PORT_D1_PIN, 0);

		/* Wait 1/2 clock period */
		for (wait = 0; wait < jtag_bit_delay; wait++)
			asm volatile("nop");
		/* Rising edge to TCK */
		ios_pin_set(PORT_D2_PIN, 1);
		/* Wait 1/2 clock period */
		for (wait = 0; wait < jtag_bit_delay; wait++)
			asm volatile("nop");
		/* Falling edge to TCK */
		ios_pin_set(PORT_D2_PIN, 0);

		/* Shift sequence to select next bit */
		seq = (seq >> 1);
	}
}

/**
 * @brief Shift bits to/from the target (constant TMS)
 *
 * @param value Mask of the bits to shift out
 * @param len   Number of bits to shift (0 to 32)
 * @param tms   Value to set for TMS (same to all shifts)
 * @return uint Mask of bits readed for shift in
 */
u32 jtag_shift(u32 value, uint len, uint tms)
{
	u32  result = 0;
	uint wait;
	uint i;

	/* First, set TMS value */
	ios_pin_set(PORT_D1_PIN, tms);

	for (i = 0; i < len; i++)
	{
		/* Set next TDO bit */
		ios_pin_set(PORT_D3_PIN, (value & 1));
		value = (value >> 1);

		/* Wait 1/2 clock period */
		for (wait = 0; wait < jtag_bit_delay; wait++)
			asm volatile("nop");

		/* Get next input bit */
		result = (result << 1);
		result |= ios_pin(PORT_D0_PIN);

		/* Rising edge to TCK */
		ios_pin_set(PORT_D2_PIN, 1);
		/* Wait 1/2 clock period */
		for (wait = 0; wait < jtag_bit_delay; wait++)
			asm volatile("nop");
		/* Falling edge to TCK */
		ios_pin_set(PORT_D2_PIN, 0);
	}
	return(result);
}

/**
 * @brief Shift bits to/from the target (constant TMS)
 *
 * @param value Mask of the bits to shift out
 * @param len   Number of bits to shift (0 to 8)
 * @param tms   Value to set for TMS (same to all shifts)
 * @return uint Mask of bits readed for shift in
 */
u8 jtag_rshift(u8 value, uint len, uint tms)
{
	u32  result = 0;
	uint wait;
	uint i;

	/* First, set TMS value */
	ios_pin_set(PORT_D1_PIN, tms);

	for (i = 0; i < len; i++)
	{
		/* Set next TDO bit */
		ios_pin_set(PORT_D3_PIN, (value & 0x80) ? 1 : 0);
		value = (value << 1);

		/* Wait 1/2 clock period */
		for (wait = 0; wait < jtag_bit_delay; wait++)
			asm volatile("nop");

		/* Get next input bit */
		result = (result >> 1);
		result |= (ios_pin(PORT_D0_PIN) ? 0x80 : 0);

		/* Rising edge to TCK */
		ios_pin_set(PORT_D2_PIN, 1);
		/* Wait 1/2 clock period */
		for (wait = 0; wait < jtag_bit_delay; wait++)
			asm volatile("nop");
		/* Falling edge to TCK */
		ios_pin_set(PORT_D2_PIN, 0);
	}
	return(result);
}
/* EOF */
