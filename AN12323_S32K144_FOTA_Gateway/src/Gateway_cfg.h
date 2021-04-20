/*
 * Gateway_cfg.h
 *
 *  Created on: Jun 17, 2017
 *      Author: B46911
 */

#ifndef GATEWAY_CFG_H_
#define GATEWAY_CFG_H_

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
#define CAN_COMMUNICATION    0
#define CANFD_COMMUNICATION  1
#define CSEC_ENABLE          1

#define CMAC_GENERATION 1
#define SECURITY_KEYS	1

#define USECASE_CMAC_0  1
#define USECASE_CMAC_1  0

#define MAX_FW_SIZE_KB 			248
#define MAX_FW_SIZE_BYTES		0x3E000

#define FW1_START_ADDRESS  0x2000
#define FW2_START_ADDRESS  0x42000

#define CAN_RX_MB  4		/* CAN MB num for data reception */
#define CAN_TX_MB  0		/* CAN MB num for data transmision */



#define PTC12 12        /* Port PTC12, bit 12: FRDM EVB input from BTN0 [SW2] */
#define PTC13 13        /* Port PTC13, bit 13: FRDM EVB input from BTN1 [SW3] */
#define IMAGE_HEADER_SIZE 0x1C /* 28 bytes */
#define RND_NUM_SIZE_WORDS 4


#define START_FRAME_STATE 0
#define FW_HDR_STATE      1
#define FW_APP_STATE      2
#if USECASE_CMAC_1
#define FW_CMAC_STATE     3
#endif

#if CAN_COMMUNICATION
#define DLC_SIZE_START_FRAME    4   /* 4 BYTES */
#define DLC_SIZE_END_FRAME      4   /* 4 BYTES */
#define DATA_TX_WORDS 2  /* Number of words transmitted */
#define DATA_TX_BYTES 8  /* Number of bytes transmitted */
#define DLC_SIZE 	  8
#define DLC_SIZE_ADDRESS_FRAME  4   /* 4 BYTES  */
#define DLC_SIZE_HDR_DATA_FRAME	8	/* 8 BYTES  */
#define DLC_SIZE_APP_DATA_FRAME	8	/* 8 BYTES  */

#endif

#if CANFD_COMMUNICATION
#define DLC_SIZE_START_FRAME    4   /* 4 BYTES */
#if CSEC_ENABLE
#define DLC_SIZE_ADDRESS_FRAME  13  /* 32 BYTES  */
#define DLC_SIZE_HDR_DATA_FRAME	15	/* 64 BYTES  */
#if USECASE_CMAC_0
#define DLC_SIZE_APP_DATA_FRAME		15	/* 64 BYTES  */
#endif
#if USECASE_CMAC_1
#define DLC_SIZE_APP_DATA_FRAME		14	/* 48 BYTES  */
#define DLC_SIZE_CMAC_FRAME			13  /* 16 BYTES  */
#define CMAC_SIZE_WORDS			     4   /* 4 words*/
#endif
#define DATA_TX_WORDS  			8	/* Number of words transmitted */
#define DATA_TX_BYTES 			32	/* Number of bytes transmitted */
#define DLC_SIZE_END_FRAME      13  /* 32 BYTES */

#else
#define DLC_SIZE_ADDRESS_FRAME  	4   /* 4 BYTES  */
#define DLC_SIZE_APP_DATA_FRAME     13  /* 32 BYTES  */
#define DLC_SIZE_HDR_DATA_FRAME     13  /* 32 BYTES  */
#define DATA_TX_WORDS           	8   /* Number of words transmitted */
#define DATA_TX_BYTES           	32 /* Number of bytes transmitted */
#define DLC_SIZE_END_FRAME      	4   /* 4 BYTES */

#endif
#endif

#define GATEWAY_FW_ADDRESS_ID      0x100
#define GATEWAY_START_ID      	   0x200
#define GATEWAY_FW_DATA_ID         0x300
#define EDGE_NODE_ID               0x400
#define GATEWAY_FW_CMAC_ID         0x700

#define ACK_PAYLOAD                0x04040404
#define START_TRANSMISION_PAYLOAD  0x15151515
#define END_DATA_PAYLOAD           0x53535353
#define ERROR_PAYLOAD              0x55555555

#if CAN_COMMUNICATION
#define MB_SIZE  4    /* Msg Buffer Size. (CAN 2.0AB: 2 hdr +  2 data= 4 words) */
#endif
#if CANFD_COMMUNICATION
#define MB_SIZE  18    /* Msg Buffer Size. (2 words hdr + 16 words data  = 18 words) */
#endif

#if CSEC_ENABLE
#define CMAC_SIZE_WORDS 4
#endif


#endif /* GATEWAY_CFG_H_ */
