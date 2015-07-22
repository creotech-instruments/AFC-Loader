/*
 * ipmi_handlers.c
 *
 *   AFCIPMI  --
 *
 *   Copyright (C) 2015  Piotr Miedzik  <P.Miedzik@gsi.de>
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


#include "ipmi_handlers.h"
#include "ipmi.h"
#include "board_api.h"
#include "fru.h"


ipmiProcessFunc ipmi_get_device_id(struct ipmi_msg *req, struct ipmi_msg* rsp)
{
    int len = rsp->msg.data_len;

    rsp->msg_data[len++] = 0x00;                            // Device ID
    rsp->msg_data[len++] = 0x80;                            // Device revision and SDRs
    rsp->msg_data[len++] = MMC_FW_REL_MAJ;                  // Major Firmware revision
    rsp->msg_data[len++] = MMC_FW_REL_MIN;                  // Minor Firmware revision
    rsp->msg_data[len++] = MMC_IPMI_REL;                    // IPMI version 1.5
    rsp->msg_data[len++] = IPMI_MSG_ADD_DEV_SUPP;           // Additional device support(commands and functions)
    rsp->msg_data[len++] = IPMI_MSG_MANU_ID_LSB;            // Manufacturer ID LSB
    rsp->msg_data[len++] = IPMI_MSG_MANU_ID_B2;
    rsp->msg_data[len++] = IPMI_MSG_MANU_ID_MSB;
    rsp->msg_data[len++] = IPMI_MSG_PROD_ID_LSB;            // Product ID LSB
    rsp->msg_data[len++] = IPMI_MSG_PROD_ID_MSB;
    rsp->msg.data_len = len;
    rsp->retcode = IPMI_CC_OK;
}

ipmiProcessFunc ipmi_picmg_get_PROPERTIES(struct ipmi_msg *req, struct ipmi_msg* rsp){
	int len = rsp->msg.data_len;

	    rsp->msg_data[len++] = IPMI_PICMG_GRP_EXT;                            // Device ID
	    rsp->msg_data[len++] = 0x14;                            // per AMC Spec 3.1.5
	    rsp->msg_data[len++] = 0x01;                  // Maximum FRU Device ID
	    rsp->msg_data[len++] = 0x00;                  // FRU Device ID
	    rsp->msg.data_len = len;
	    rsp->retcode = IPMI_CC_OK;
}



ipmiProcessFunc ipmi_picmg_set_fru_led_state(struct ipmi_msg *req, struct ipmi_msg* rsp){
	int len = rsp->msg.data_len;
	rsp->msg_data[len++] = IPMI_PICMG_GRP_EXT;                            // Device ID
	rsp->msg.data_len = len;
    rsp->retcode = IPMI_CC_OK;
}

ipmiProcessFunc ipmi_picmg_get_device_locator_record(struct ipmi_msg *req, struct ipmi_msg* rsp){
	int len = rsp->msg.data_len;
	rsp->msg_data[len++] = IPMI_PICMG_GRP_EXT;                            // Device ID
	rsp->msg_data[len++] = 0;
	rsp->msg_data[len++] = 0;
	rsp->msg.data_len = len;
    rsp->retcode = IPMI_CC_OK;

}


ipmiProcessFunc ipmi_se_set_event_reciever(struct ipmi_msg *req, struct ipmi_msg* rsp){
    rsp->retcode = IPMI_CC_OK;
}

ipmiProcessFunc ipmi_storage_get_fru_info(struct ipmi_msg *req, struct ipmi_msg* rsp){
	int len = rsp->msg.data_len;
	if (req->msg_data[0] == 0) {
		rsp->msg_data[len++] = FRU_SIZE && 0xFF;
		rsp->msg_data[len++] = (FRU_SIZE >> 8 ) & 0xFF;
		//rsp->msg_data[len++] = 0;
		//rsp->msg_data[len++] = 0;

		rsp->msg_data[len++] = 0; // device accessed by bytes
	} else {
		return IPMI_CC_INV_CMD;
	}
	rsp->msg.data_len = len;
    rsp->retcode = IPMI_CC_OK;
}

struct __attribute__((__packed__)) read_fru_data_param {
	unsigned char fru_id;
	unsigned char address[2];
	unsigned char length;

} read_fru_data_param_t;
ipmiProcessFunc ipmi_storage_read_fru_data_cmd(struct ipmi_msg *req, struct ipmi_msg* rsp){
	struct read_fru_data_param * params = (struct read_fru_data_param *)req->msg_data;
	int len = rsp->msg.data_len;

	int address = params->address[0] |  (params->address[1] << 8);
	DEBUGOUT("read message:\r\n");
	DEBUGOUT(" fruid:     %d\r\n", params->fru_id);
	DEBUGOUT(" address_h: %d\r\n", params->address[1]);
	DEBUGOUT(" address_l: %d\r\n", params->address[0]);
	DEBUGOUT(" address: %d\r\n",   address);
	DEBUGOUT(" length:    %d\r\n", params->length);
	fru_read_to_buffer(&rsp->msg_data[len+1], address, params->length);
    rsp->msg_data[len] = params->length; // first byte after retcode is length of returned data
	rsp->msg.data_len += params->length+1;
    rsp->retcode = IPMI_CC_OK;
}

ipmiProcessFunc ipmi_general_invalid(struct ipmi_msg *req, struct ipmi_msg* rsp) {
	rsp->retcode =  IPMI_CC_INV_CMD;
}

