/**
 * @file  dap_info.c
 * @brief Collection of functions to test all DAP_Info commands
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
#include "dap_info.h"

/**
 * @brief Test the DAP_Info "Capabilities" request
 *
 */
int tst_info_capabilities(cmsis_env *env)
{
	const uint8_t req[] = { 0x00, 0xF0 };

	printf(" - Test DAP_Info::Get_Capabilities ... ");

	memcpy(env->tx, req, 2);
	env->tx_len = 2;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len != 3) || (env->rx[0] != 0x00) || (env->rx[1] != 0x01))
		return( err_header(env, 2) );

	color(32); printf("Success"); color(0);

	/* Decode and display capabilities */
	printf(" support:");
	if (env->rx[2] & (1 << 0))
		printf(" SWD");
	if (env->rx[2] & (1 << 1))
		printf(" JTAG");
	printf("\n");

	return(0);
}

int tst_info_packet_count(cmsis_env *env)
{
	const uint8_t req[] = { 0x00, 0xFE };

	printf(" - Test DAP_Info::Get_PacketCount ... ");

	memcpy(env->tx, req, 2);
	env->tx_len = 2;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len != 3) || (env->rx[0] != 0x00) || (env->rx[1] != 0x01))
		return( err_header(env, 2) );

	color(32); printf("Success"); color(0);

	/* Decode and display packet count */
	printf(" count=%d\n", (unsigned int)env->rx[2]);

	return(0);
}

int tst_info_packet_size(cmsis_env *env)
{
	const uint8_t req[] = { 0x00, 0xFF };
	int  size;

	printf(" - Test DAP_Info::Get_PacketSize ... ");

	memcpy(env->tx, req, 2);
	env->tx_len = 2;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len != 4) || (env->rx[0] != 0x00) || (env->rx[1] != 0x02))
		return( err_header(env, 2) );

	color(32); printf("Success"); color(0);

	/* Decode and display packet count */
	size = (unsigned int)((env->rx[3] << 8) | env->rx[2]);
	printf(" size=%d\n", size);

	return(0);
}

int tst_info_product_name(cmsis_env *env)
{
	const uint8_t req[] = { 0x00, 0x02 };
	char str[128];
	int len;

	printf(" - Test DAP_Info::Get_ProductName ... ");

	memcpy(env->tx, req, 2);
	env->tx_len = 2;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len < 2) || (env->rx[0] != 0x00))
		return( err_header(env, 1) );

	color(32); printf("Success"); color(0);

	/* Extract and display product name string */
	len = env->rx[1];
	if (len > 0)
	{
		memset(str, 0, 128);
		memcpy(str, env->rx + 2, len);
		printf(" \"%s\"", str);
	}
	printf("\n");

	return(0);
}

/**
 * @brief Test the DAP_Info "Protocol Version" request
 *
 */
int tst_info_protocol_version(cmsis_env *env)
{
	const uint8_t req[] = { 0x00, 0x04 };
	char str[128];
	int len;

	printf(" - Test DAP_Info::Get_ProtocolVersion ... ");

	memcpy(env->tx, req, 2);
	env->tx_len = 2;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len < 2) || (env->rx[0] != 0x00))
		return( err_header(env, 1) );

	color(32); printf("Success"); color(0);

	/* Extract and display version string */
	len = env->rx[1];
	if (len > 0)
	{
		memset(str, 0, 128);
		memcpy(str, env->rx + 2, len);
		printf(" \"%s\"", str);
	}
	printf("\n");

	return(0);
}

/**
 * @brief Test the DAP_Info "Serial Number" request
 *
 */
int tst_info_serial(cmsis_env *env)
{
	const uint8_t req[] = { 0x00, 0x03 };
	char str[128];
	int len;

	printf(" - Test DAP_Info::Get_Serial ... ");

	memcpy(env->tx, req, 2);
	env->tx_len = 2;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len < 2) || (env->rx[0] != 0x00))
		return( err_header(env, 1) );

	color(32); printf("Success"); color(0);

	/* Extract and display serial string */
	len = env->rx[1];
	if (len > 0)
	{
		memset(str, 0, 128);
		memcpy(str, env->rx + 2, len);
		printf(" \"%s\"", str);
	}
	printf("\n");

	return(0);
}

/**
 * @brief Test the DAP_Info "Vendor Name" request
 *
 */
int tst_info_vendor(cmsis_env *env)
{
	const uint8_t req[] = { 0x00, 0x01 };
	char str[128];
	int len;

	printf(" - Test DAP_Info::Get_Vendor ... ");

	memcpy(env->tx, req, 2);
	env->tx_len = 2;

	if (cmsis_txrx(env) < 0)
		return( err_request() );

	/* Check header of received response */
	if ((env->rx_len < 2) || (env->rx[0] != 0x00))
		return( err_header(env, 1) );

	color(32); printf("Success"); color(0);

	/* Extract and display vendor string */
	len = (unsigned int)env->rx[1];
	if (len > 0)
	{
		memset(str, 0, 128);
		memcpy(str, env->rx + 2, len);
		printf(" \"%s\"", str);
	}
	printf("\n");

	return(0);
}
/* EOF */
