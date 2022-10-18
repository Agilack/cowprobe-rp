/**
 * @file  main.c
 * @brief Entry point and main function of cmsis-usbmon test tool
 *
 * @author Saint-Genest Gwenael <gwen@cowlab.fr>
 * @copyright Cowlab (c) 2022
 *
 * @page License
 * This software is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as published
 * by the Free Software Foundation. You should have received a copy of the
 * GNU General Public License along with this program, see LICENSE.md file
 * for more details.
 * This program is distributed WITHOUT ANY WARRANTY.
 */
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include "usbmon.h"

#define DATA_SIZE 1024

static int last_dap_info;

void req_dap_info(usbmon_pkt *pkt);
void rsp_dap_info(usbmon_pkt *pkt);
void req_swd_sequence(usbmon_pkt *pkt);
void req_swj_pins(usbmon_pkt *pkt);
void rsp_connect(usbmon_pkt *pkt);
void rsp_swj_pins(usbmon_pkt *pkt);
void req_swj_sequence(usbmon_pkt *pkt);

/**
 * @brief Entry point of this program
 *
 */
int main(int argc, char **argv)
{
	char dev_default[32] = "/dev/usbmon0";
	unsigned char data[DATA_SIZE];
	struct usbmon_packet hdr;
	struct mon_get_arg event;
	int fd, r;
	int sel_dev = 0;
	unsigned int i;
	u8 last_cmd = 0xFF;

	if (argc >= 2)
	{
		sel_dev = atoi(argv[1]);
		printf("Activate filter on device %d\n", sel_dev);
	}

	fd = open(dev_default, O_RDONLY);
	if (fd < 0)
	{
		fprintf(stderr, "Failed to open %s (%d)\n", dev_default, fd);
		perror("");
		return 1;
	}

	memset(&hdr, 0, sizeof hdr);
	event.hdr  = &hdr;
	event.data = data;
	event.alloc = DATA_SIZE;

	while(1)
	{
		memset(data, 0, DATA_SIZE);

		// Try to get next packet from usbmon
		r = ioctl(fd, MON_IOCX_GET, &event);
		if ((r == -1) && (errno == EINTR))
			continue;
		if (r < 0) {
			perror("ioctl");
			break;
		}

		// Compare device with filter (if any)
		if ((sel_dev > 0) && (hdr.devnum != sel_dev))
			continue;

		// Dump raw content of the packet
		printf("USB: ");
		switch (hdr.type)
		{
			case 'S': printf("Submit   "); break;
			case 'C': printf("Complete "); break;
			case 'E': printf("Error    "); break;
			default:  printf("  '%c'   ", hdr.type); break;
		}
		switch(hdr.xfer_type)
		{
			case 0x02: printf("CTRL "); break;
			case 0x03: printf("BULK "); break;
			default:   printf("%.2X ", hdr.xfer_type);
		}
		printf(" bus=%d dev=%d ep=%.2X (%d): ", hdr.busnum, hdr.devnum, hdr.epnum, hdr.length);
		if ((hdr.xfer_type == 2) && (hdr.epnum == 0x80) && (hdr.type == 'S'))
		{
			for (i=0; i<8; i++)
				printf("%02X ", hdr.s.setup[i]);
		}
		else if (hdr.len_cap)
		{
			for (i=0; i<hdr.len_cap; i++)
				printf("%02X ", data[i]);
		}
		printf("\n");

		// If packet is Host-to-Device
		if ((hdr.epnum == 0x07) && (hdr.len_cap > 0))
		{
			last_cmd = data[0];
			printf("\x1B[33m");
			switch(data[0])
			{
				case 0x00: req_dap_info(&event); break;
				case 0x01: printf("DAP_HostStatus"); break;
				case 0x02: printf("DAP_Connect");    break;
				case 0x03: printf("DAP_Disconnect"); break;
				case 0x04: printf("DAP_TransferConfigure"); break;
				case 0x05: printf("DAP_Transfer");   break;
				case 0x10: req_swj_pins(&event);     break;
				case 0x11: printf("DAP_SWJ_Clock");  break;
				case 0x12: req_swj_sequence(&event); break;
				case 0x13: printf("DAP_SWD_Configure"); break;
				case 0x1D: req_swd_sequence(&event); break;
			}
			printf("\x1B[0m\n");
		}
		// If packet is for Device-to-Host
		else if ((hdr.epnum == 0x88) && (hdr.type == 'C'))
		{
			if (data[0] != last_cmd)
				printf("\x1B[31mLast command is %.2x but response is %.2X\n", last_cmd, data[0]);
			printf("\x1B[34m");
			switch(data[0])
			{
				case 0x00: rsp_dap_info(&event);           break;
				case 0x01: printf("Recv: DAP_HostStatus"); break;
				case 0x02: rsp_connect(&event);            break;
				case 0x03: printf("Recv: DAP_Disconnect"); break;
				case 0x04: printf("Recv: DAP_TransferConfigure"); break;
				case 0x05: printf("Recv: DAP_Transfer");      break;
				case 0x10: rsp_swj_pins(&event);              break;
				case 0x11: printf("Recv: DAP_SWJ_Clock");     break;
				case 0x12: printf("Recv: DAP_SWJ_Sequence");  break;
				case 0x13: printf("Recv: DAP_SWD_Configure"); break;
				case 0x1D: printf("Recv: DAP_SWD_Sequence");  break;
			}
			printf("\x1B[0m\n");
			last_cmd = 0;
		}
	}
	close(fd);

	return 0;
}

/**
 * @brief Analyze and decode a DAP_Connect response
 *
 * @param pkt Pointer to the received usbmon packet
 */
void rsp_connect(usbmon_pkt *pkt)
{
	// Sanity check
	if ((pkt == 0) || (pkt->hdr->length < 2) || (pkt->data[0] != 0x02))
		return;

	printf("Recv: DAP_Connect");
	if (pkt->data[1] == 1)
		printf(" mode=SWD");
	else if (pkt->data[1] == 2)
		printf(" mode=JTAG");
	else if (pkt->data[1] == 0)
		printf(" FAILED");
	else
		printf(" Unknown result %x", pkt->data[1]);
}


/**
 * @brief Analyze and decode a DAP_Info request
 *
 * @param pkt Pointer to the received usbmon packet
 */
void req_dap_info(usbmon_pkt *pkt)
{
	// Sanity check
	if ((pkt == 0) || (pkt->hdr->length < 2) || (pkt->data[0] != 0x00))
		return;

	printf("Send: DAP_Info");
	switch(pkt->data[1])
	{
		case 0x01: printf(" Get probe vendor name"); break;
		case 0x02: printf(" Get probe product name"); break;
		case 0x03: printf(" Get probe serial number"); break;
		case 0x04: printf(" Get CMSIS-DAP Protocol Version"); break;
		case 0xF0: printf(" Get capabilities of debug unit"); break;
		case 0xFE: printf(" Get packet count"); break;
		case 0xFF: printf(" Get packet size"); break;
	}
	last_dap_info = pkt->data[1];
}

/**
 * @brief Analyze and decode a DAP_Info response
 *
 * @param pkt Pointer to the received usbmon packet
 */
void rsp_dap_info(usbmon_pkt *pkt)
{
	char str[256];
	u32  v;

	// Sanity check
	if ((pkt == 0) || (pkt->data[0] != 0x00))
		return;

	memset(str, 0, 256);

	printf("Recv: DAP_Info");
	switch(last_dap_info)
	{
		case 0x02:
			memcpy(str, pkt->data + 2, pkt->data[1]);
			printf(" Product name is \"%s\"", str);
			break;
		case 0x03:
			memcpy(str, pkt->data + 2, pkt->data[1]);
			printf(" probe serial number is \"%s\"", str);
			break;
		case 0x04:
			memcpy(str, pkt->data + 2, pkt->data[1]);
			printf(" supported CMSIS-DAP protocol version \"%s\"", str);
			break;
		case 0xF0:
			printf(" Capabilities:");
			if (pkt->data[1] & 0x01)
				printf(" SWD is supported");
			if (pkt->data[2] & 0x02)
				printf(" JTAG is supported");
			break;
		case 0xFF:
			v = (pkt->data[3] << 8) | pkt->data[2];
			printf(" packet size is %d", v);
			break;
	}
}

/**
 * @brief Analyze and decode a DAP_SWD_Sequence request
 *
 * @param pkt Pointer to the received usbmon packet
 */
void req_swd_sequence(usbmon_pkt *pkt)
{
	unsigned int seq_count;
	unsigned char *p;
	unsigned int dir, len;
	unsigned int i;

	// Sanity check
	if ((pkt == 0) || (pkt->hdr->length < 2) || (pkt->data[0] != 0x1D))
		return;

	// Extract number of sequences of this request
	seq_count = pkt->data[1];
	// Set pointer on first sequence byte
	p = (pkt->data + 2);

	printf("Send: DAP_SWD_Sequence (%d) ", seq_count);

	for (i = 0; i < seq_count; i++)
	{
		dir = (p[0] & 0x80) ? 1 : 0;
		len = (p[0] & 0x3F);

		if (dir)
		{
			printf("IN(%d) ", len);
			p++;
		}
		else
		{
			printf("OUT(%d) ", len);
			p += 1 + (len / 8);
		}
	}
}

/**
 * @brief Analyze and decode a DAP_SWJ_Pins request
 *
 * @param pkt Pointer to the received usbmon packet
 */
void req_swj_pins(usbmon_pkt *pkt)
{
	// Sanity check
	if ((pkt == 0) || (pkt->data[0] != 0x10))
		return;
	// Format check
	if (pkt->hdr->length < 7)
		goto err_malformed;

	printf("Send: DAP_SWJ_Pins");

	if (pkt->data[2] & 1)
		printf(" SWCLK=%d", (pkt->data[1] & 1) ? 1 : 0);
	if (pkt->data[2] & 2)
		printf(" SWDIO=%d", (pkt->data[1] & 2) ? 1 : 0);
	if (pkt->data[2] & 0x80)
		printf(" nReset=%d", (pkt->data[1] & 0x80) ? 1 : 0);
	return;

err_malformed:
	printf("\x1B[31m");
	printf("Malformed DAP_SWJ_Pins");
}

/**
 * @brief Analyze and decode a DAP_SWJ_Pins response
 *
 * @param pkt Pointer to the received usbmon packet
 */
void rsp_swj_pins(usbmon_pkt *pkt)
{
	// Sanity check
	if (pkt->data[0] != 0x10)
		return;

	printf("Recv: DAP_SWJ_Pins");

	printf(" SWCLK=%d",  (pkt->data[1] & 0x01) ? 1 : 0);
	printf(" SWDIO=%d",  (pkt->data[1] & 0x02) ? 1 : 0);
	printf(" nReset=%d", (pkt->data[1] & 0x80) ? 1 : 0);
}

/**
 * @brief Analyze and decode a DAP_SWJ_Sequence request
 *
 * @param pkt Pointer to the received usbmon packet
 */
void req_swj_sequence(usbmon_pkt *pkt)
{
	unsigned int len;
	unsigned char v;
	unsigned int i, j;

	// Sanity check
	if (pkt->data[0] != 0x12)
		return;

	len = pkt->data[1];
	printf("Send: DAP_SWJ_Sequence ");
	for (i = 0; i < len; i++)
	{
		v = pkt->data[2 + (i / 8)];
		for (j = 0; j < 8; j++)
		{
			printf("%d", (v & 1));
			v = (v >> 1);
			i++;
		}
		printf(" ");
	}
}
/* EOF */
