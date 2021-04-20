
#include "S32K146.h" /* include peripheral declarations S32K146 */
#include "S32K146_features.h"
#include "CSEc_functions.h"
#include "CSEc_macros.h"
#include "clocks_and_modes.h"
#include "FlexCAN.h"
#include "CSEc_functions.h"
#include "CSEc_macros.h"
#include "Gateway_cfg.h"


// The sizes of the firmware binaries are obtained from linker file
extern uint32_t __FW1_SIZE[];
extern uint32_t __FW2_SIZE[];

#if SECURITY_KEYS
uint32_t test_key[4] = {0xc13dd10d,0xbfb5e142,0x316c6d5c,0x2e903ec9};
#endif

#if CSEC_ENABLE
uint16_t __attribute__((unused)) csec_error = 0;
uint32_t cmac_value[4];
uint32_t init_vector[4]={0x00000000,0x00000000,0x00000000,0x00000000};
#endif

volatile uint32_t adress_flash=0x00000000;
volatile uint32_t start_address=0x00000000;

uint32_t flash_data = 0;

uint32_t header_address = 0x00000000;
uint32_t logic_address = 0x00000000;

uint32_t image_size;

uint8_t start_transmission = 0;

CAN_RXbuffer CAN_Rx_Message;

uint32_t image1_header[1024] = {0x00000001,   /* FW version */
								0x00000000,	  /* Oldest fw version */
								0x4e585020,   /* Manufacturer information */
								0x53656d69,
								0x636f6e64,
								0x7563746f,
								0x00000072,
};

uint32_t image2_header[1024] = {0x00000002,   /* FW version */
								0x00000001,	  /* Oldest fw version */
								0x4e585020,   /* Manufacturer information */
								0x53656d69,
								0x636f6e64,
								0x7563746f,
								0x00000072,
};

uint32_t* image_header_ptr;
uint16_t image_header_idx = 0;

void PORT_init (void)
{
	PCC->PCCn[PCC_PORTC_INDEX ] |= PCC_PCCn_CGC_MASK;		/* Enable clock for PORTC */
	PTC->PDDR &= ~(1<<PTC12);								/* Port C12: Data Direction= input (default) */
	PORTC->PCR[12] = (PORT_PCR_MUX(1) | PORT_PCR_PFE_MASK);	/* Port C12: MUX = GPIO, input filter enabled */
	PTC->PDDR &= ~(1<<PTC13);								/* Port C13: Data Direction= input (default) */
	PORTC->PCR[13] = (PORT_PCR_MUX(1) | PORT_PCR_PFE_MASK);	/* Port C13: MUX = GPIO, input filter enabled */

	PORTC->PCR[6] |= PORT_PCR_MUX(2);					/* Port C6: MUX = ALT2,UART1 TX */
	PORTC->PCR[7] |= PORT_PCR_MUX(2);					/* Port C7: MUX = ALT2,UART1 RX */

	PCC->PCCn[PCC_PORTD_INDEX ] |= PCC_PCCn_CGC_MASK;	/* Enable clock for PORTD */
	PORTD->PCR[15] |= PORT_PCR_MUX(1);					/* Port D15: MUX = ALT1,GPIO */
	PTD->PDDR |= 1<<15;									/* Port D15:  Data Direction= output */
	PTD->PSOR |= 1<<15;

	PCC->PCCn[PCC_PORTE_INDEX] |= PCC_PCCn_CGC_MASK;	/* Enable clock for PORTE */
	PORTE->PCR[4] |= PORT_PCR_MUX(5);					/* Port E4: MUX = ALT5, CAN0_RX */
	PORTE->PCR[5] |= PORT_PCR_MUX(5);					/* Port E5: MUX = ALT5, CAN0_TX */
}

void Array_Copy (uint32_t *data_buffer1_ptr, uint32_t *data_buffer2_ptr, uint16_t array_size)
{
	uint8_t cntr=0;
	do
	{
		*data_buffer2_ptr=*data_buffer1_ptr;
		data_buffer2_ptr++;
		data_buffer1_ptr++;
		cntr++;
	}while(cntr<array_size);


}

void Clean_Buffer ( uint32_t *data_buffer_ptr,uint16_t buffer_size)
{
	uint8_t cntr=0;
	do
	{
		*data_buffer_ptr=0x00000000;
		data_buffer_ptr++;
		cntr++;
	}while(cntr<buffer_size);
}

int main(void)
{
	uint16_t rx_msg_count = 0;
	uint16_t i = 0;
	uint8_t fw_transmission_state = START_FRAME_STATE;
	CAN_TXbuffer CAN_data;
	uint8_t canfd_en = 0;

#if CSEC_ENABLE
	csec_error = INIT_RNG();
	LOAD_RAM_KEY(test_key);
#endif

	SCG->FIRCDIV = SCG_FIRCDIV_FIRCDIV2(0b100);	/* FIRCDIV2 enable @ 48MHz/8=6MHz */
	SOSC_init_8MHz();							/* Initialize system oscillator for 8 MHz xtal */
	SPLL_init_160MHz();							/* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	NormalRUNmode_80MHz();						/* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
	LMEM->PCCCR = 0x85000001;					/* Invalidate cache & enable write buffer, cache */
#if CANFD_COMMUNICATION
	canfd_en = 1;
#endif
	FLEXCAN0_init(canfd_en);			/* Init FlexCAN0 */
	SET_MB_RX_FD(CAN_RX_MB, MB_SIZE);	/* Set MB #4 for FD reception */
	SET_MB_ID(CAN_RX_MB, MB_SIZE, EDGE_NODE_ID);
	CAN_data.MB = CAN_TX_MB;
	CAN_data.MBSize = MB_SIZE;
	PORT_init();					/* Initialize Ports*/

	for(;;)
	{
		if (PTC->PDIR & (1<<PTC12))
		{
			/* Wait until button is released */
			while(PTC->PDIR & (1<<PTC12));

			adress_flash = FW1_START_ADDRESS;
			start_address = adress_flash;
			image_header_ptr = image1_header;
			image_size = (uint32_t)__FW1_SIZE;
			start_transmission = 1;
		}

		else if(PTC->PDIR & (1<<PTC13))
		{
			/* Wait until button is released */
			while(PTC->PDIR & (1<<PTC13));

			adress_flash = FW2_START_ADDRESS;
			start_address = adress_flash;
			image_header_ptr = image2_header;
			image_size = (uint32_t)__FW2_SIZE;
			start_transmission = 1;
		}

		while(1 == start_transmission)
		{
			switch (fw_transmission_state)
			{
			case START_FRAME_STATE:
				rx_msg_count = 0;
				CAN_data.TxID = GATEWAY_START_ID;
				CAN_data.TxLENGTH = DLC_SIZE_START_FRAME;
				CAN_data.TxDATA[0] = START_TRANSMISION_PAYLOAD;
				FLEXCAN0_transmit_msg(&CAN_data, canfd_en);
				while(0 == ((CAN0->IFLAG1 >> 4) & 1 ) );
				CAN_Rx_Message = FLEXCAN0_receive_msg (CAN_RX_MB, MB_SIZE, canfd_en);		/* Read message */
				if((CAN_Rx_Message.data.RxID == EDGE_NODE_ID) && (CAN_Rx_Message.data.RxDATA[0] == ACK_PAYLOAD))
				{
					fw_transmission_state = FW_HDR_STATE;
					Clean_Buffer(&CAN_data.TxDATA[0], 16);
				}
				else
				{
					start_transmission = 0; /* Stop transmission */
					break;
				}
				break;

			case FW_HDR_STATE:
				CAN_data.TxID = GATEWAY_FW_ADDRESS_ID;
				CAN_data.TxLENGTH = DLC_SIZE_ADDRESS_FRAME;
				CAN_data.TxDATA[0] = header_address;
#if CSEC_ENABLE
				Array_Copy(&CAN_Rx_Message.data.RxDATA[1], &CAN_data.TxDATA[1], RND_NUM_SIZE_WORDS);	/* Copy of received rnd number to tx payload */
				csec_error=ENC_CBC(CAN_data.TxDATA, init_vector, RAM_KEY, CAN_data.TxDATA, 2);			/* Encrypt payload */
#endif
				FLEXCAN0_transmit_msg(&CAN_data, canfd_en);
				while(0 == ((CAN0->IFLAG1 >> 4) & 1 ) );
				CAN_Rx_Message=FLEXCAN0_receive_msg(CAN_RX_MB, MB_SIZE, canfd_en);		/* Read message */
				/* If received ack from edge node */
				if((CAN_Rx_Message.data.RxID == EDGE_NODE_ID)  && (CAN_Rx_Message.data.RxDATA[0] == ACK_PAYLOAD))
				{
					Clean_Buffer(&CAN_data.TxDATA[0], 16);
					CAN_data.TxID = GATEWAY_FW_DATA_ID;					/* Setup ID for data */
					CAN_data.TxLENGTH = DLC_SIZE_HDR_DATA_FRAME;		/* Update DLC */
				}
				else
				{
					start_transmission = 0;		/* Stop transmission */
					fw_transmission_state = START_FRAME_STATE;
					break;
				}
				Array_Copy(&image_header_ptr[image_header_idx], &CAN_data.TxDATA[0], DATA_TX_WORDS);		/* Fill the payload with the data to send */
				image_header_idx = image_header_idx + DATA_TX_WORDS;

#if CSEC_ENABLE
				Array_Copy(&CAN_Rx_Message.data.RxDATA[1], &CAN_data.TxDATA[12], RND_NUM_SIZE_WORDS);	/* Fill payload with received rnd number*/
				csec_error = CMAC(cmac_value, CAN_data.TxDATA, RAM_KEY, 256);							/* Calculate CMAC for the data to send */
				Array_Copy(&cmac_value[0], &CAN_data.TxDATA[8], CMAC_SIZE_WORDS);						/* Fill payload with cmac*/
				ENC_CBC(CAN_data.TxDATA, init_vector, RAM_KEY, CAN_data.TxDATA, 4);						/* Encrypt payload */
#endif

				FLEXCAN0_transmit_msg(&CAN_data, canfd_en);
				while (0 == ((CAN0->IFLAG1 >> 4) & 1));
				CAN_Rx_Message = FLEXCAN0_receive_msg(CAN_RX_MB, MB_SIZE, canfd_en);		/* Read message */
				if((CAN_Rx_Message.data.RxID == EDGE_NODE_ID)  && (CAN_Rx_Message.data.RxDATA[0] == ACK_PAYLOAD))
				{
					header_address = header_address + DATA_TX_BYTES;
					if(header_address < IMAGE_HEADER_SIZE)
					{
						fw_transmission_state = FW_HDR_STATE;
					}
					else
					{
						Clean_Buffer(&CAN_data.TxDATA[0], 16);
						CAN_data.TxID = GATEWAY_FW_ADDRESS_ID;
						CAN_data.TxLENGTH = DLC_SIZE_END_FRAME;
						CAN_data.TxDATA[0] = END_DATA_PAYLOAD;
#if CANFD_COMMUNICATION && CSEC_ENABLE
						Array_Copy(&CAN_Rx_Message.data.RxDATA[1], &CAN_data.TxDATA[1], RND_NUM_SIZE_WORDS);			/* Copy of received rnd number to tx payload */
						ENC_CBC(CAN_data.TxDATA, init_vector, RAM_KEY, CAN_data.TxDATA, 2);							/* Encrypt payload */
#endif
						FLEXCAN0_transmit_msg(&CAN_data, canfd_en);
						while(0 == ((CAN0->IFLAG1 >> 4) & 1));
						CAN_Rx_Message = FLEXCAN0_receive_msg(CAN_RX_MB, MB_SIZE, canfd_en);		/* Read message */
						if((CAN_Rx_Message.data.RxID == EDGE_NODE_ID)  && (CAN_Rx_Message.data.RxDATA[0] == ACK_PAYLOAD))
						{
							header_address = 0x00000000;
							image_header_idx = 0;
							fw_transmission_state = FW_APP_STATE;
						}
						else
						{
							start_transmission = 0;	/* Stop transmission */
							fw_transmission_state = START_FRAME_STATE;
							break;
						}
					}
				}
				else
				{
					start_transmission = 0;	/* Stop transmission */
					fw_transmission_state = START_FRAME_STATE;
					break;
				}
				break;

			case FW_APP_STATE:
				Clean_Buffer(&CAN_data.TxDATA[0], 16);
				CAN_data.TxID = GATEWAY_FW_ADDRESS_ID;
				CAN_data.TxLENGTH = DLC_SIZE_ADDRESS_FRAME;
				CAN_data.TxDATA[0] = logic_address;
#if CANFD_COMMUNICATION && CSEC_ENABLE
				Array_Copy(&CAN_Rx_Message.data.RxDATA[1], &CAN_data.TxDATA[1], RND_NUM_SIZE_WORDS);	/* Copy of received rnd number to tx payload */
				ENC_CBC(CAN_data.TxDATA, init_vector, RAM_KEY, CAN_data.TxDATA, 2);						/* Add encrypt random number to the payload*/
#endif
				FLEXCAN0_transmit_msg(&CAN_data, canfd_en);
				while(0 == ((CAN0->IFLAG1 >> 4) & 1));
				CAN_Rx_Message = FLEXCAN0_receive_msg (CAN_RX_MB,MB_SIZE,canfd_en);		/* Read message */
				if((CAN_Rx_Message.data.RxID == EDGE_NODE_ID)  && (CAN_Rx_Message.data.RxDATA[0] == ACK_PAYLOAD))
				{
					CAN_data.TxID = GATEWAY_FW_DATA_ID;
					CAN_data.TxLENGTH = DLC_SIZE_APP_DATA_FRAME;
				}
				else
				{
					start_transmission = 0;		/* Stop transmission */
					fw_transmission_state = START_FRAME_STATE;
					break;
				}

				for(i = 0; i < DATA_TX_WORDS; i++)
				{
					flash_data = ((uint32_t)(*(uint32_t*)((adress_flash) + (i*4))));
					CAN_data.TxDATA[i] = flash_data;
				}
#if CANFD_COMMUNICATION && CSEC_ENABLE
#if USECASE_CMAC_0
				Array_Copy(&CAN_Rx_Message.data.RxDATA[1], &CAN_data.TxDATA[12], RND_NUM_SIZE_WORDS);	/* Fill payload with received rnd number*/
				csec_error = CMAC(cmac_value, CAN_data.TxDATA, RAM_KEY, 256);							/* Calculate CMAC to the data to send */
				Array_Copy(&cmac_value[0], &CAN_data.TxDATA[8], CMAC_SIZE_WORDS);						/* Fill payload with cmac*/
#endif
#if USECASE_CMAC_1
				Array_Copy(&CAN_Rx_Message.data.RxDATA[1], &CAN_data.TxDATA[8],RND_NUM_SIZE_WORDS);	/* Fill payload with received rnd number*/
#endif
				ENC_CBC(CAN_data.TxDATA, init_vector, RAM_KEY, CAN_data.TxDATA, 4);					/* Add encrypt random number to the payload*/
#endif
				FLEXCAN0_transmit_msg(&CAN_data, canfd_en);
				while(0 == ((CAN0->IFLAG1 >> 4) & 1));
				CAN_Rx_Message = FLEXCAN0_receive_msg (CAN_RX_MB,MB_SIZE, canfd_en);		/* Read message */
				if((CAN_Rx_Message.data.RxID == EDGE_NODE_ID)  && (CAN_Rx_Message.data.RxDATA[0] == ACK_PAYLOAD))
				{
					adress_flash = adress_flash + DATA_TX_BYTES;
					logic_address = adress_flash-start_address;

					if((logic_address < MAX_FW_SIZE_BYTES) && (logic_address < image_size))
					{
						fw_transmission_state = FW_APP_STATE;
					}
					else
					{
						CAN_data.TxID = GATEWAY_FW_ADDRESS_ID;
						CAN_data.TxLENGTH = DLC_SIZE_END_FRAME;
						CAN_data.TxDATA[0] = END_DATA_PAYLOAD;
#if CANFD_COMMUNICATION && CSEC_ENABLE
						Array_Copy(&CAN_Rx_Message.data.RxDATA[1], &CAN_data.TxDATA[1], RND_NUM_SIZE_WORDS);		/* Copy of received rnd number to tx payload */
						ENC_CBC(CAN_data.TxDATA, init_vector, RAM_KEY, CAN_data.TxDATA, 2);							/* Add encrypt random number to the payload*/
#endif
						FLEXCAN0_transmit_msg(&CAN_data, canfd_en);

						while(0 == ((CAN0->IFLAG1 >> 4) & 1));
						CAN_Rx_Message = FLEXCAN0_receive_msg (CAN_RX_MB, MB_SIZE, canfd_en);		/* Read message */
						if((CAN_Rx_Message.data.RxID == EDGE_NODE_ID)  && (CAN_Rx_Message.data.RxDATA[0] == ACK_PAYLOAD))
						{
							logic_address = 0x00000000;
							start_transmission = 0;		/*   Finished transmission */
							fw_transmission_state = START_FRAME_STATE;
						}
						else
						{
							start_transmission = 0;		/* Stop transmission */
							fw_transmission_state = START_FRAME_STATE;
							break;
						}

#if USECASE_CMAC_1
						while(0 == ((CAN0->IFLAG1 >> 4) & 1));
						CAN_Rx_Message = FLEXCAN0_receive_msg(CAN_RX_MB, MB_SIZE, canfd_en);		/* Read message */
						if((CAN_Rx_Message.data.RxID == EDGE_NODE_ID)  && (CAN_Rx_Message.data.RxDATA[0] == ACK_PAYLOAD))
						{
							fw_transmission_state = FW_CMAC_STATE;
						}
						else
						{
							start_transmission = 0;		/* Stop transmission */
							fw_transmission_state = START_FRAME_STATE;
							break;
						}
#endif
#if USECASE_CMAC_0
						PTD->PSOR |= 1<<15;			/*   Turn off port D15 (Red LED) */
						start_transmission = 0;		/*   Finished transmission */
						fw_transmission_state = START_FRAME_STATE;
#endif
					}
				}
				else
				{
					start_transmission = 0;		/* Stop transmission */
					fw_transmission_state = START_FRAME_STATE;
					break;
				}

				rx_msg_count++;				/* Increment receive msg counter */
				if (rx_msg_count == 500)	/* If 500 messages have been received */
				{
					PTD->PTOR |= 1<<15;		/*   Toggle output port D15 (Red LED) */
					rx_msg_count = 0;		/*   and reset message counter */
				}
				break;

			default:
				start_transmission = 0;	/* stop transmission */
				fw_transmission_state = START_FRAME_STATE;
				break;
				break;
			}
		}
	}


	/* to avoid the warning message for GHS and IAR: statement is unreachable*/
#if defined (__ghs__)
#pragma ghs nowarning 111
#endif
#if defined (__ICCARM__)
#pragma diag_suppress=Pe111
#endif
	return 0;
}
