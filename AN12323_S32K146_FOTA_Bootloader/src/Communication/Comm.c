/*
 * Comm.c
 *
 *  Created on: Jul 18, 2017
 *      Author: B46911
 */

/*******************************************************************************
* Includes
********************************************************************************/
#include "Comm.h"
#include "Array.h"
#include "Bootloader_cfg.h"
#include "Secure_Comm.h"
/**********************************************************************************************
* Global variables
**********************************************************************************************/
CAN_TXbuffer CAN_ackdata;
CAN_RXbuffer CAN_Rx_Message;
uint8_t canfd_en=1;

/**********************************************************************************************
* Constants and macros
**********************************************************************************************/
/**********************************************************************************************
* Local types
**********************************************************************************************/


/**********************************************************************************************
* Local function prototypes
*********************************************************************************************/

/**********************************************************************************************
* Local variables
**********************************************************************************************/


/**********************************************************************************************
* Local functions
**********************************************************************************************/


/**********************************************************************************************
* Global functions
**********************************************************************************************/
void Communication_Init (void)
{
	FLEXCAN0_init(canfd_en);
	SET_MB_RX_FD(CAN_RX_MB,MB_SIZE);  									   /* Set MB #4 for FD reception */
	SET_MB_ID(CAN_RX_MB,MB_SIZE,GATEWAY_START_ID);						   /* Set MB #4 ID */
	Clean_Array(&CAN_Rx_Message.RxBuff[0],16);
	Clean_Array(&CAN_ackdata.TxDATA[0],16);
	CAN_ackdata.MB=CAN_TX_MB;                                              /* Set MB for transmission */
	CAN_ackdata.MBSize=MB_SIZE;                                            /* Set the size of MB in FlexCAN */
	CAN_ackdata.TxID=EDGE_NODE_ID;                                         /* Set ID for ACK frame */
	CAN_ackdata.TxLENGTH=ACK_PAYLOAD_SIZE;                                 /* Set ACK frame DLC size */
}

void Communication_deinit(void)
{
	deinit_FLEXCAN0();
}

void Comm_Transmit_Ack (uint8_t err_flag)
{
	if(1==err_flag)
	{
		CAN_ackdata.TxDATA[0]=ERROR_PAYLOAD;
		FLEXCAN0_transmit_msg(&CAN_ackdata,canfd_en);    /* Transmit ERR frame */
	}
	else if(0==err_flag)
	{
		CAN_ackdata.TxDATA[0]=ACK_PAYLOAD;
		#if CANFD_BOOTLOADER && CSEC_ENABLE
		// Generate RND for ACK
		Generate_Rnd_Ack_Msg();
		#endif
		Clean_Array(&CAN_Rx_Message.RxBuff[0],PAYLOAD_SIZE_WORDS);
		FLEXCAN0_transmit_msg(&CAN_ackdata,canfd_en);   /* Transmit ACK frame */
	}
}

uint8_t Comm_CRC_Verif(void)
{
	return FlexCAN0_check_crc(canfd_en);
}

uint8_t Comm_New_Msg_Rx (void)
{
	return ( (CAN0->IFLAG1 >> CAN_RX_MB) & RX_NEW_MSG );
}

void Comm_Rx_Data (void)
{
	CAN_Rx_Message=FLEXCAN0_receive_msg(CAN_RX_MB,MB_SIZE,canfd_en);  /* Read message */
}

