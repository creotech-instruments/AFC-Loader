/*
 * payload.c
 *
 *   AFCIPMI  --
 *
 *   Copyright (C) 2015  Piotr Miedzik <P.Miedzik@gsi.de>
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

#if USE_FREERTOS == 1
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#endif

#include "ipmi/payload.h"
#include "ipmi/sdr.h"
#include "ipmi/ipmi_oem.h"

#include "ic/ic_ADN4604.h"
#include <afc/board_version.h>

/* payload states
 *   0 - no power
 *   1 - power switching on
 *       sekwencja włączania zasilania
 *
 *   2 - power good wait
 *       Od momentu zakończenia włączania zasilań
 *       do momentu wykrycia power good
 *
 *   3 - power good
 *       Tutaj konfigurowanie urządzeń takich jak clock crossbar i inne
 *       We have to reset pin state program b
 *
 *   4 - fpga booting
 *       Od momentu zezwolenia na bootowanie
 *       Do momentu wykrycia DONE
 *       about 30 sec
 *
 *   5 - fpga working
 *
 *
 *   6 - power switching off
 *       Sekwencja wyłączania zasilania
 *
 *   7 - power QUIESCED
 *       trwa do momentu zaniku napięcia na linii 12v
 *       lub przez 30 sekund (???)
 *
 * 255 - power fail
 */


void setDC_DC_ConvertersON(bool on) {
	bool _on = on;
	//_on = false;

	// @todo: check vadj relationship
	bool _on_fmc1 = false | on;
	bool _on_fmc2 = false | on;
	bool _on_rtm = false | on;
	//Chip_GPIO_SetPinDIR(LPC_GPIO, 0, 17, true);
	//if (!on)
//		Chip_GPIO_SetPinState(LPC_GPIO, 0, 17, on );


        Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_FMC1_PVADJ_PORT, GPIO_EN_FMC1_PVADJ_PIN, _on_fmc1);
        Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EM_FMC1_P12V_PORT, GPIO_EM_FMC1_P12V_PIN, _on_fmc1);
        Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_FMC1_P3V3_PORT, GPIO_EN_FMC1_P3V3_PIN, _on_fmc1);

        Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_FMC2_PVADJ_PORT, GPIO_EN_FMC2_PVADJ_PIN, _on_fmc2);
        Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EM_FMC2_P12V_PORT, GPIO_EM_FMC2_P12V_PIN, _on_fmc2);
        Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_FMC2_P3V3_PORT, GPIO_EN_FMC2_P3V3_PIN, _on_fmc2);

        // P1V0 - VCCINT, MGTAVCC, VCCBRAM
        // P1V2 - MGTAVTT
        // P1V8 - VCCAUX, MGTVCCAUX
        // P2V0 - VCCAUX_IO
        // P1V5 - VCCO_34, 33
        // PVADJ1 - VCCO_18, 17, 16, 15
        // PVADJ2 - VCCO_12, 13, 14, 32
        // P3V3 - VCCO_0
        Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_P1V0_PORT, GPIO_EN_P1V0_PIN, _on);
        Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_P1V2_PORT, GPIO_EN_P1V2_PIN, _on);
        Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_P1V8_PORT, GPIO_EN_P1V8_PIN, _on); // <- this one causes problems if not switched off before power loss


        Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_1V5_VTT_PORT, GPIO_EN_1V5_VTT_PIN, _on);
        Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_P3V3_PORT, GPIO_EN_P3V3_PIN, _on);

//        if(_on_rtm && !Chip_GPIO_ReadPortBit(LPC_GPIO, GPIO_PS_RTM_PORT, GPIO_PS_RTM_PIN))
//          Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_RTM_PORT, GPIO_EN_RTM_PIN, _on_rtm);
//        else
//          Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_RTM_PORT, GPIO_EN_RTM_PIN, false);
//if (on)
//		Chip_GPIO_SetPinState(LPC_GPIO, 0, 17, on );
}

void initializeDCDC() {
        SPI_CONFIG_FORMAT_T spi_format;
        SPI_DATA_SETUP_T spi_x_data;
        uint16_t spi_tx_buf = 0x0128;
        uint8_t spi_rx_buf[2];

	setDC_DC_ConvertersON(false);
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_P1V2_PORT, GPIO_EN_P1V2_PIN, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_P1V8_PORT, GPIO_EN_P1V8_PIN, true);

	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_FMC2_P3V3_PORT, GPIO_EN_FMC2_P3V3_PIN, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_FMC2_PVADJ_PORT, GPIO_EN_FMC2_PVADJ_PIN, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EM_FMC2_P12V_PORT, GPIO_EM_FMC2_P12V_PIN, true);

	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EM_FMC1_P12V_PORT, GPIO_EM_FMC1_P12V_PIN, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_FMC1_P3V3_PORT, GPIO_EN_FMC1_P3V3_PIN, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_FMC1_PVADJ_PORT,	GPIO_EN_FMC1_PVADJ_PIN, true);

	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_P3V3_PORT, GPIO_EN_P3V3_PIN, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_1V5_VTT_PORT, GPIO_EN_1V5_VTT_PIN, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_P1V0_PORT, GPIO_EN_P1V0_PIN, true);

        // Set VADJ DAC
        Chip_GPIO_SetPinDIR(LPC_GPIO, 0, 21, true);
        Chip_GPIO_SetPinState(LPC_GPIO, 0, 21, true);

        /* Set up clock and muxing for SSP0 interface */
        /*
         * Initialize SSP0 pins connect
         * P0.15: SCK
         * P0.16: SSEL
         * P0.18: MOSI
         */
//        Chip_IOCON_PinMux(LPC_IOCON, 0, 15, IOCON_MODE_PULLUP, IOCON_FUNC3);
//        Chip_IOCON_PinMux(LPC_IOCON, 0, 16, IOCON_MODE_PULLUP, IOCON_FUNC0);
//        Chip_IOCON_PinMux(LPC_IOCON, 0, 18, IOCON_MODE_INACT, IOCON_FUNC3);
//
//        Chip_GPIO_SetPinDIR(LPC_GPIO, 0, 16, true);
//        Chip_GPIO_SetPinState(LPC_GPIO, 0, 16, true);

//        Chip_SPI_Init(LPC_SPI);
        // Setup VADJ to 2.5 V;
        spi_format.bits = SPI_BITS_10;
        spi_format.clockMode = SPI_CLOCK_CPHA0_CPOL0;
        spi_format.dataOrder = SPI_DATA_MSB_FIRST;
        Chip_SPI_SetFormat(LPC_SPI, &spi_format);
        Chip_SPI_SetBitRate(LPC_SPI, 10000000);
        Chip_SPI_SetMode(LPC_SPI, SPI_MODE_MASTER);

        spi_x_data.cnt = 0;
        spi_x_data.length = 2;
        spi_x_data.pTxData = (uint8_t*) &spi_tx_buf;
        spi_x_data.pRxData = spi_rx_buf;
        spi_x_data.fnAftFrame = NULL;
        spi_x_data.fnBefFrame = NULL;
        spi_x_data.fnBefTransfer = NULL;
        spi_x_data.fnAftTransfer = NULL;

        Board_SPI_AssertSSEL();
        Chip_SPI_RWFrames_Blocking(LPC_SPI, &spi_x_data);
        Board_SPI_DeassertSSEL();

        spi_x_data.cnt = 0;
        spi_tx_buf = 0x0028;
        Board_SPI_AssertSSEL();
        Chip_SPI_RWFrames_Blocking(LPC_SPI, &spi_x_data);
        Board_SPI_DeassertSSEL();

        // RTM
        Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_RTM_PORT, GPIO_EN_RTM_PIN, true);
}


QueueHandle_t queue_payload_handle = 0;

void payload_send_message(uint8_t msg){
	if (queue_payload_handle == 0)  return;

	xQueueSend(queue_payload_handle, &msg, (TickType_t) 0);

}

extern struct I2C_Mutex i2c_mutex_array[2];


void vTaskPayload(void *pvParmeters) {
// payload
	enum payload_state state = PAYLOAD_NO_POWER;
	enum payload_state new_state = PAYLOAD_STATE_NO_CHANGE;

	I2C_ID_T i2c_bus_id;
	queue_payload_handle = xQueueCreate(16, sizeof(uint8_t));
	uint8_t carrier_type, board_version;

	TickType_t xDelay = 10;

	uint8_t P12V_good = 0;
	uint8_t P1V0_good = 0;
	uint8_t FPGA_boot_DONE = 0;
	uint8_t QUIESCED_req = 0;
#ifdef MMC_CONF_RTM
	enum rtm_state rtm_state = RTM_NO_POWER;
	enum rtm_state rtm_new_state = RTM_NO_POWER;

	uint8_t RTM_PP_on = 0;
	uint8_t RTM_QUIESCED_req = 0;
#endif

	uint8_t current_message;

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	afc_get_board_type(&carrier_type, &board_version);

	// Setup Init_b as input
	//Chip_GPIO_SetPinDIR(LPC_GPIO, 0, 19, false);
	if ((carrier_type == CARRIER_TYPE_AFC && board_version == 0x03) || (carrier_type == CARRIER_TYPE_AFCK)) {
	    Chip_GPIO_SetPinDIR(LPC_GPIO, 0, 20, true);
	    Chip_GPIO_SetPinState(LPC_GPIO, 0, 20, true);
        }

	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_PROGRAM_B_PORT, GPIO_PROGRAM_B_PIN, true);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PROGRAM_B_PORT, GPIO_PROGRAM_B_PIN, true);
	initializeDCDC();

	while(1) {
		new_state = state;
		xDelay = 10;

		while(xQueueReceive(queue_payload_handle, &current_message, (TickType_t) 0 )) {
			if (current_message == PAYLOAD_MESSAGE_P12GOOD) {
				P12V_good = 1;
			} else if (current_message == PAYLOAD_MESSAGE_P12GOODn) {
				P12V_good = 0;
			} else if (current_message == PAYLOAD_MESSAGE_PGOOD) {
				P1V0_good = 1;
			} else if (current_message == PAYLOAD_MESSAGE_PGOODn) {
				P1V0_good = 0;
			} else if (current_message == PAYLOAD_MESSAGE_QUIESCED) {
				QUIESCED_req = 1;
			}
#ifdef MMC_CONF_RTM
			else if (current_message == PAYLOAD_MESSAGE_RTM_PP_ON) {
			        RTM_PP_on = 1;
			} else if (current_message == PAYLOAD_MESSAGE_RTM_QUIESCED) {
			        RTM_QUIESCED_req = 1;
			}
#endif
		}

		FPGA_boot_DONE = Chip_GPIO_GetPinState(LPC_GPIO, GPIO_DONE_B_PORT, GPIO_DONE_B_PIN);
		P1V0_good = Chip_GPIO_GetPinState(LPC_GPIO, GPIO_PGOOD_P1V0_PORT,GPIO_PGOOD_P1V0_PIN);

		switch(state) {
			case PAYLOAD_NO_POWER:
				if (P12V_good == 1) {
					new_state = PAYLOAD_SWITCHING_ON;
					xDelay = 0;
				}
				QUIESCED_req = 0;
				break;
			case PAYLOAD_SWITCHING_ON:
				xDelay = 10;
				//Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PROGRAM_B_PORT, GPIO_PROGRAM_B_PIN, true);
				setDC_DC_ConvertersON(true);
				new_state = PAYLOAD_POWER_GOOD_WAIT;
				break;

			case PAYLOAD_POWER_GOOD_WAIT:
				if (P1V0_good == 1) {
					xDelay = 1000;
					new_state = PAYLOAD_STATE_FPGA_SETUP;
				}

				break;
			case PAYLOAD_STATE_FPGA_SETUP:
				// @todo: things like reconfiguring clock crossbar
				//Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PROGRAM_B_PORT, GPIO_PROGRAM_B_PIN, false);

				//vTaskDelayUntil( &xLastWakeTime, 1000 );
				if (afc_i2c_take_by_chipid(CHIP_ID_ADN, NULL, &i2c_bus_id, 100 ) == pdTRUE) {
					adn4604_setup(i2c_bus_id);
					afc_i2c_give(i2c_bus_id);
					new_state = PAYLOAD_FPGA_BOOTING;
				}

				xDelay = 0;
				//xDelay = 1000;
				// xDelay = 0;
				// Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PROGRAM_B_PORT, GPIO_PROGRAM_B_PIN, true);
				//new_state = PAYLOAD_FPGA_BOOTING;
				break;
			case PAYLOAD_FPGA_BOOTING:
				if (QUIESCED_req == 1) {
					new_state = PAYLOAD_SWITCHING_OFF;
					xDelay = 0;
				} else if (FPGA_boot_DONE) {
					new_state = PAYLOAD_FPGA_WORKING;
					xDelay = 0;
				}
				QUIESCED_req = 0;
				break;
			case PAYLOAD_FPGA_WORKING:
				if (QUIESCED_req == 1) {
					new_state = PAYLOAD_SWITCHING_OFF;
					xDelay = 0;
				} else if (P12V_good == 0) {
					new_state = PAYLOAD_POWER_FAIL;
				}

				QUIESCED_req = 0;
				break;
			case PAYLOAD_SWITCHING_OFF:
				xDelay = 0;
				QUIESCED_req = 0;
				new_state = PAYLOAD_QUIESCED;

				setDC_DC_ConvertersON(false);
				do_quiesced(FRU_CTLCODE_QUIESCE);
				// @todo: emit QUIESCED event
				break;
			case PAYLOAD_QUIESCED:
				if (P12V_good == 0) {
					new_state = PAYLOAD_NO_POWER;
				}
				QUIESCED_req = 0;
				break;
			case PAYLOAD_POWER_FAIL:
				QUIESCED_req = 0;
				break;
			default:
				break;
		}

#ifdef MMC_CONF_RTM
		switch(rtm_state) {
		case RTM_NO_POWER:
		    if(RTM_PP_on == 1) {
		        rtm_new_state = RTM_SWITCHING_ON;
		    }
		    Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_RTM_PORT, GPIO_EN_RTM_PIN, false);
		    RTM_QUIESCED_req = 0;
		    break;

		case RTM_SWITCHING_ON:
		    rtm_new_state = RTM_POWER_GOOD;
		    Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_RTM_PORT, GPIO_EN_RTM_PIN, true);
		    if(xDelay == 0)
		        xDelay = 100;
		    break;

		case RTM_POWER_GOOD:
		    if(RTM_QUIESCED_req == 1)
		        rtm_new_state = RTM_SWITCHING_OFF;
		    else if (P12V_good == 0)
		        rtm_new_state = RTM_NO_POWER;
		    break;

		case RTM_SWITCHING_OFF:
		    rtm_new_state = RTM_QUISCED;
		    RTM_QUIESCED_req = 0;
		    Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_RTM_PORT, GPIO_EN_RTM_PIN, false);
		    do_quiesced(FRU_CTLCODE_RTM_QUIESCE);
		    break;

		case RTM_QUISCED:
		    rtm_new_state = RTM_NO_POWER;
		    RTM_PP_on = 0;

		    break;

		default:
		    break;
		}

		rtm_state = rtm_new_state;
#endif

		// wait only if no
		if (xDelay != 0)
			vTaskDelayUntil( &xLastWakeTime, xDelay );

		state = new_state;

	}
	asm("nop");
}
