/**
 * @file  dap_general.h
 * @brief Headers for the test functions of general DAP commands
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
#ifndef DAP_GENERAL_H
#define DAP_GENERAL_H
#include "test.h"

int tst_connect(cmsis_env *env);
int tst_delay(cmsis_env *env);
int tst_disconnect(cmsis_env *env);
int tst_host_status(cmsis_env *env);
int tst_reset_target(cmsis_env *env);
int tst_write_abort(cmsis_env *env);

#endif
