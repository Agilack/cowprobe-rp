/**
 * @file  jtag.h
 * @brief Headers and definitions for JTAG interface
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
#ifndef JTAG_H
#define JTAG_H
#include "types.h"

typedef struct jtag_state_s
{
	char name[16];
	struct jtag_state_s *t0; // Next state when TMS=0
	struct jtag_state_s *t1; // Next state when TMS=1
} jtag_state;

int  jtag_connect(void);
int  jtag_disconnect(void);
void jtag_tms_sequence(u32 seq, uint len);
u32  jtag_shift(u32 value, uint len, uint tms);
u8   jtag_rshift(u8 value, uint len, uint tms);

#endif
