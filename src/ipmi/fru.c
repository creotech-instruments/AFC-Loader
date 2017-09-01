/*
 * fru.c
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

#include "fru.h"
#include "ipmi.h"

/*
 * fru generated with external tool
 * mainly it says x4 pci-express Gen2
 * without this MCH will not enable FCLK
 */
const char default_fru[136] __attribute__ ((section (".FRU"))) = {
0x01, 0x00, 0x00, 0x01, 0x07, 0x0D, 0x00, 0xEA, 			// common header with CRC
0x01, 0x06, 0x19, /* Manufacturing time */ 0x00, 0x00, 0x00, 						// Board Info header
0xC8, 0x43, 0x72, 0x65, 0x6F, 0x74, 0x65, 0x63, 0x68, // Board manufacturer
0xC3, 0x41, 0x46, 0x43, // Board product name
0xC9, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, // Board serial number
0xC5, 0x41, 0x46, 0x43, 0x76, 0x32, // Board part number
0xC7, 0x43, 0x6F, 0x72, 0x65, 0x46, 0x52, 0x55, // FRU File ID

0xC1, 0x00, 0x00, 0x00, 0xAF, 0x01, 0x06, 0x19, // Product Info header
0xC3, 0x47, 0x53, 0x49, // Manufacturer name
0xC3, 0x41, 0x46, 0x43, // Product name
0xC5, 0x30, 0x30, 0x30, 0x30, 0x31, // Product part
0xC2, 0x76, 0x33, // Product version
0xC5, 0x30, 0x30, 0x30, 0x30, 0x31, // Product serial number
0xC6, 0x4E, 0x6F, 0x20, 0x74, 0x61, 0x67, // Asset Tag
0xC7, 0x43, 0x6F, 0x72, 0x65, 0x46, 0x52, 0x55, // FRU File ID
0xC1, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF9, //
// Module Current Requirements record
0xC0, 0x02, 0x06, 0x45 /*Data CRC:0x1E*/, 0xF3/*HDR CRC: 0x1A*/, 0x5A, 0x31, 0x00, 0x16, 0x00, 0x1A, /*0x41,*/

// Point-to-Point Connectivity record
0xC0, 0x82, 0x10, 0x50, 0x5E, // Multirecord header
0x5A, 0x31, 0x00, //Manufacturer ID (in this case PICMG)
0x19, //PICMG Record ID. For the AMC Point-to-Point Connectivity record, the value 19h must be used.
0x00, //Record Format Version. For this specification, the value 00h must be used.
0x00, //OEM GUID Count. The number, n, of OEM GUIDs defined in this record.
0x80, //[7] Record Type – 1 AMC Module
0x01, //AMC Channel Descriptor Count. The number, m, of AMC Channel Descriptors defined in this record.
0xA4, 0x98, 0xF3, //AMC Channel Descriptors --- port 4, port 5, port 6, port 7
//AMC Link Descriptors
0x00, // Channel ID
0x2F, // Lane 0, Lane 1, Lane 2, Lane 3; AMC.1 PCI Express
0x30, // gen2 SSC
0x00,
0xFD, // AMC Asymmetric Match --- Matches with '10b'
};

#ifdef MMC_CONF_RTM
const char default_rtm_fru[] = {
    0x01, 0x00, 0x00, 0x01, 0x07, 0x0D, 0x00, 0xEA,     // common header with CRC
    0x01, 0x06, 0x19, 0x00, 0x00, 0x00,                 // Board Info header
    0xC8, 'C', 'r', 'e', 'o', 't', 'e', 'c', 'h',       // Board Manufacturer
    0xC3, 'R', 'T', 'M',                                // Board Product Name
    0xC9, '0', '0', '0', '0', '0', '0', '0', '0', '1',  // Board Serial Number
    0xC7, 'R', 'T', 'M', '_', 'S', 'F', 'P',            // Board Part Number
    0xC7, 'C', 'o', 'r', 'e', 'F', 'R', 'U',            // FRU File ID
    0xC1, 0x00, 0x00, 0x00, 0xAF,                       // End of Board Info Area

	// MultiRecord Header
    0xC0, //Record type ID (C0h - OEM)
	0x02, //Record format record
	0x06, //Length
	0x1E, //Record Checksum
	0x1A, //Header CHecksum
	// MultiRecord #1 - Module Current Requirements Record
    0x5A, 0x31, 0x00, //Manufacturer ID (in this case PICMG)
	0x16, //PICMG Record ID. For the Module Power Descriptor table, the value 16h must be used.
	0x00, //Record Format Version. For this specification, the value 0h must be used.
	0x41, //Current Draw. This field holds the Payload Power (PWR) requirement ofthe Module given as current requirement in units of 0.1A at 12V. (Thisequals the value of the power in W divided by 1.2.)
	// MultiRecord Header
	0xC0, 0x82, 0x10, 0xBB, 0xF3,
	// MultiRecord #2 - Point-to-Point Connectivity record (AMC0_Spec, 3.1.9.2)
    0x5A, 0x31, 0x00, 0x19, 0x00, //Manufacturer ID, PICMG Record ID, Record Format Version
	0x00, //OEM GUID Count
	0x80, //AMC Module
	0x01, //AMC Channel Descriptor Count. The number, m, of AMC Channel Descriptors defined in this record.
    0x04, 0x00, 0xF0, //AMC Channel Descriptor. A variable length list of m three-byte AMC Channel Descriptors, each defining the Ports that make up an AMC Channel (least significant byte first).
	0x00, 0x2F, 0x00, 0x00, 0xFD, // AMC Link Descriptor.
};
#endif


// @todo: add support for writing/readint to external memory

void fru_read_to_buffer(char *buff, int offset, int length) {
		int i;
		int j = offset;
		//char *write_buffer = buff;
		//char *read_buffer = default_fru+offset;
		for (i = 0; i<length; i++, j++ ) {
			if (j < sizeof(default_fru)) {
				buff[i] = default_fru[j];
			} else {
				buff[i] = 0xFF;
			}

		}
}

void fru_read_common_header(fru_common_header_t * header) {
	fru_read_to_buffer(header, 0, sizeof(fru_common_header_t));
}
