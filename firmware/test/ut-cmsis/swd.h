/**
 * @file  swd.h
 * @brief Headers for the test functions of SWD interface
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
#ifndef SWD_H
#define SWD_H

int swd_connect(cmsis_env *env);
int swd_dpidr(cmsis_env *env);
int swd_j2s(cmsis_env *env);
int swd_reset(cmsis_env *env);

#endif
