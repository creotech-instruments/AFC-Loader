/*
 * rtm.h
 *
 *   AFCIPMI  --
 *
 *   Copyright (C) 2015  Marcin Krzewski  <marcin.krzewski@creotech.pl>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IPMI_RTM_H_
#define IPMI_RTM_H_

#include <stdint.h>
#include "FreeRTOS.h"

bool rtm_check_presence(void);
bool rtm_check_compatibility(void);
bool rtm_check_handle(void);
bool rtm_io_init(void);
void rtm_led_ctrl(uint8_t led_id, uint8_t on);
void rtm_write_eeprom(uint16_t addr, uint8_t* buf, uint8_t len);
void rtm_read_eeprom(uint16_t addr, uint8_t* buf, uint8_t len);

bool RTM_init(void);
void vTaskRTM(void *pvParmeters);

#endif //IPMI_RTM_H_
