/**
 * @file  swd.c
 * @brief Collection of functions to test SD interface
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
#include <stdio.h>
#include <string.h>
#include "test.h"

/**
 * @brief Use DAP_Connect to enable SWD port mode
 *
 * @param env Pointer to a structure with probe environment
 * @return integer On success 0 is returned, negative value for error
 */
int swd_connect(cmsis_env *env)
{
	int result = 0;

	printf(" - SWD DAP_Connect ...");

	env->tx[0]  = 0x02;
	env->tx[1]  = 0x01;
	env->tx_len = 2;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len != 2) || (env->rx[0] != 0x02))
		return( err_header(env, 1) );

	if (env->rx[1] == 0x01)
	{
		color(32); printf("Success"); color(0);
		printf(" (SWD)\n");
	}
	else
	{
		color(31); printf("Failed"); color(0);
		printf(" Bad port for default mode: %.2X\n", env->rx[1]);
		result = -3;
	}
	return(result);
}

/**
 * @brief Use DAP_Transfer to read IDCODE
 *
 * @param env Pointer to a structure with probe environment
 * @return integer On success 0 is returned, negative value for error
 */
int swd_dpidr(cmsis_env *env)
{
	unsigned long data;
	int result = 0;

	printf(" - SWD read DPIDR ... ");

	env->tx[0] = 0x05; /* DAP_Transfer */
	env->tx[1] = 0x00; /* Index of the DAP */
	env->tx[2] = 1;    /* Transfer count */
	env->tx[3] = 0x02; /* SWD request value */
	env->tx_len = 4;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len != 7) || (env->rx[0] != 0x05))
		return( err_header(env, 1) );

	if ((env->rx[1] == 0x01) && (env->tx[2] == 0x01))
	{
		color(32); printf("Success"); color(0);
		data  = (env->rx[6] << 24);
		data |= (env->rx[5] << 16);
		data |= (env->rx[4] <<  8);
		data |= (env->rx[3] <<  0);
		printf(" 0x%.8X\n", (unsigned int)data);
	}
	else
	{
		color(31); printf("Failed"); color(0);
		printf(" error reported: %.2X %.2X\n", env->rx[1], env->rx[2]);
		result = -3;
	}
	return(result);
}

/**
 * @brief Test the SWD line reset cycle (50 cycles)
 *
 * @param env Pointer to a structure with probe environment
 * @return integer On success 0 is returned, negative value for error
 */
int swd_reset(cmsis_env *env)
{
	unsigned char bits[] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff};
	int result = 0;

	printf(" - SWD reset ... ");

	env->tx[0]  = 0x12; /* SWJ_Sequence */
	env->tx[1]  = 50;   /* Number of bits to transfer */
	memcpy(env->tx + 2, bits, 7);
	env->tx_len = 9;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len != 2) || (env->rx[0] != 0x12))
		return( err_header(env, 1) );

	if (env->rx[1] == 0x00)
	{
		color(32); printf("Success"); color(0);
		printf("\n");
	}
	else
	{
		color(31); printf("Failed"); color(0);
		printf(" error reported: %.2X\n", env->rx[1]);
		result = -3;
	}
	return(result);
}

/**
 * @brief Test the Jtag-to-SWD sequence
 *
 * @param env Pointer to a structure with probe environment
 * @return integer On success 0 is returned, negative value for error
 */
int swd_j2s(cmsis_env *env)
{
	unsigned char bits[] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	                        0x9e, 0xe7,
	                        0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	                        0x00};
	int result = 0;

	printf(" - SWD Jtag-to-SWD ... ");

	env->tx[0] = 0x12; /* SWJ_Sequence */
	env->tx[1] =  136; /* Number of bits to transfer */
	memcpy(env->tx + 2, bits, 17);
	env->tx_len = 19;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len != 2) || (env->rx[0] != 0x12))
		return( err_header(env, 1) );

	if (env->rx[1] == 0x00)
	{
		color(32); printf("Success"); color(0);
		printf("\n");
	}
	else
	{
		color(31); printf("Failed"); color(0);
		printf(" error reported: %.2X\n", env->rx[1]);
		result = -3;
	}
	return(result);
}
/* EOF */
