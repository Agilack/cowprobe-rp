/**
 * @file  log.h
 * @brief Headers and definitions for log and debug module
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
#ifndef LOG_H
#define LOG_H

void log_init  (void);
void log_puthex(const uint32_t c, const uint8_t len);
void log_puts  (char *s);

#endif
