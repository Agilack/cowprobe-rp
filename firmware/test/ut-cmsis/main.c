/**
 * @file  main.c
 * @brief Main functions of the CMSIS unit-test
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
#include <libusb-1.0/libusb.h>
#include "dap_general.h"
#include "dap_info.h"
#include "test.h"

int find_probe(libusb_device_handle **probe);

/**
 * @brief Entry point of the program
 *
 * @param argc Number of argument into command line
 * @param argv Array of string with command line arguments
 * @return integer Result of program execution (should be 0)
 */
int main(int argc, char **argv)
{
	cmsis_env env;
	int ret = 0;
	int err = 0;

	(void)argc;
	(void)argv;
	memset((void *)&env, 0, sizeof(cmsis_env));

	if (libusb_init(0) < 0)
	{
		fprintf(stderr, "Failed to init libusb\n");
		return(-1);
	}

	/* Search cowprobe USB device */
	if (find_probe(&env.dev) < 0)
	{
		fprintf(stderr, "Cowprobe: USB device not found\n");
		ret = -1;
		goto finish;
	}

	/* Test DAP_Info commands */
	err += tst_info_vendor(&env) ? 1 : 0;
	err += tst_info_product_name(&env) ? 1 : 0;
	err += tst_info_serial(&env) ? 1 : 0;
	err += tst_info_protocol_version(&env) ? 1 : 0;
	// TODO TargetDeviceVendor
	// TODO TargetDeviceName
	// TODO TargetBoardVendor
	// TODO TargetBoardName
	// TODO Product Firmware Version
	err += tst_info_capabilities(&env) ? 1 : 0;
	err += tst_info_packet_count(&env) ? 1 : 0;
	err += tst_info_packet_size(&env) ? 1 : 0;
	/* Test other general DAP commands */
	err += tst_connect(&env)     ? 1 : 0;
	err += tst_disconnect(&env)  ? 1 : 0;
	err += tst_host_status(&env) ? 1 : 0;
	err += tst_write_abort(&env) ? 1 : 0;
	err += tst_delay(&env)       ? 1 : 0;
	err += tst_reset_target(&env)? 1 : 0;

	printf("\n Test complete ");
	if (err == 0)
	{
		color(32);
		printf("0 error");
		color(0);
	}
	else
	{
		color(31);
		printf("%d errors", err);
		color(0);
	}
	printf("\n");

finish:
	if (env.dev)
	{
		libusb_close(env.dev);
		env.dev = 0;
	}
	libusb_exit(0);

	return(ret);
}

void color(int x)
{
	switch (x)
	{
		case  0: printf("\x1b[0m");  break;
		case 31: printf("\x1b[1;91m"); break;
		case 32: printf("\x1b[1;92m"); break;
	}
}

int find_probe(libusb_device_handle **probe)
{
	struct libusb_device_descriptor desc;
	libusb_device *dev, **list;
	ssize_t count;
	int result;
	int i;

	dev = 0;
	count = libusb_get_device_list(NULL, &list);

	for (i = 0; i < count; i++)
	{
		/* Get informations about next USB device */
		dev = list[i];
		result = libusb_get_device_descriptor(dev, &desc);
		/* If vendor and product id match cowprobe, device found :) */
		if ((desc.idVendor == 0x2E8A) && (desc.idProduct == 0x4002))
		{
			result = 0;
			break;
		}
		dev = 0;
	}

	libusb_free_device_list(list, 1);

	if (dev)
		result = libusb_open(dev, probe);
	else
		result = -1;

	return(result);
}

int cmsis_txrx(cmsis_env *env)
{
	int r, tr;

	r = libusb_bulk_transfer(env->dev, 0x07, env->tx, env->tx_len, &tr, 5000);

	if ((r != 0) || (tr != env->tx_len))
		return(-1);

	r = libusb_bulk_transfer(env->dev, 0x88, env->rx, 1024, &tr, 200);
	if (r == 0)
		env->rx_len = tr;
	else
		env->rx_len = 0;
	
	return(r);
}

/**
 * @brief Write a generic message when a bad header is received
 *
 * @return integer Always return -2 (bad header error code)
 */
int err_header(cmsis_env *env, int n)
{
	int i;

	color(31);
	printf("Bad response header");
	color(0);
	printf(" len=%d hdr=", env->rx_len);
	for (i = 0; i < n; i++)
		printf("%.2X ", env->rx[i]);
	printf("\n");
	return(-2);
}

/**
 * @brief Write a generic message in case of USB request error
 *
 * @return integer Always return -1 (request error code)
 */
int err_request(void)
{
	color(31);
	printf("Request failed\n");
	color(0);
	return(-1);
}
/* EOF */
