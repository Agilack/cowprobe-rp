/**
 * @file  dap_info.h
 * @brief Headers for the DAP_Info test functions
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
#ifndef DAP_INFO_H
#define DAP_INFO_H
#include "test.h"

int tst_info_capabilities(cmsis_env *env);
int tst_info_packet_count(cmsis_env *env);
int tst_info_packet_size(cmsis_env *env);
int tst_info_product_name(cmsis_env *env);
int tst_info_protocol_version(cmsis_env *env);
int tst_info_serial(cmsis_env *env);
int tst_info_vendor(cmsis_env *env);

#endif
