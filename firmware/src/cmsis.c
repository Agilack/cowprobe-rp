/**
 * @file  cmsis.c
 * @brief This module contains a CMSIS compatible probe (USB interface and DAP)
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
#include "cmsis.h"
#include "swd.h"
#include "usb.h"

#ifdef USE_CMSIS
#undef  DEBUG_CMSIS
#undef  DEBUG_CMSIS_USB

#define RX_SIZE 256

static uint8_t  cmsis_mode;
static uint16_t cmsis_clock;
/* USB and communication buffers */
static uint8_t ep_in_n;
static uint8_t ep_out_n;
static uint8_t rx_buffer[RX_SIZE];
static uint8_t tx_buffer[256];

static void dap_init(void);

/**
 * @brief Initialize the "cmsis" module
 *
 * This function initialize the cmsis module and configure IOs of the debug
 * port for SWD signals. For this module to work properly, this function must
 * be called before any other cmsis functions.
 */
void cmsis_init(void)
{
	log_puts("CMSIS: Initialization\r\n");
	cmsis_mode  = 0; // 0:Unused 1:SWD 2:JTAG
	cmsis_clock = 0;
	dap_init();
}

/* -------------------------------------------------------------------------- */
/* --                             DAP commands                             -- */
/* -------------------------------------------------------------------------- */

static inline int dap_connect(cmsis_pkt *req, cmsis_pkt *rsp);
static inline int dap_delay(cmsis_pkt *req, cmsis_pkt *rsp);
static inline int dap_disconnect(cmsis_pkt *req, cmsis_pkt *rsp);
static inline int dap_host_status(cmsis_pkt *req, cmsis_pkt *rsp);
static inline int dap_info(cmsis_pkt *req, cmsis_pkt *rsp);
static inline int dap_info_cap(cmsis_pkt *req, cmsis_pkt *rsp);
static inline int dap_reset_target(cmsis_pkt *req, cmsis_pkt *rsp);
static inline int dap_swd_configure(cmsis_pkt *req, cmsis_pkt *rsp);
static inline int dap_swd_sequence(cmsis_pkt *req, cmsis_pkt *rsp);
static inline int dap_swj_clock(cmsis_pkt *req, cmsis_pkt *rsp);
static inline int dap_swj_pins(cmsis_pkt *req, cmsis_pkt *rsp);
static inline int dap_swj_sequence(cmsis_pkt *req, cmsis_pkt *rsp);
static inline int dap_transfer(cmsis_pkt *req, cmsis_pkt *rsp);
static inline int dap_transfer_configure(cmsis_pkt *req, cmsis_pkt *rsp);
static inline int dap_write_abort(cmsis_pkt *req, cmsis_pkt *rsp);

static int  dap_data_phase;
static int  dap_idle_cycles;
static int  dap_retry_wait;
static int  dap_retry_match;
static int  dap_ta_period;
static char str_serial[]  = "12345678";
static char str_version[] = "1.0";

/**
 * @brief Initialize DAP submodule
 *
 * This function is used to initialize the DAP component of the CMSIS
 * module (mainly internal variables). For the DAP to work properly, this
 * function must be called before any use of DAP.
 */
static void dap_init(void)
{
	dap_data_phase  = 0;
	dap_idle_cycles = 0;
	dap_retry_wait  = 16;
	dap_retry_match = 0;
	dap_ta_period   = 0;
}

/**
 * @brief Process an incoming CMSIS-DAP packet
 *
 * @param rx  Point to a buffer with received packet
 * @param len Number of bytes into received packet
 */
void dap_recv(uint8_t *rx, uint16_t len)
{
	cmsis_pkt req, rsp;
	int result = 1;
	int i;

	req.buffer = rx;
	req.len    = len;
	rsp.buffer = tx_buffer;
	rsp.len    = 0;

	rsp.buffer[0] = req.buffer[0];

	switch(req.buffer[0])
	{
		/* == General Commands == */

		/* DAP_Info */
		case 0x00:
			result = dap_info(&req, &rsp);
			break;
		/* DAP_HostStatus */
		case 0x01:
			result = dap_host_status(&req, &rsp);
			break;
		/* DAP_Connect */
		case 0x02:
			result = dap_connect(&req, &rsp);
			break;
		/* DAP_Disconnect */
		case 0x03:
			result = dap_disconnect(&req, &rsp);
			break;
		/* DAP_WriteABORT */
		case 0x08:
			result = dap_write_abort(&req, &rsp);
			break;
		/* DAP_Delay */
		case 0x09:
			result = dap_delay(&req, &rsp);
			break;
		/* DAP_ResetTarget */
		case 0x0A:
			result = dap_reset_target(&req, &rsp);
			break;

		/* == Common SWD/JTAG Commands == */

		/* DAP_SWJ_Pins */
		case 0x10:
			result = dap_swj_pins(&req, &rsp);
			break;
		/* DAP_SWJ_Clock */
		case 0x11:
			result = dap_swj_clock(&req, &rsp);
			break;
		/* DAP_SWJ_Sequence */
		case 0x12:
			result = dap_swj_sequence(&req, &rsp);
			break;

		/* == SWD Commands == */

		/* DAP_SWD_Configure */
		case 0x13:
			result = dap_swd_configure(&req, &rsp);
			break;
		/* DAP_SWD_Sequence */
		case 0x1D:
			result = dap_swd_sequence(&req, &rsp);
			break;

		/* == SWO Commands == */

		/* DAP_SWO_Transport */
		case 0x17:
		/* DAP_SWO_Mode */
		case 0x18:
		/* DAP_SWO_Baudrate */
		case 0x19:
		/* DAP_SWO_Control */
		case 0x1A:
		/* DAP_SWO_Status */
		case 0x1B:
		/* DAP_ExtendedStatus */
		case 0x1E:
		/* DAP_SWO_Data */
		case 0x1C:
			log_puts("CMSIS: SWO command ");
			log_puthex(req.buffer[0], 8);
			log_puts(" not supported yet.\r\n");
			rsp.buffer[1] = 0xFF;
			rsp.len = 2;
			result = 0;
			break;

		/* == Unsorted Commands (SWD, JTAG, Transfer ...) == */

		/* DAP_TransferConfigure */
		case 0x04:
			result = dap_transfer_configure(&req, &rsp);
			break;
		/* DAP_Transfer */
		case 0x05:
			result = dap_transfer(&req, &rsp);
			break;
		/* DAP_TransferBlock */
		case 0x06:
		/* DAP_TransferAbort */
		case 0x07:
			result = -1;
			break;
	}

	if (result == 0)
	{
		if (rsp.len <= 0)
		{
			tx_buffer[0] = req.buffer[0]; // Copy command ID
			tx_buffer[1] = 0xFF;          // DAP_ERROR
			rsp.len = 2;
		}
		usbd_edpt_xfer(0, ep_in_n, tx_buffer, rsp.len);
	}
	else
	{
		log_puts("CMSIS: dap_recv() :\r\n");
		for (i = 0; i < len; i++)
		{
			log_puthex(rx[i], 8);
			log_puts(" ");
		}
		log_puts("\r\n");
	}

}

/**
 * @brief Handle the DAP_Connect command
 *
 * This command is used to establish an electrical connection with the target.
 * Into argument, the protocol to use (SWD or JTAG) is also specified for a
 * correct pin configuration.
 *
 * @param rep Pointer to the request packet
 * @param rsp Pointer to a packet where response can be stored
 * @return integer On success zero is returned, -1 for error
 */
static inline int dap_connect(cmsis_pkt *req, cmsis_pkt *rsp)
{
#ifdef DEBUG_CMSIS
	/* Sanity check */
	if ((req == 0) || (rsp == 0))
		return(-1);

	log_puts("CMSIS: Connect ");
	log_puthex(req->buffer[1], 8);
	log_puts("\r\n");
#endif

	/* If request port is SWD (or Default) */
	if ((req->buffer[1] == 1) || (req->buffer[1] == 0))
	{
		cmsis_mode = 1;
		ios_mode(PORT_MODE_SWD);
		swd_config.retry_count = dap_retry_wait;
		/* Response: cmsis mode is now SWD */
		rsp->buffer[1] = 0x01;
	}
	/* If request port is JTAG */
	else if (req->buffer[1] == 2)
	{
		cmsis_mode = 2;
		ios_mode(PORT_MODE_JTAG);
		/* Response: cmsis mode is now JTAG */
		rsp->buffer[1] = 0x02;
	}
	/* For all other ports, Initialization Failed */
	else
		rsp->buffer[1] = 0x00;

	rsp->buffer[0] = 2;
	rsp->len = 2;

	return(0);
}

/**
 * @brief Handle the DAP_Delay command
 *
 * This command is used to wait for a specific delay (in micro-seconds)
 *
 * TODO This command is not implemented yet
 *
 * @param rep Pointer to the request packet
 * @param rsp Pointer to a packet where response can be stored
 * @return integer On success zero is returned, -1 for error
 */
static inline int dap_delay(cmsis_pkt *req, cmsis_pkt *rsp)
{
#ifdef DEBUG_CMSIS
	/* Sanity check */
	if ((req == 0) || (rsp == 0))
		return(-1);
#endif
	log_puts("CMSIS: Delay (not supported yet)\r\n");

	rsp->buffer[1] = 0xFF; // ERROR
	rsp->len = 2;
	return(0);
}

/**
 * @brief Handle the DAP_Disconnect command
 *
 * This command is used to release the IOs of the debug port.
 *
 * @param rep Pointer to the request packet
 * @param rsp Pointer to a packet where response can be stored
 * @return integer On success zero is returned, -1 for error
 */
static inline int dap_disconnect(cmsis_pkt *req, cmsis_pkt *rsp)
{
#ifdef DEBUG_CMSIS
	/* Sanity check */
	if ((req == 0) || (rsp == 0))
		return(-1);

	log_puts("CMSIS: Disconnect\r\n");
#endif
	ios_mode(PORT_MODE_HIZ);

	rsp->buffer[1] = 0x00; // OK
	rsp->len = 2;
	return(0);
}

/**
 * @brief Handle the DAP_HostStatus command
 *
 *
 * @param rep Pointer to the request packet
 * @param rsp Pointer to a packet where response can be stored
 * @return integer On success zero is returned, -1 for error
 */
static inline int dap_host_status(cmsis_pkt *req, cmsis_pkt *rsp)
{
#ifdef DEBUG_CMSIS
	/* Sanity check */
	if ((req == 0) || (rsp == 0))
		return(-1);

	log_puts("CMSIS: HostStatus\r\n");
#endif

	rsp->buffer[1] = 0x00; // OK
	rsp->len = 2;

	return(0);
}

/**
 * @brief Handle the DAP_Info command
 *
 * This commands are used by the host software to get informations about the
 * cmsis probe itself and about the target. There is a long list of available
 * informations so this function only decode the identifier of the request and
 * branch to other dedicated functions (below).
 *
 * @param rep Pointer to the request packet
 * @param rsp Pointer to a packet where response can be stored
 * @return integer On success zero is returned, -1 for error
 */
static inline int dap_info(cmsis_pkt *req, cmsis_pkt *rsp)
{
	char *str = 0;
	int len;
	int result = 0;

	switch (req->buffer[1])
	{
		/* Vendor Name (string) */
		case 0x01:
			goto ret_str;
		/* Product Name (string) */
		case 0x02:
			goto ret_str;
		/* Serial Number (string) */
		case 0x03:
			str = str_serial;
			goto ret_str;
		/* CMSIS-DAP Protocol Version */
		case 0x04:
			str = str_version;
			goto ret_str;
		/* Target Device Vendor */
		case 0x05:
		/* Target Device Name */
		case 0x06:
		/* Target Board Vendor */
		case 0x07:
		/* Target Board Name */
		case 0x08:
		/* Product Firmware Version */
		case 0x09:
			goto ret_str;
		/* Capabilities of the Debug Unit */
		case 0xF0:
			result = dap_info_cap(req, rsp);
			break;
		/* Test Domain Timer */
		case 0xF1:
			rsp->buffer[1] = 0x08;
			/* Return 0 for the timer freq */
			rsp->buffer[2] = 0x00;
			rsp->buffer[3] = 0x00;
			rsp->buffer[4] = 0x00;
			rsp->buffer[5] = 0x00;
			rsp->len = 6;
			break;
		/* UART Receive Buffer Size */
		case 0xFB:
		/* UART Transmit Buffer Size */
		case 0xFC:
		/* SWO Trace Buffer Size */
		case 0xFD:
			rsp->buffer[1] = 0x04; /* Len */
			goto ret_word;
		/* Packet Count */
		case 0xFE:
			rsp->buffer[1] = 1; // Response size
			rsp->buffer[2] = 1; // Packet count = 1
			rsp->len = 3;
			break;
		/* Packet Size */
		case 0xFF:
			rsp->buffer[1] = 2;    // Response size
			rsp->buffer[2] = 0x40; // Packet size = 64
			rsp->buffer[3] = 0x00;
			rsp->len = 4;
			break;

		/* Unknown or unsupported command */
		default:
			result = -1;
	}
	return(result);

/* Generic code to return a WORD */
ret_word:
	rsp->buffer[2] = 0x00;
	rsp->buffer[3] = 0x00;
	rsp->buffer[4] = 0x00;
	rsp->buffer[5] = 0x00;
	rsp->len = 6;
	return(result);
/* Generic code to return a string */
ret_str:
	if (str == 0)
	{
		rsp->buffer[1] = 0;
		rsp->len = 2;
		return(result);
	}

	len = strlen(str);
	/* Insert header */
	rsp->buffer[1] = len + 1;
	/* Copy string into response packet */
	strcpy((char *)(rsp->buffer + 2), str);
	rsp->buffer[2+len] = 0; // Add a nul char to finish string
	rsp->len = (2 + len + 1);
	return(result);
}

/**
 * @brief Handle Dap_Info::Capabilities command
 *
 * @return integer On success zero is returned, -1 for error
 */
static inline int dap_info_cap(cmsis_pkt *req, cmsis_pkt *rsp)
{
#ifdef DEBUG_CMSIS
	/* Sanity check */
	if ((req == 0) || (rsp == 0))
		return(-1);

	log_puts("CMSIS: Get Capabilities\r\n");
#endif
	rsp->buffer[1] = 1;
	rsp->buffer[2] = (1 << 0) | // SWD is supported
	                 (1 << 1);  // JTAG is supported
	rsp->len = 3;
	return(0);
}

/**
 * @brief Handle DAP_ResetTarget command
 *
 * This command request a target reset with device specific sequence.
 *
 * TODO This command is not implemented yet
 *
 * @param rep Pointer to the request packet
 * @param rsp Pointer to a packet where response can be stored
 * @return integer On success zero is returned, -1 for error
 */
static inline int dap_reset_target(cmsis_pkt *req, cmsis_pkt *rsp)
{
#ifdef DEBUG_CMSIS
	/* Sanity check */
	if ((req == 0) || (rsp == 0))
		return(-1);
#endif
	log_puts("CMSIS: ResetTarget (not supported yet)\r\n");

	/* Inform the host that this command is known but not implemented */
	rsp->buffer[1] = 0x00; /* Command status OK */
	rsp->buffer[2] = 0x00; /* Execute: 0 = not implemented */
	rsp->len = 3;

	return(0);
}

/**
 * @brief Handle DAP_SWD_Configure command
 *
 * This command is used to set configuration parameters specific to SWD
 * interface (like turnaround period or data phase).
 *
 * @param rep Pointer to the request packet
 * @param rsp Pointer to a packet where response can be stored
 * @return integer On success zero is returned, -1 for error
 */
static inline int dap_swd_configure(cmsis_pkt *req, cmsis_pkt *rsp)
{
#ifdef DEBUG_CMSIS
	/* Sanity check */
	if ((req == 0) || (rsp == 0))
		return(-1);
#endif
	/* Extract new SWD configuration values */
	dap_ta_period  = ((req->buffer[1] & 0x03) + 1);
	dap_data_phase =  (req->buffer[1] & 4) ? 1 : 0;

#ifdef DEBUG_CMSIS
	log_puts("DAP: Configure SWD,");
	log_puts(" TA_period="); log_putdec(dap_ta_period);
	log_puts(" DataPhase="); log_putdec(dap_data_phase);
	log_puts("\r\n");
#endif

	rsp->buffer[1] = 0x00; // OK
	rsp->len = 2;
	return(0);
}

/**
 * @brief Handle DAP_SWD_Sequence command
 *
 * This command is used to generate special sequences in SWD mode on the pins
 * SWDCLK and/or SWDIO.
 *
 * @param rep Pointer to the request packet
 * @param rsp Pointer to a packet where response can be stored
 * @return integer On success zero is returned, -1 for error
 */
static inline int dap_swd_sequence(cmsis_pkt *req, cmsis_pkt *rsp)
{
	uint seq_count;
	u8  *p, *q;
	int tck_count;
	int in_out;
	int len_out = 0;
	int i, j, l;
#ifdef DEBUG_CMSIS
	/* Sanity check */
	if ((req == 0) || (rsp == 0))
		return(-1);
#endif
	/* Extract number of sequences */
	seq_count = req->buffer[1];

#ifdef DEBUG_CMSIS_SEQ
	log_puts("DAP_SWD_Sequence: count="); log_putdec(seq_count);
#endif

	p = (req->buffer + 2);
	q = (rsp->buffer + 2);
	for (i = 0; i < seq_count; i++)
	{
		/* Extract number of TCK clock cycle */
		tck_count = (p[0] & 0x3F);
		if (tck_count == 0)
			tck_count = 64;

		// If sequence direction is specified as input
		if (p[0] & 0x80)
		{
#ifdef DEBUG_CMSIS_SEQ
			log_puts(" IN("); log_putdec(tck_count); log_puts(")");
#endif
			// Force SWD-IO pin to input
			swd_io_dir(IO_DIR_IN);
			// Read the specified number of bits
			for (j = tck_count; j > 0; j -= l)
			{
				l = (j >= 8) ? 8 : j;
				*q++ = swd_rd(l);
			}
			p++;
		}
		// Sequence direction is specified as output
		else
		{
#ifdef DEBUG_CMSIS_SEQ
			log_puts(" OUT("); log_putdec(tck_count); log_puts(")");
#endif
			p++;
			// Force SWD-IO pin to output
			swd_io_dir(IO_DIR_OUT);
			// Write the specified number of bits
			for (j = tck_count; j > 0; j -= l)
			{
				l = (j >= 8) ? 8 : j;
				swd_wr(*p++, l);
			}
		}
	}
#ifdef DEBUG_CMSIS_SEQ
	log_puts("\r\n");
#endif
	rsp->buffer[1] = 0x00; // OK
	rsp->len = (q - rsp->buffer);
	swd_io_dir(IO_DIR_OUT);

	return(0);
}

/**
 * @brief Handle DAP_SWJ_Clock command
 *
 * This command is used to set the clock frequency of the bus (common to SWD
 * and JTAG modes).
 *
 * @param rep Pointer to the request packet
 * @param rsp Pointer to a packet where response can be stored
 * @return integer On success zero is returned, -1 for error
 */
static inline int dap_swj_clock(cmsis_pkt *req, cmsis_pkt *rsp)
{
#ifdef DEBUG_CMSIS
	/* Sanity check */
	if ((req == 0) || (rsp == 0))
		return(-1);
#endif

	cmsis_clock = (req->buffer[1] << 8) | req->buffer[2];

#ifdef DEBUG_CMSIS
	log_puts("CMSIS: Set clock ");
	log_puthex(cmsis_clock, 16);
	log_puts("\r\n");
#endif

	rsp->buffer[1] = 0x00; // OK
	rsp->len = 2;
	return(0);
}

/**
 * @brief Handle DAP_SWJ_Pins command
 *
 * This command is used to monitor and control the IOs pins including
 * reset lines.
 *
 * @param rep Pointer to the request packet
 * @param rsp Pointer to a packet where response can be stored
 * @return integer On success zero is returned, -1 for error
 */
static inline int dap_swj_pins(cmsis_pkt *req, cmsis_pkt *rsp)
{
	uint8_t output;
	uint8_t select;
	uint8_t wait;
	uint8_t sig;

#ifdef DEBUG_CMSIS
	/* Sanity check */
	if ((req == 0) || (rsp == 0))
		return(-1);

	log_puts("CMSIS: Set DAP_SWJ pins\r\n");
#endif
	output = req->buffer[1];
	select = req->buffer[2];
	wait   = req->buffer[3];

	/* Bit0: TCK/SWD-CLK */
	if (select & (1 << 0))
	{
		sig = (output & (1 << 0)) ? 1 : 0;
		ios_pin_set(PORT_D2_PIN, sig);
	}
	/* Bit1: TMS/SWD-DAT */
	if (select & (1 << 1))
	{
		sig = (output & (1 << 1)) ? 1 : 0;
		ios_pin_set(PORT_D1_PIN, sig);
	}
	/* Bit3: TDO */
	if (select & (1 << 3))
	{
		/* TDO signal available only in JTAG mode */
		if (cmsis_mode == 2)
		{
			sig = (output & (1 << 3)) ? 1 : 0;
			ios_pin_set(PORT_D3_PIN, sig);
		}
	}
	/* Bit5: nTRST */
	if (select & (1 << 5))
	{
		/* nTRST is not available */
	}
	/* Bit7: nReset */
	if (select & (1 << 7))
	{
		/* Reset signal available only in SWD mode */
		if (cmsis_mode == 1)
		{
			sig = (output & (1 << 7)) ? 1 : 0;
			ios_pin_set(PORT_D3_PIN, sig);
		}
	}
	/* TODO: Handle wait argument */
	(void)wait;

	/* Insert current IOs values into response */
	rsp->buffer[1] = (ios_pin(PORT_D1_PIN) << 1) |
	                 (ios_pin(PORT_D2_PIN) << 0);
	if (cmsis_mode == 1)
		rsp->buffer[1] |= (ios_pin(PORT_D3_PIN) << 7);
	else if (cmsis_mode == 2)
		rsp->buffer[1] |= (ios_pin(PORT_D3_PIN) << 3);

	rsp->len = 2;
	return(0);
}

/**
 * @brief Handle DAP_SWJ_Sequence command
 *
 * This command is used to send a sequence of bits without taking care about
 * input value or state of the target. This allow (for example) to send bit
 * pattern for SWD/JTAG reset or SWD->JTAG transition.
 *
 * @param rep Pointer to the request packet
 * @param rsp Pointer to a packet where response can be stored
 * @return integer On success zero is returned, -1 for error
 */
static inline int dap_swj_sequence(cmsis_pkt *req, cmsis_pkt *rsp)
{
	unsigned char *p, v;
	int bit_count, bit_sent, bit_rem;
	int len, wait;

#ifdef DEBUG_CMSIS
	/* Sanity check */
	if ((req == 0) || (rsp == 0))
		return(-1);
#endif
	/* Extract bit count from the first field of the request */
	bit_count = (req->buffer[1] == 0) ? 256 : req->buffer[1];

#ifdef DEBUG_CMSIS
	log_puts("DAP: SWJ_Sequence");
	log_puts(" bit_count="); log_putdec(bit_count);
	log_puts("\r\n");
#endif

	p = (req->buffer + 2);

	for (bit_sent = 0; bit_sent < bit_count ; )
	{
		/* Extract next byte */
		v = *p;
		bit_rem = (bit_count - bit_sent);
		len = (bit_rem > 8) ? 8 : bit_rem;
		swd_wr(v, len);
		/* Update counter of processed bits */
		bit_sent += len;
		p++;
	}

	/* Sequence complete ! prepare response */
	rsp->buffer[1] = 0x00; // OK
	rsp->len = 2;

	return(0);
}

/**
 * @brief Handle DAP_Transfer command
 *
 * This command is used to read or write data to CoreSight registers. Each
 * access is for a 32bits value.
 *
 * @param rep Pointer to the request packet
 * @param rsp Pointer to a packet where response can be stored
 * @return integer On success zero is returned, -1 for error
 */
static inline int dap_transfer(cmsis_pkt *req, cmsis_pkt *rsp)
{
	int count, pos, pos_resp;
	int request, ack;
	unsigned long data;
	int rd_posted = 0;
	int wr_rd = 0;
	int i;

#ifdef DEBUG_CMSIS
	/* Sanity check */
	if ((req == 0) || (rsp == 0))
		return(-1);
#endif

	count = req->buffer[2];

#ifdef DEBUG_CMSIS_TR
	log_puts("CMSIS: DAP Transfer with ");
	log_putdec(count);
	log_puts(" requests\r\n");
#endif
	pos_resp = 3;
	for (i = 0, pos = 2; i < count; i++)
	{
		wr_rd = 0;

		request = req->buffer[++pos];

		if (rd_posted)
		{
			/* In case of another read on AP, after a posted read */
			if ( (request & (1 << 1)) && (request & (1 << 0)) )
				ack = swd_transfer(request, &data);
			else
			{
				ack = swd_transfer(0x0C | (1 << 1), &data);
				rd_posted = 0;
			}
			if (ack != 1)
				break;

			rsp->buffer[pos_resp + 0] = ((data >>  0) & 0xFF);
			rsp->buffer[pos_resp + 1] = ((data >>  8) & 0xFF);
			rsp->buffer[pos_resp + 2] = ((data >> 16) & 0xFF);
			rsp->buffer[pos_resp + 3] = ((data >> 24) & 0xFF);
			pos_resp += 4;

			if (rd_posted)
				continue;
		}
		/* In case of a read on AP, insert an extra read cycle */
		if ( (request & (1 << 1)) && (request & (1 << 0)) )
		{
			ack = swd_transfer(request, &data);
			if (ack != 1)
				break;
			rd_posted = 1;
		}
		/* If RnW bit is set, read request */
		else if (request & (1 << 1))
		{
			ack = swd_transfer(request, &data);
			if (ack == 1)
			{
				rsp->buffer[pos_resp + 0] = ((data >>  0) & 0xFF);
				rsp->buffer[pos_resp + 1] = ((data >>  8) & 0xFF);
				rsp->buffer[pos_resp + 2] = ((data >> 16) & 0xFF);
				rsp->buffer[pos_resp + 3] = ((data >> 24) & 0xFF);
				pos_resp += 4;
			}
		}
		/* RnW is clear, Write request */
		else
		{
			/* Extract data to write from request */
			data  = (req->buffer[pos+4] << 24);
			data |= (req->buffer[pos+3] << 16);
			data |= (req->buffer[pos+2] <<  8);
			data |= (req->buffer[pos+1] <<  0);
			pos += 4;

			ack = swd_transfer(request, &data);
			if (ack == 1)
				wr_rd = 1;
		}
		if (ack != 1)
			break;
	}

	if (ack == 1)
	{
		if (rd_posted)
		{
			ack = swd_transfer(0x0C | (1 << 1), &data);
			if (ack == 1)
			{
				rsp->buffer[pos_resp + 0] = ((data >>  0) & 0xFF);
				rsp->buffer[pos_resp + 1] = ((data >>  8) & 0xFF);
				rsp->buffer[pos_resp + 2] = ((data >> 16) & 0xFF);
				rsp->buffer[pos_resp + 3] = ((data >> 24) & 0xFF);

				pos_resp += 4;
			}
		}
		else if (wr_rd)
			ack = swd_transfer(0x0C | (1 << 1), 0);
	}

	/* Make response header */
	rsp->buffer[1] = i;   /* Number of transfer */
	rsp->buffer[2] = ack; /* Status of last transfer */
	rsp->len = pos_resp;

	return(0);
}

/**
 * @brief Handle DAP_TransferConfigure command
 *
 * This command is used to set some parameters that will be used for next
 * DAP_Transfer and DAP_TransferBlock commands.
 *
 * @param rep Pointer to the request packet
 * @param rsp Pointer to a packet where response can be stored
 * @return integer On success zero is returned, -1 for error
 */
static inline int dap_transfer_configure(cmsis_pkt *req, cmsis_pkt *rsp)
{
#ifdef DEBUG_CMSIS
	/* Sanity check */
	if ((req == 0) || (rsp == 0))
		return(-1);
#endif
	/* Extract new Transfer configuration values */
	dap_idle_cycles = req->buffer[1];
	dap_retry_wait  = (req->buffer[3] << 8) | req->buffer[2];
	dap_retry_match = (req->buffer[5] << 8) | req->buffer[4];

	swd_config.retry_count = dap_retry_wait;

#ifdef DEBUG_CMSIS
	log_puts("DAP: Configure transfer:");
	log_puts(" IdleCycles="); log_putdec(dap_idle_cycles);
	log_puts(" RetryWait=");  log_putdec(dap_retry_wait);
	log_puts(" RetryMatch="); log_putdec(dap_retry_match);
	log_puts("\r\n");
#endif

	rsp->buffer[0] = 0x04;
	rsp->buffer[1] = 0x00; // OK
	rsp->len = 2;

	return(0);
}

/**
 * @brief Handle DAP_WriteABORT command
 *
 * This command is used write an abort request into the ABORT register
 * of the target. This command should only be used when something really
 * wrong happens during a transfer that must be interrupted.
 *
 * TODO This command is not implemented yet
 *
 * @param rep Pointer to the request packet
 * @param rsp Pointer to a packet where response can be stored
 * @return integer On success zero is returned, -1 for error
 */
static inline int dap_write_abort(cmsis_pkt *req, cmsis_pkt *rsp)
{
#ifdef DEBUG_CMSIS
	/* Sanity check */
	if ((req == 0) || (rsp == 0))
		return(-1);
#endif
	log_puts("CMSIS: WriteABORT not supported yet\r\n");

	rsp->buffer[1] = 0xFF; // ERROR
	rsp->len = 2;
	return(0);
}

/* -------------------------------------------------------------------------- */
/* --                         TinyUSB class driver                         -- */
/* -------------------------------------------------------------------------- */

/**
 * @brief TinyUSB class driver init
 *
 * This function is called by TinyUSB stack when the cmsis class driver is
 * registered into the list of custom drivers (see usb.c for descriptors)
 */
void cmsis_usb_init(void)
{
#ifdef DEBUG_CMSIS_USB
	log_puts("cmsis_usb_init()\r\n");
#endif
	ep_in_n  = 0;
	ep_out_n = 0;
}

/**
 * @brief TinyUSB class driver "open" function
 *
 * This function is called by TinyUSB during the SET_CONFIGURATION step of the
 * enumeration when trying to find a valid class driver for each available
 * interfaces. If a cmsis interface is defined, this function will match.
 */
uint16_t cmsis_usb_open(uint8_t rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len)
{
	const tusb_desc_endpoint_t *p_desc_ep;
	uint8_t const *p_desc;
	uint16_t drv_len;
	uint8_t ep_n;

	(void)rhport;

#ifdef DEBUG_CMSIS_USB
	log_puts("cmsis_usb_open()\r\n");
#endif

	if (itf_desc->bInterfaceNumber != TUD_ITF_CMSIS)
		return(0);

	drv_len = sizeof(tusb_desc_interface_t);

	/* Search next descriptor (should be endpoint OUT) */
	p_desc = tu_desc_next(itf_desc);
	if (tu_desc_type(p_desc) == TUSB_DESC_ENDPOINT)
	{
		p_desc_ep = (const tusb_desc_endpoint_t *)p_desc;
		ep_n = p_desc_ep->bEndpointAddress;
		if (usbd_edpt_open(rhport, p_desc_ep) == 0)
		{
			log_puts("CMSIS: failed to activate endpoint.\r\n");
			goto err;
		}
		if (usbd_edpt_claim(rhport, ep_n) == 0)
		{
			log_puts("CMSIS: Open failed #1\r\n");
			goto err;
		}
		ep_out_n = ep_n;
		usbd_edpt_xfer(rhport, ep_out_n, rx_buffer, RX_SIZE);
		drv_len += tu_desc_len(p_desc);
	}
	else
		goto err;

	/* Search next descriptor (should be endpoint IN) */
	p_desc = tu_desc_next(p_desc);
	if (tu_desc_type(p_desc) == TUSB_DESC_ENDPOINT)
	{
		p_desc_ep = (const tusb_desc_endpoint_t *)p_desc;
		if (usbd_edpt_open(rhport, p_desc_ep) == 0)
		{
			log_puts("CMSIS: failed to activate endpoint.\r\n");
			return(0);
		}
		ep_in_n = p_desc_ep->bEndpointAddress;
		drv_len += tu_desc_len(p_desc);
	}
	else
		goto err;

#ifdef DEBUG_CMSIS_USB
	log_puts("CMSIS: Found\r\n");
#endif

	if (drv_len > max_len)
	{
		log_puts("CMSIS: Error into usb_open() : max_len\r\n");
		return(0);
	}

	return(drv_len);
err:
	ep_in_n  = 0;
	ep_out_n = 0;
	return(0);
}

/**
 * @brief TinyUSB class driver "reset" function
 *
 * This function is called by TinyUSB when a class driver is no more used
 * and can be revert to a "reset" state.
 */
void cmsis_usb_reset(uint8_t rhport)
{
#ifdef DEBUG_CMSIS_USB
	log_puts("cmsis_usb_reset()\r\n");
#endif
}

/**
 * @brief TinyUSB class driver "control transfer" function
 *
 * This function is called by TinyUSB when a control transfer is requested on
 * an interface attached to the cmsis class driver.
 */
bool cmsis_usb_ctl(uint8_t rhport, uint8_t stage, tusb_control_request_t const* req)
{
#ifdef DEBUG_CMSIS_USB
	log_puts("cmsis_usb_ctl()\r\n");
#endif
}

/**
 * @brief TinyUSB class driver "transfer" function
 *
 * This function is called by TinyUSB when a transfer is made on an endpoint
 * managed by cmsis class driver.
 */
bool cmsis_usb_xfer(uint8_t rhport, uint8_t ep, xfer_result_t result, uint32_t xferred_bytes)
{
	int i;

#ifdef DBG_XFER
	log_puts("cmsis_usb_xfer()");
	log_puts(" ep=");     log_puthex(ep, 8);
	log_puts(" result="); log_puthex(result, 32);
	log_puts(" len=");    log_puthex(xferred_bytes, 16);
	log_puts("\r\n");
#endif

	if (ep == ep_out_n)
	{
		/* Call DAP to process received command */
		dap_recv(rx_buffer, xferred_bytes);

		/* Prepare endpoint for next transfer */
		usbd_edpt_xfer(rhport, ep_out_n, rx_buffer, 256);
	}
	else if (ep == ep_in_n)
	{
		/* Do nothing :) */
	}
	else
		/* Unknown endpoint ?! */
		return(0);

	return(1);
}
#endif
/* EOF */
