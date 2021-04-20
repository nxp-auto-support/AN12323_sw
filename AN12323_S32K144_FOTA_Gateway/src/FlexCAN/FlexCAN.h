/* FlexCAN.h              (c) 2018 Freescale Semiconductor, Inc.
 */


#ifndef FLEXCAN_H_
#define FLEXCAN_H_
/*******************************************************************************
* Includes
********************************************************************************/
#include "S32K144.h"


/*******************************************************************************
* Constants
********************************************************************************/

/*******************************************************************************
* Macros
********************************************************************************/
#define ID_STANDARD_SHIFT 18
#define WORD_LENGTH 4
#define MB_WORD_0 0
#define MB_WORD_1 1
#define MB_WORD_2 2


#define SET_MB_RX(mb,mb_size)     CAN0->RAMn[ mb*mb_size + 0]= mb<<24; /* Msg Buf mb, word 0: Enable for reception */
																	/* EDL,BRS,ESI=0: CANFD not used */
																	/* CODE=mb: MB set to RX inactive */
																	/* IDE=0: Standard ID */
																	/* SRR, RTR, TIME STAMP = 0: not applicable */


#define SET_MB_RX_FD(mb,mb_size)     CAN0->RAMn[ mb*mb_size + 0]=0xC4000000;           /* Msg Buf 4, word 0: Enable for reception */
																					/* EDL=1: Extended Data Length for CAN FD */
																					/* BRS = 1: Bit Rate Switch enabled */
																					/* ESI = 0: Error state */
																					/* CODE=4: MB set to RX inactive */
																					/* IDE=0: Standard ID */
																					/* SRR, RTR, TIME STAMP = 0: not applicable */
#define SET_MB_ID(mb,mb_size,id)  CAN0->RAMn[ mb*mb_size + 1]= id<<18;

#define ENABLE_MB_IRQ(mb)		CAN0->IMASK1 |= (1 << mb);

#define DISABLE_MB_IRQ(mb)		CAN0->IMASK1 &= ~(1 << mb);

/*******************************************************************************
* Types
********************************************************************************/


/*******************************************************************************
* Variables
********************************************************************************/
typedef union
{
	uint32_t RxBuff[6]; /* Byte level access to the Phrase */
	struct
	{

		uint32_t  RxCODE;              /* Received message buffer code */
		uint32_t  RxID;                /* Received message ID */
		uint32_t  RxLENGTH;            /* Recieved message number of data bytes */
		uint32_t  RxDATA[16];           /* Received message data (up to 16 words for CANFD) */
		uint32_t  RxTIMESTAMP;         /* Received message time */
	}data;

}CAN_RXbuffer;


typedef struct
{

		uint8_t   MB;                  /* MB for transmission */
		uint8_t   MBSize;               /* MB size*/
		uint32_t  TxID;                /* Transmit message ID */
		uint32_t  TxLENGTH;            /*  DLC*/
		uint32_t  TxDATA[16];           /* Transmit message data (up to 16 words for CANFD) */

}CAN_TXbuffer;

/*******************************************************************************
* Global Functions
********************************************************************************/
void FLEXCAN0_init(uint8_t canfd_enable);
void deinit_FLEXCAN0 (void);
void FLEXCAN0_transmit_msg(CAN_TXbuffer *CAN_TXbuffer_ptr,uint8_t canfd_frame);
CAN_RXbuffer FLEXCAN0_receive_msg(uint8_t rx_mb,uint8_t mb_size,uint8_t can_frame);
uint8_t FlexCAN0_check_crc(uint8_t canfd_enable);

#endif /* FLEXCAN_H_ */
