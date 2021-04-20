/*
 * Bootloader_cfg.h
 *
 *  Created on: Apr 15, 2017
 *      Author: Osvaldo
 */

#ifndef BOOTLOADER_CFG_H_
#define BOOTLOADER_CFG_H_

/*******************************************************************************
* Includes
********************************************************************************/

/*******************************************************************************
* Constants
********************************************************************************/


/*******************************************************************************
* Macros
********************************************************************************/
/* Define the Communication for the bootloader*/

#define CANFD_BOOTLOADER 1
#define CAN_BOOTLOADER   0
#define CSEC_ENABLE      1
#define BOOT_SECURE      0
#define MEASURE_TIME     0

#define CAN_RX_MB  4		/* CAN MB num for data reception */
#define CAN_TX_MB  0		/* CAN MB num for data transmision */

#define FW_SIZE_BITS       0x1F0000

#define START_VTOR_TABLE	0x00000000
#define END_VTOR_TABLE		0x00000400

#define IMAGE1_HEADER_ADDRESS 0x1000
#define IMAGE2_HEADER_ADDRESS 0x41000


#define FW_BUFFER_SIZE_WORDS 8
#define FW_BUFFER_SIZE_BYTES 32
#define DATA_RX_BYTES		 32
#define DATA_RX_WORDS		 8
#if CSEC_ENABLE
#define PAYLOAD_SIZE_WORDS   16
#else
#define PAYLOAD_SIZE_WORDS   8
#endif

#define GATEWAY_FW_ADDRESS_ID      0x100
#define GATEWAY_START_ID      	   0x200
#define GATEWAY_FW_DATA_ID         0x300
#define EDGE_NODE_ID               0x400

#define ACK_PAYLOAD                0x04040404
#define ERROR_PAYLOAD              0x55555555
#define START_TRANSMISION_PAYLOAD  0x15151515
#define END_DATA_PAYLOAD           0x53535353
#if CSEC_ENABLE
#define ACK_PAYLOAD_SIZE 13 /* 32 bytes */
#else
#define ACK_PAYLOAD_SIZE 4  /* 4 bytes */
#endif
#define MB_SIZE  18    /* Msg Buffer Size. (2 words hdr + 16 words data  = 18 words) */

#define APP_KEY 0x55AA55AA

/*******************************************************************************
* Types
********************************************************************************/


/*******************************************************************************
* Variables
********************************************************************************/


/*******************************************************************************
* Global Variables
********************************************************************************/


/*******************************************************************************
* Global Functions
********************************************************************************/


#endif /* BOOTLOADER_CFG_H_ */
