/**
 * @file  ios.h
 * @brief Headers and definitions for IOs configuration
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
#ifndef IOS_H
#define IOS_H

#define IO_DIR_IN  0
#define IO_DIR_OUT 1

#define PORT_MODE_HIZ  0
#define PORT_MODE_GPIO 1
#define PORT_MODE_JTAG 2
#define PORT_MODE_SWD  3

/* IOs of the main debug port */
#define UART_TX_PIN 8
#define UART_RX_PIN 9
#define PORT_D0_PIN 19
#define PORT_D0_DIR 18
#define PORT_D1_PIN 17
#define PORT_D1_DIR 16
#define PORT_D2_PIN 15
#define PORT_D2_DIR 14
#define PORT_D3_PIN 10
#define PORT_D3_DIR 11
/* IOs of the internal extension */
#define EXT_01_PIN   7
#define EXT_02_PIN   6
#define EXT_03_PIN   5
#define EXT_04_PIN   4
#define EXT_05_PIN   3
#define EXT_06_PIN   2
#define EXT_07_PIN   1
#define EXT_08_PIN   0
#define EXT_09_PIN  22
#define EXT_10_PIN  23
#define EXT_11_PIN  24
#define EXT_12_PIN  25
#define EXT_13_PIN  26
#define EXT_14_PIN  27
#define EXT_15_PIN  28
#define EXT_16_PIN  29

void ios_init(void);
void ios_mode(int mode);
int  ios_pin (int pin);
void ios_pin_mode(int pin, int mode);
void ios_pin_set (int pin, int state);

#endif
