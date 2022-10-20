/**
 * @file  swd.h
 * @brief Headers and definitions for SWD interface
 *
 * @authors Saint-Genest Gwenael <gwen@cowlab.fr>
 *          Blot Alexandre <alexandre.blot@agilack.fr>
 *          Jousseaume Florent <florent.jousseaume@agilack.fr>
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
#ifndef SWD_H
#define SWD_H
#include "types.h"

typedef struct swd_param_s
{
	uint retry_count;
} swd_param;

extern swd_param swd_config;

int  swd_connect(void);
int  swd_disconnect(void);

int  swd_transfer(u8 req, u32 *value);
/* Low level SWD functions */
void swd_idle(void);
void swd_io_dir(int dir);
u32  swd_rd(uint len);
void swd_turna(int dir);
void swd_wr(u32 value, uint len);

#endif
