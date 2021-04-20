/* FlexCAN.c              (c) 2018 NXP
 */

/*******************************************************************************
* Includes
********************************************************************************/
#include "FlexCAN.h"


/**********************************************************************************************
* Global variables
**********************************************************************************************/




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
void FLEXCAN0_init(uint8_t canfd_enable)
{
	uint32_t   i=0;
	PCC->PCCn[PCC_FlexCAN0_INDEX] |= PCC_PCCn_CGC_MASK; /* CGC=1: enable clock to FlexCAN0 */
	CAN0->MCR |= CAN_MCR_MDIS_MASK;                     /* MDIS=1: Disable module before selecting clock */
	if(canfd_enable==0)
	{
		CAN0->CTRL1 &= ~CAN_CTRL1_CLKSRC_MASK;           /* CLKSRC=0: Clock Source = oscillator (8 MHz) */
	}
	else
	{
		CAN0->CTRL1 |=CAN_CTRL1_CLKSRC_MASK;             /* CLKSRC=1: Clock Source = BUSCLK (80 MHz) */
	}
	CAN0->MCR &= ~CAN_MCR_MDIS_MASK;                  /* MDIS=0; Enable module config. (Sets FRZ, HALT)*/
	while (!((CAN0->MCR & CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT))
	{
		/* Good practice: wait for FRZACK=1 on freeze mode entry/exit */
	}

	if(canfd_enable==0)

	CAN0->CTRL1 = 0x00DB0006;        /* Configure for 500 KHz bit time */
							         /* Time quanta freq = 16 time quanta x 500 KHz bit time= 8MHz */
							         /* PRESDIV+1 = Fclksrc/Ftq = 8 MHz/8 MHz = 1 */
							         /* so PRESDIV = 0 */
							         /* PSEG2 = Phase_Seg2 - 1 = 4 - 1 = 3 */
							         /* PSEG1 = PSEG2 = 3 */
							         /* PROPSEG= Prop_Seg - 1 = 7 - 1 = 6 */
							         /* RJW: since Phase_Seg2 >=4, RJW+1=4 so RJW=3. */
							         /* SMP = 1: use 3 bits per CAN sample */
							         /* CLKSRC=0 (unchanged): Fcanclk= Fosc= 8 MHz */
	else
	{
		CAN0->CBT = 0x802FBDEF;     /* Configure nominal phase: 500 KHz bit time, 80 MHz Sclock */
									/* Prescaler = CANCLK / Sclock = 80 MHz / 40 MHz = 2 */
									/* EPRESDIV = Prescaler - 1 = 1 */
									/* EPSEG2 = 15 */
									/* EPSEG1 = 15 */
									/* EPROPSEG = 47 */
									/* ERJW = 15 */
		/* BITRATEn =Fcanclk /( [(1 + (EPSEG1+1) + (EPSEG2+1) + (EPROPSEG + 1)] x (EPRESDIV+1))
		*          = 40 MHz /( [(1 + ( 15   +1) + ( 15   +1) + (   46    + 1)] x (    0   +1))
		*          = 40 MHz /( [1+16+16+47] x 1) = 40 MHz /(80x1) = 500 kHz
		*/

		CAN0->FDCBT = 0x00131CE3;    /* Configure data phase: 2 MHz bit time, 80 MHz Sclock */
									/* Prescaler = CANCLK / Sclock = 80 MHz / 40 MHz = 1 */
									/* FPRESDIV = Prescaler - 1 = 1 */
									/* FPSEG2 = 3 */
									/* FPSEG1 = 7 */
									/* FPROPSEG = 7 */
									/* FRJW = 3 */
		/* BITRATEf =Fcanclk /( [(1 + (FPSEG1+1) + (FPSEG2+1) + (FPROPSEG)] x (FPRESDIV+!)) */
		/*          = 40 MHz /( [(1 + (  7   +1) + (  3   +1) + (   7    )] x (    0   +1)) */
		/*          = 40 MHz /( [1+8+4+7] x 1) = 40 MHz /(20x1) = 40 MHz / 20 = 2 MHz  */

		CAN0->FDCTRL =0x80038500;   /* Configure bit rate switch, data size, transcv'r delay  */
									/* BRS=1: enable Bit Rate Swtich in frame's header */
									/* MBDSR1: Not applicable */
									/* MBDSR0=3: Region 0 has 64 bytes data in frame's payload */
									/* TDCEN=1: enable Transceiver Delay Compensation */
									/* TDCOFF=5: 5 CAN clocks (300us) offset used */

	}

	for(i=0; i<128; i++ )             /* CAN0: clear 128 words RAM in FlexCAN 0 */
	{
		CAN0->RAMn[i] = 0;            /* Clear msg buf words. All buffers CODE=0 (inactive) */
	}
	for(i=0; i<16; i++ )              /* In FRZ mode, init CAN0 16 msg buf filters */
	{
		CAN0->RXIMR[i] = 0xFFFFFFFF;  /* Check all ID bits for incoming messages */
	}
	CAN0->RXMGMASK = 0x1FFFFFFF;      /* Global acceptance mask: check all ID bits */

	if(canfd_enable==0)
	{
		 	 	 	 	 	 	 	  /* PRIO = 0: CANFD not used */
		CAN0->MCR = 0x0000001F;       /* Negate FlexCAN 1 halt state for 32 MBs */
	}
	else
	{
		CAN0->CTRL2 |= CAN_CTRL2_ISOCANFDEN_MASK;       /* Enable CRC fix for ISO CAN FD */
		CAN0->MCR = 0x0000081F;                       /* Negate FlexCAN 1 halt state & enable CAN FD for 32 MBs */
	}

	while ((CAN0->MCR && CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT)
	{
		 /* Good practice: wait for FRZACK to clear (not in freeze mode) */
	}
	while ((CAN0->MCR && CAN_MCR_NOTRDY_MASK) >> CAN_MCR_NOTRDY_SHIFT)
	{
		 /* Good practice: wait for NOTRDY to clear (module ready)  */
	}
}


void deinit_FLEXCAN0 (void)
{
	uint32_t   i=0;
	CAN0->MCR |= CAN_MCR_MDIS_MASK;         /* MDIS=1: Disable module before selecting clock */
	CAN0->CTRL1 = 0x00000000;              /*Reset value CAN CTRL1 */

	CAN0->CBT = 0x00000000;     /*Reset value CAN CBT*/
	CAN0->FDCBT = 0x00000000;  /* Reset value CAN FDCBT*/
	CAN0->FDCTRL =0x80000100;  /* Reset value CAN FDCTRL*/

	for(i=0; i<128; i++ )       /* CAN0: clear 128 words RAM in FlexCAN 0 */
	{
		CAN0->RAMn[i] = 0;       /* Clear msg buf words. All buffers CODE=0 (inactive) */
	}

	CAN0->CTRL2 |= 0x00A0000;       /*Reset value CAN0 CTRL2 */
	CAN0->MCR = 0xD890000F;       /* Reset Value CAN0 MCR */

	PCC->PCCn[PCC_FlexCAN0_INDEX] &= ~PCC_PCCn_CGC_MASK; /* CGC=0: disable clock to FlexCAN0 */
}


void FLEXCAN0_transmit_msg(CAN_TXbuffer *CAN_TXbuffer_ptr,uint8_t canfd_frame)
{
	uint8_t data_words=0;
	uint8_t tx_word_cntr=0;

	if (CAN_TXbuffer_ptr->TxLENGTH <5)
	{
		data_words=1;
	}
	else if(CAN_TXbuffer_ptr->TxLENGTH <9)
	{
		data_words=2;
	}
	else if(9==CAN_TXbuffer_ptr->TxLENGTH)
	{
		data_words=3;
	}
	else if(10==CAN_TXbuffer_ptr->TxLENGTH)
	{
		data_words=4;
	}
	else if(11==CAN_TXbuffer_ptr->TxLENGTH)
	{
		data_words=5;
	}
	else if(12==CAN_TXbuffer_ptr->TxLENGTH)
	{
		data_words=6;
	}
	else if(13==CAN_TXbuffer_ptr->TxLENGTH)
	{
		data_words=8;
	}
	else if(14==CAN_TXbuffer_ptr->TxLENGTH)
	{
		data_words=12;
	}
	else if(15==CAN_TXbuffer_ptr->TxLENGTH)
	{
		data_words=16;
	}


	for(tx_word_cntr=0;tx_word_cntr<data_words;tx_word_cntr++)
	{
		CAN0->RAMn[ CAN_TXbuffer_ptr->MB*CAN_TXbuffer_ptr->MBSize + MB_WORD_2+tx_word_cntr] = CAN_TXbuffer_ptr->TxDATA[tx_word_cntr]; /* MB0 word 2: data word 0 */
	}

	CAN0->RAMn[ CAN_TXbuffer_ptr->MB*CAN_TXbuffer_ptr->MBSize + MB_WORD_1] = CAN_TXbuffer_ptr->TxID<<ID_STANDARD_SHIFT; /*MB ID= TxID */

	if (0==canfd_frame)
	{
		CAN0->RAMn[ CAN_TXbuffer_ptr->MB*CAN_TXbuffer_ptr->MBSize + MB_WORD_0] = 0x0C400000 |CAN_WMBn_CS_DLC(CAN_TXbuffer_ptr->TxLENGTH); /* MB0 word 0: */
																																	/* EDL,BRS,ESI=0: CANFD not used */
																																	/* CODE=0xC: Activate msg buf to transmit */
																																	/* IDE=0: Standard ID */
																																	/* SRR=1 Tx frame (not req'd for std ID) */
																																	/* RTR = 0: data, not remote tx request frame*/
																																	/* DLC = TxLENGTH */
	}
	else
	{
		CAN0->RAMn[ CAN_TXbuffer_ptr->MB*CAN_TXbuffer_ptr->MBSize + MB_WORD_0] = 0xCC400000 |CAN_WMBn_CS_DLC(CAN_TXbuffer_ptr->TxLENGTH);   /* MB word 0: */
																																		/* EDL,BRS,ESI=1: CANFD not used */
																																		/* CODE=0xC: Activate msg buf to transmit */
																																		/* IDE=0: Standard ID */
																																		/* SRR=1 Tx frame (not req'd for std ID) */
																																		/* RTR = 0: data, not remote tx request frame*/
																																		/* DLC = TxLENGTH */
	}

	CAN0->IFLAG1 = (uint32_t)(1 << CAN_TXbuffer_ptr->MB);			/* Clear MB flag without clearing others */
	while(0 == ( (CAN0->IFLAG1 >> CAN_TXbuffer_ptr->MB) & 1 )); 	/* Wait to transmit data */
}


CAN_RXbuffer FLEXCAN0_receive_msg(uint8_t rx_mb,uint8_t mb_size,uint8_t canfd_frame)
{
	uint8_t rx_word_cntr=0;
	uint8_t rx_words=0;
	CAN_RXbuffer CAN_Rx_Data= {{0}};

	CAN_Rx_Data.data.RxCODE   = (CAN0->RAMn[ rx_mb*mb_size + MB_WORD_0] & 0x07000000) >> 24;     							/* Read CODE field */
	CAN_Rx_Data.data.RxID     = (CAN0->RAMn[ rx_mb*mb_size + MB_WORD_1] & CAN_WMBn_ID_ID_MASK)  >> ID_STANDARD_SHIFT ;  	/* Read frame ID */
	CAN_Rx_Data.data.RxLENGTH = (CAN0->RAMn[ rx_mb*mb_size + MB_WORD_0] & CAN_WMBn_CS_DLC_MASK) >> CAN_WMBn_CS_DLC_SHIFT;	/* Read frame DLC */
	if (CAN_Rx_Data.data.RxLENGTH <5)
	{
		rx_words=1;
	}
	else if(CAN_Rx_Data.data.RxLENGTH <9)
	{
		rx_words=2;
	}
	else if(9==CAN_Rx_Data.data.RxLENGTH)
	{
		rx_words=3;
	}
	else if(10==CAN_Rx_Data.data.RxLENGTH)
	{
		rx_words=4;
	}
	else if(11==CAN_Rx_Data.data.RxLENGTH)
	{
		rx_words=5;
	}
	else if(12==CAN_Rx_Data.data.RxLENGTH)
	{
		rx_words=6;
	}
	else if(13==CAN_Rx_Data.data.RxLENGTH)
	{
		rx_words=8;
	}
	else if(14==CAN_Rx_Data.data.RxLENGTH)
	{
		rx_words=12;
	}
	else if(15==CAN_Rx_Data.data.RxLENGTH)
	{
		rx_words=16;
	}
	if(0==canfd_frame)
	{
		for (rx_word_cntr=0; rx_word_cntr<rx_words; rx_word_cntr++)
		{
		CAN_Rx_Data.data.RxDATA[rx_word_cntr] = CAN0->RAMn[ rx_mb*mb_size + MB_WORD_2 + rx_word_cntr];
		}
	}
	else
	{
		for (rx_word_cntr=0; rx_word_cntr<rx_words; rx_word_cntr++)
		{
			CAN_Rx_Data.data.RxDATA[rx_word_cntr] = CAN0->RAMn[ rx_mb*mb_size + MB_WORD_2 + rx_word_cntr];
		}
	}
	CAN_Rx_Data.data.RxTIMESTAMP = (CAN0->RAMn[ rx_mb*mb_size + MB_WORD_0] & 0x000FFFF);
	(void)CAN0->TIMER;				/* Read TIMER to unlock message buffers */
	CAN0->IFLAG1 = (uint32_t)(1 << rx_mb);	/* Clear MB flag without clearing others */
	return CAN_Rx_Data;
}

uint8_t FlexCAN0_check_crc(uint8_t canfd_enable )
{
	if(1==canfd_enable)
	{
		return (CAN0->ESR1>>CAN_ESR1_CRCERR_FAST_SHIFT)&0x01;
	}
	else
	{
		return (CAN0->ESR1>>CAN_ESR1_CRCERR_SHIFT)&0x01;
	}
}
