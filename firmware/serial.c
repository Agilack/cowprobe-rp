/**
 * @file  serial.c
 * @brief Handle communication with UART interface
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
#include "serial.h"

static void serial_irq(void);
static void tx_send_next(uart_hw_t *dev);

static uint8_t rx_buffer[SERIAL_RX_SZ];
static volatile int rx_index_r, rx_index_w;
static uint8_t tx_buffer[SERIAL_TX_SZ];
static volatile int tx_index_r, tx_index_w;

/**
 * @brief Initialize the "serial" module
 *
 * This function initialize the serial module and configure UART interface. For
 * this driver to work properly, this function must be called before any other
 * serial functions.
 */
void serial_init(void)
{
	/* Initialize indexes of buffers */
	rx_index_r = 0;
	rx_index_w = 0;
	tx_index_r = 0;
	tx_index_w = 0;

	uart_init(uart1, 115200);

	gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

	/* Set default/initial UART configuration */
	uart_set_hw_flow(uart1, false, false);
	uart_set_format (uart1, 8, 1, UART_PARITY_NONE);
	uart_set_fifo_enabled(uart1, false);

	/* Configure interrupts */
	irq_set_exclusive_handler(UART1_IRQ, serial_irq);
	irq_set_enabled(UART1_IRQ, true);
	uart_set_irq_enables(uart1, true, false);
}

/**
 * @brief Read bytes from rx buffer
 *
 * Bytes received from UART are handled by interrupt (see serial_irq) and put
 * into a buffer. This function allow to extract bytes from this rx buffer.
 *
 * @param buffer Pointer to a buffer where to put data
 * @param len    Maximum number of bytes to read
 * @return integer Number of readed bytes
 */
int serial_read(char *buffer, int len)
{
	int count = 0;

	/* Sanity check */
	if (buffer == 0)
		return(0);

	while (count < len)
	{
		/* If the rx buffer is empty, nothing more to do */
		if (rx_index_r == rx_index_w)
			break;

		/* Read one byte from rx buffer */
		*buffer++ = rx_buffer[rx_index_r];
		count++;
		/* Update index */
		rx_index_r++;
		if (rx_index_r == SERIAL_RX_SZ)
			rx_index_r = 0;
	}
	return(count);
}

/**
 * @brief Get the number of bytes available into receive buffer
 *
 * @return integer Number of received bytes
 */
int serial_rx_avail(void)
{
	int r_copy, w_copy;
	int len;

	if (rx_index_r == rx_index_w)
		return(0);

	/* Save a copy of indexes */
	r_copy = rx_index_r;
	w_copy = rx_index_w;

	if (w_copy > r_copy)
		len = (w_copy - r_copy);
	else
		len = (SERIAL_RX_SZ - r_copy) + w_copy;

	return(len);
}

/**
 * @brief Set the uart line coding parameters (speed, parity, ...)
 *
 * @param bits   Number of data bits per byte (default 8)
 * @param stop   Number of stop bits (default 1)
 * @param parity Parity bit format (default 0 for none)
 * @param speed  Port speed in bits per second
 */
void serial_set_format(int bits, int stop, int parity, int speed)
{
	/* Check limits for number of data bits argument */
	if (bits < 5) bits = 5;
	if (bits > 8) bits = 8;
	/* Check limits for number of stop bits argument */
	if (stop < 1) stop = 1;
	if (stop > 2) stop = 2;
	/* Check parity argument */
	if (parity == 1)
		parity = UART_PARITY_ODD;
	else if (parity == 2)
		parity = UART_PARITY_EVEN;
	else
		parity = UART_PARITY_NONE;

	uart_set_baudrate(uart1, speed);
	uart_set_format  (uart1, bits, stop, parity);
}

/**
 * @brief Send bytes to UART
 *
 * @param data Pointer to a buffer with data to send
 * @param len  Number of bytes to send
 */
void serial_write(uint8_t *data, int len)
{
	uart_hw_t *dev;
	int idx_next;
	int wait;

	/* Sanity check */
	if (data == 0)
		return;

	dev = uart_get_hw(uart1);

	while (len > 0)
	{
		/* Put next byte into transmit fifo */
		tx_buffer[tx_index_w] = *data++;
		len--;

		/* Update index */
		idx_next = tx_index_w + 1;
		if (idx_next == SERIAL_TX_SZ)
			idx_next = 0;
		/* If fifo is full, wait ... */
		if (idx_next == tx_index_r)
		{
			wait = 0;
			while (idx_next == tx_index_r)
			{
				/* If UART is stuck for a too long time, abort */
				if (++wait == 10000)
					goto err_timeout;
			}
		}
		tx_index_w = idx_next;

		if ((dev->imsc & (1 << 5)) == 0)
		{
			/* Re-enable TX interrupt */
			dev->imsc |= (1 << 5);
			/* And send next byte */
			tx_send_next(dev);
		}
	}
	return;

err_timeout:
	/* Notify ? */
	return;
}

/**
 * @brief UART interrupt handler
 *
 * This function is called when an interrupt is raised by the UART peripheral
 * (mainly on received byte event)
 */
static void serial_irq(void)
{
	uart_hw_t *dev;
	uint32_t ris, dr;
	uint8_t c;
	int idx_next;

	dev = uart_get_hw(uart1);

	/* Get the Raw Interrupt Status */
	ris = dev->ris;

	/* Receive Timeout is not really used, disable it */
	if (ris & (1 << 6))
		dev->imsc &= ~(1 << 6);

	/* If one byte has been received */
	if (ris & UART_UARTRIS_RXRIS_BITS)
	{
		c = (dev->dr & 0xFF);
		rx_buffer[rx_index_w] = c;

		/* Compute index of next byte into circular buffer */
		idx_next = rx_index_w + 1;
		if (idx_next == SERIAL_RX_SZ)
			idx_next = 0;
		/* If buffer is not full, update write index */
		if (idx_next != rx_index_r)
			rx_index_w = idx_next;
	}
	/* If the transmit register is empty */
	if (ris & UART_UARTRIS_TXRIS_BITS)
	{
		/* If there is more bytes to send, process next */
		if (tx_index_w != tx_index_r)
			tx_send_next(dev);
		/* TX buffer is empty, end of transmission */
		else
		{
			/* Clear interrupt (TXIC) */
			dev->icr = (1 << 5);
			/* And disable it (TXIM) */
			dev->imsc &= ~(1 << 5);
		}
	}
}

/**
 * @brief Send the next available byte from TX buffer
 *
 */
static void tx_send_next(uart_hw_t *dev)
{
	uint8_t c;
	int     idx_next;

	if (tx_index_w == tx_index_r)
		return;

	/* Get next byte to send from TX buffer */
	c = tx_buffer[tx_index_r];
	/* It is possible to update index now because byte has been get */
	idx_next = tx_index_r + 1;
	if (idx_next == SERIAL_TX_SZ)
		idx_next = 0;
	tx_index_r = idx_next;

	/* Write byte to UART transmit register */
	dev->dr = c;
}
/* EOF */
