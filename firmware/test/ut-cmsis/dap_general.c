/**
 * @file  dap_general.c
 * @brief Collection of functions to test general DAP commands
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
#include "dap_general.h"

/**
 * @brief Test the DAP_Connect request
 *
 * @param env Pointer to a structure with probe environment
 * @return integer On success 0 is returned, negative value for error
 */
int tst_connect(cmsis_env *env)
{
	int result = 0;

	printf(" - Test DAP_Connect :\n");

	env->tx[0]  = 0x02;
	env->tx_len = 2;

	printf("     - Mode default ... ");

	env->tx[1]  = 0x00;

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

	/* 2) Now ask probe to use explicit SWD mode */

	printf("     - Mode SWD ... ");

	env->tx[1] = 1;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len != 2) || (env->rx[0] != 0x02))
		return( err_header(env, 1) );

	if (env->rx[1] == 0x01)
	{
		color(32); printf("Success"); color(0);
		printf("\n");
	}
	else
	{
		color(31); printf("Failed"); color(0);
		printf(" mode=%.2X (0x01 expected)\n", env->rx[1]);
		result = -3;
	}

	/* 2) Now ask probe to use explicit JTAG mode */

	printf("     - Mode JTAG ... ");

	env->tx[1] = 2;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len != 2) || (env->rx[0] != 0x02))
		return( err_header(env, 1) );

	if (env->rx[1] == 0x02)
	{
		color(32); printf("Success"); color(0);
		printf("\n");
	}
	else
	{
		color(31); printf("Failed"); color(0);
		printf(" mode=%.2X (0x02 expected)\n", env->rx[1]);
		result = -3;
	}

	return(result);
}

/**
 * @brief Test the DAP_Delay request
 *
 * @param env Pointer to a structure with probe environment
 * @return integer On success 0 is returned, negative value for error
 */
int tst_delay(cmsis_env *env)
{
	unsigned short delay;
	int result = 0;

	delay = 10;

	printf(" - Test DAP_Delay (%.2x) ... ", delay);

	env->tx[0]  = 0x09; /* Command */
	env->tx[1]  = ((delay >> 8) & 0xFF);
	env->tx[2]  = ((delay >> 0) & 0xFF);
	env->tx_len = 3;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len != 2) || (env->rx[0] != 0x09))
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
 * @brief Test the DAP_Disconnect request
 *
 * @param env Pointer to a structure with probe environment
 * @return integer On success 0 is returned, negative value for error
 */
int tst_disconnect(cmsis_env *env)
{
	int result = 0;

	printf(" - Test DAP_Disconnect ... ");

	env->tx[0]  = 0x03; /* Command */
	env->tx_len = 1;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len != 2) || (env->rx[0] != 0x03))
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
 * @brief Test the DAP_HostStatus command
 *
 * @param env Pointer to a structure with probe environment
 * @return integer On success 0 is returned, negative value for error
 */
int tst_host_status(cmsis_env *env)
{
	int result = 0;

	printf(" - Test DAP_HostStatus ... ");

	env->tx[0]  = 0x01; /* Command */
	env->tx[1]  = 0x00; /* Type: connect */
	env->tx[2]  = 1;    /* Status: true */
	env->tx_len = 3;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len != 2) || (env->rx[0] != 0x01))
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
 * @brief Test the DAP_ResetTarget command
 *
 * @param env Pointer to a structure with probe environment
 * @return integer On success 0 is returned, negative value for error
 */
int tst_reset_target(cmsis_env *env)
{
	int result = 0;

	printf(" - Test DAP_ResetTarget ... ");

	env->tx[0]  = 0x0A; /* Command */
	env->tx_len = 1;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len != 3) || (env->rx[0] != 0x0A))
		return( err_header(env, 1) );

	if (env->rx[1] == 0x00)
	{
		color(32); printf("Success"); color(0);
		printf(" (%.2x)\n", env->rx[2]);
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
 * @brief Test the DAP_WriteABORT command
 *
 * @param env Pointer to a structure with probe environment
 * @return integer On success 0 is returned, negative value for error
 */
int tst_write_abort(cmsis_env *env)
{
	unsigned long vreg = 0x00000001;
	int result = 0;

	printf(" - Test DAP_WriteAbort (%.8X) ... ", (unsigned int)vreg);

	env->tx[0]  = 0x08; /* Command */
	env->tx[1]  = 0x00; /* Dap index*/
	env->tx[2]  = ((vreg >> 24) & 0xFF);
	env->tx[3]  = ((vreg >> 16) & 0xFF);
	env->tx[4]  = ((vreg >>  8) & 0xFF);
	env->tx[5]  = ((vreg >>  0) & 0xFF);
	env->tx_len = 6;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len != 2) || (env->rx[0] != 0x08))
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
