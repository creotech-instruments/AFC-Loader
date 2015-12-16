/*
 * rtm.c
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

#include "rtm.h"
#include <afc/board_version.h>

#define GPIO_PS_RTM_PORT        0
#define GPIO_PS_RTM_PIN        29

#define STATE_URTM_ABSENT       0
#define STATE_URTM_PRSENT       1

#define RTM_CHIP_ID_PCA9554     0x20
#define RTM_CHIP_ID_EEPROM      0x50

#define RTM_IO_BLUE_LED
#define RTM_IO_RED_LED
#define RTM_IO_GREEN_LED


typedef struct {
    bool compatible;
    uint8_t presence;
    uint8_t handle;
    uint8_t led;
} RTM_params;

// Global variables declaration
static RTM_params rtm_status;

// Private functions declaration
//bool rtm_check_presence(void);
//bool rtm_check_compatibility(void);
//bool rtm_check_handle(void);
//bool rtm_io_init(void);
//void rtm_led_ctrl(uint8_t led_id, uint8_t on);
//void rtm_write_eeprom(uint16_t addr, uint8_t* buf, uint8_t len);
//void rtm_read_eeprom(uint16_t addr, uint8_t* buf, uint8_t len);

bool rtm_check_presence(void) {
    bool rtm_pm_state = false;

    Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PS_RTM_PORT, GPIO_PS_RTM_PIN, true);
    rtm_pm_state = Chip_GPIO_ReadPortBit(LPC_GPIO, GPIO_PS_RTM_PORT, GPIO_PS_RTM_PIN);

    Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PS_RTM_PORT, GPIO_PS_RTM_PIN, false);

    if(rtm_pm_state == Chip_GPIO_ReadPortBit(LPC_GPIO, GPIO_PS_RTM_PORT, GPIO_PS_RTM_PIN))
        return true;
    else
        return false;

    //return !Chip_GPIO_ReadPortBit(LPC_GPIO, GPIO_PS_RTM_PORT, GPIO_PS_RTM_PIN);
}

bool rtm_check_compatibility(void) {
    if(rtm_status.presence)
        return true;
    else
        return false;
}

bool rtm_check_handle(void) {
    I2C_ID_T i2c_bus_id;
    uint8_t buf;

    if((rtm_status.presence == STATE_URTM_ABSENT) && !rtm_status.compatible)
        return false;

    if(afc_i2c_take_by_busid(I2C_BUS_RTM_ID, &i2c_bus_id, (TickType_t)100) == pdTRUE) {
        Chip_I2C_MasterCmdRead(i2c_bus_id, RTM_CHIP_ID_PCA9554, 0x00, &buf, 1);
        afc_i2c_give(i2c_bus_id);
    }

    // check handle I/O
    if((buf & 0x1) == 0x1)
        return false;
    else
        return true;
}

bool rtm_io_init(void) {
    I2C_ID_T i2c_bus_id;
    uint8_t buf[2];

    if(!rtm_status.presence)
        return false;

    if(afc_i2c_take_by_busid(I2C_BUS_RTM_ID, &i2c_bus_id, (TickType_t)100) == pdFALSE)
        return false;

    buf[0] = 0x03;
    buf[1] = 0x1F;

    // Setup LED IO as output
    Chip_I2C_MasterSend(i2c_bus_id, RTM_CHIP_ID_PCA9554, buf, 2);

    buf[0] = 0x01;
    buf[1] = 0x60;

    // Set Blue LED
    Chip_I2C_MasterSend(i2c_bus_id, RTM_CHIP_ID_PCA9554, buf, 2);

    afc_i2c_give(i2c_bus_id);

    return true;
}

void rtm_led_ctrl(uint8_t led_id, uint8_t on) {

}

// Public functions definition
bool RTM_init(void) {

    // RTM
    Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_PS_RTM_PORT, GPIO_PS_RTM_PIN, true);
    Chip_GPIO_SetPinDIR(LPC_GPIO, 0, 30, true);
//    Chip_IOCON_PinMux(LPC_IOCON, GPIO_PS_RTM_PORT, GPIO_PS_RTM_PIN, IOCON_MODE_PULLUP, IOCON_FUNC0);
//    Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_RTM_PORT, GPIO_EN_RTM_PIN, true);

    rtm_status.presence = rtm_check_presence();
    rtm_status.compatible = rtm_check_compatibility();
    rtm_io_init();
    rtm_status.handle = rtm_check_handle();

    return true;
}

void vTaskRTM(void *pvParmeters);
