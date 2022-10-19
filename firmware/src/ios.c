/**
 * @file  ios.c
 * @brief Manage configuration of IOs, main debug port and extension
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

/**
 * @brief Initialize GPIOs
 *
 * The goal of the IOs module is to drive gpios (as inputs, outputs or
 * dedicated functions). This function initialize and configure the gpios
 * used for the debug port or extension. For the ios to work properly, this
 * function must be called before any other function of this module.
 */
void ios_init(void)
{
	/* Configure main port pin D0 */
	gpio_init(PORT_D0_PIN);
	gpio_init(PORT_D0_DIR);
	ios_pin_mode(PORT_D0_PIN, IO_DIR_IN);
	gpio_set_dir(PORT_D0_DIR, GPIO_OUT);
	/* Configure main port pin D1 */
	gpio_init(PORT_D1_PIN);
	gpio_init(PORT_D1_DIR);
	ios_pin_mode(PORT_D1_PIN, IO_DIR_IN);
	gpio_set_dir(PORT_D1_DIR, GPIO_OUT);
	/* Configure main port pin D2 */
	gpio_init(PORT_D2_PIN);
	gpio_init(PORT_D2_DIR);
	ios_pin_mode(PORT_D2_PIN, IO_DIR_IN);
	gpio_set_dir(PORT_D2_DIR, GPIO_OUT);
	/* Configure main port pin D3 */
	gpio_init(PORT_D3_PIN);
	gpio_init(PORT_D3_DIR);
	ios_pin_mode(PORT_D2_PIN, IO_DIR_IN);
	gpio_set_dir(PORT_D3_DIR, GPIO_OUT);

	/* Configure internal extension IOs (upper side 01 > 08) */
	gpio_init(EXT_01_PIN);   gpio_set_dir(EXT_01_PIN, GPIO_IN);
	gpio_init(EXT_02_PIN);   gpio_set_dir(EXT_02_PIN, GPIO_IN);
	gpio_init(EXT_03_PIN);   gpio_set_dir(EXT_03_PIN, GPIO_IN);
	gpio_init(EXT_04_PIN);   gpio_set_dir(EXT_04_PIN, GPIO_IN);
	gpio_init(EXT_05_PIN);   gpio_set_dir(EXT_05_PIN, GPIO_IN);
	gpio_init(EXT_06_PIN);   gpio_set_dir(EXT_06_PIN, GPIO_IN);
	gpio_init(EXT_07_PIN);   gpio_set_dir(EXT_07_PIN, GPIO_IN);
	gpio_init(EXT_08_PIN);   gpio_set_dir(EXT_08_PIN, GPIO_IN);
	/* Configure internal extension IOs (lower side 09 > 16) */
	gpio_init(EXT_09_PIN);   gpio_set_dir(EXT_09_PIN, GPIO_IN);
	gpio_init(EXT_10_PIN);   gpio_set_dir(EXT_10_PIN, GPIO_IN);
	gpio_init(EXT_11_PIN);   gpio_set_dir(EXT_11_PIN, GPIO_IN);
	gpio_init(EXT_12_PIN);   gpio_set_dir(EXT_12_PIN, GPIO_IN);
	gpio_init(EXT_13_PIN);   gpio_set_dir(EXT_13_PIN, GPIO_IN);
	gpio_init(EXT_14_PIN);   gpio_set_dir(EXT_14_PIN, GPIO_IN);
	gpio_init(EXT_15_PIN);   gpio_set_dir(EXT_15_PIN, GPIO_IN);
	gpio_init(EXT_16_PIN);   gpio_set_dir(EXT_16_PIN, GPIO_IN);
}

/**
 * @brief Configure the IOs of debug port for a specified mode
 *
 * @param mode New mode to set for the debug port
 */
void ios_mode(int mode)
{
	if (mode == PORT_MODE_HIZ)
	{
		ios_pin_mode(PORT_D0_PIN, IO_DIR_IN);
		ios_pin_mode(PORT_D1_PIN, IO_DIR_IN);
		ios_pin_mode(PORT_D2_PIN, IO_DIR_IN);
		ios_pin_mode(PORT_D3_PIN, IO_DIR_IN);
	}
	else if (mode == PORT_MODE_JTAG)
	{
		/* Configure D0 as TDI (input) */
		ios_pin_mode(PORT_D0_PIN, IO_DIR_IN);
		/* Configure D1 as TMS (output) */
		ios_pin_mode(PORT_D1_PIN, IO_DIR_OUT);
		gpio_put(PORT_D1_PIN, 0);
		/* Configure D2 as TCK (output) */
		ios_pin_mode(PORT_D2_PIN, IO_DIR_OUT);
		gpio_put(PORT_D2_PIN, 0);
		/* Configure D3 as TDO (output) */
		ios_pin_mode(PORT_D3_PIN, IO_DIR_OUT);
		gpio_put(PORT_D3_PIN, 0);
	}
	else if (mode == PORT_MODE_SWD)
	{
		/* Configure D1 as SW-DAT (output) */
		ios_pin_mode(PORT_D1_PIN, IO_DIR_OUT);
		gpio_put(PORT_D1_PIN, 1);
		/* Configure D2 as SW-CLK (io) */
		ios_pin_mode(PORT_D2_PIN, IO_DIR_OUT);
		gpio_put(PORT_D2_PIN, 1);
		/* Configure D3 as nReset (output) */
		ios_pin_mode(PORT_D3_PIN, IO_DIR_OUT);
		gpio_put(PORT_D3_PIN, 1);
	}
}

/**
 * @brief Read the current state of an IO
 *
 * @param pin Identifier of the pin to read
 * @return Current value of the pin
 */
int ios_pin(int pin)
{
	int result;

	result = gpio_get(pin);

	return(result);
}

/**
 * @brief Configure one specific pin (in or out)
 *
 * @param pin  Identifier of the pin to configure
 * @param mode New mode to set for the specified pin
 */
void ios_pin_mode(int pin, int mode)
{
	switch(pin)
	{
		case PORT_D0_PIN:
			if (mode == IO_DIR_IN)
			{
				/* Set MCU pin as input first */
				gpio_set_dir(PORT_D0_PIN, GPIO_IN);
				asm volatile("nop");
				/* Then, configure external buffer */
				gpio_put(PORT_D0_DIR, IO_DIR_IN);
			}
			else if (mode == IO_DIR_OUT)
			{
				/* Set external buffer as output first */
				gpio_put(PORT_D0_DIR, IO_DIR_OUT);
				asm volatile("nop");
				/* Then, set MCU pin as output */
				gpio_set_dir(PORT_D0_PIN, GPIO_OUT);
			}	
			break;

		case PORT_D1_PIN:
			if (mode == IO_DIR_IN)
			{
				/* Set MCU pin as input first */
				gpio_set_dir(PORT_D1_PIN, GPIO_IN);
				asm volatile("nop");
				/* Then, configure external buffer */
				gpio_put(PORT_D1_DIR, IO_DIR_IN);
			}
			else if (mode == IO_DIR_OUT)
			{
				/* Set external buffer as output first */
				gpio_put(PORT_D1_DIR, IO_DIR_OUT);
				asm volatile("nop");
				/* Then, set MCU pin as output */
				gpio_set_dir(PORT_D1_PIN, GPIO_OUT);
			}	
			break;

		case PORT_D2_PIN:
			if (mode == IO_DIR_IN)
			{
				/* Set MCU pin as input first */
				gpio_set_dir(PORT_D2_PIN, GPIO_IN);
				asm volatile("nop");
				/* Then, configure external buffer */
				gpio_put(PORT_D2_DIR, IO_DIR_IN);
			}
			else if (mode == IO_DIR_OUT)
			{
				/* Set external buffer as output first */
				gpio_put(PORT_D2_DIR, IO_DIR_OUT);
				asm volatile("nop");
				/* Then, set MCU pin as output */
				gpio_set_dir(PORT_D2_PIN, GPIO_OUT);
			}	
			break;

		case PORT_D3_PIN:
			if (mode == IO_DIR_IN)
			{
				/* Set MCU pin as input first */
				gpio_set_dir(PORT_D3_PIN, GPIO_IN);
				asm volatile("nop");
				/* Then, configure external buffer */
				gpio_put(PORT_D3_DIR, IO_DIR_IN);
			}
			else if (mode == IO_DIR_OUT)
			{
				/* Set external buffer as output first */
				gpio_put(PORT_D3_DIR, IO_DIR_OUT);
				asm volatile("nop");
				/* Then, set MCU pin as output */
				gpio_set_dir(PORT_D3_PIN, GPIO_OUT);
			}	
			break;

		/* Internal extension (upper side) */
		case EXT_01_PIN:
		case EXT_02_PIN:
		case EXT_03_PIN:
		case EXT_04_PIN:
		case EXT_05_PIN:
		case EXT_06_PIN:
		case EXT_07_PIN:
		case EXT_08_PIN:
		/* Internal extension (lower side) */
		case EXT_09_PIN:
		case EXT_10_PIN:
		case EXT_11_PIN:
		case EXT_12_PIN:
		case EXT_13_PIN:
		case EXT_14_PIN:
		case EXT_15_PIN:
		case EXT_16_PIN:
		{
			if (mode == IO_DIR_IN)
				gpio_set_dir(pin, GPIO_IN);
			else if (mode == IO_DIR_OUT)
				gpio_set_dir(pin, GPIO_OUT);
			break;
		}
	}
}

/**
 * @brief Set the state of a pin when used as gpio/output
 *
 * @param pin   Identifier of the pin to modify
 * @param state New state for this pin
 */
void ios_pin_set(int pin, int state)
{
	gpio_put(pin, state);
}
/* EOF */
