/*
 * Boot.c

 *
 *  Created on: Jan 7, 2017
 *      Author: Osvaldo
 */

/**********************************************************************************************
* Includes
**********************************************************************************************/
#include "Boot.h"

/**********************************************************************************************
* Global variables
**********************************************************************************************/
uint32_t fw_header[1024]; // Header buffer of 4kB
uint32_t header_address=0x00000000;
uint32_t fw_logic_address=0x00000000;
uint32_t fw_data[FW_BUFFER_SIZE_WORDS];
uint32_t fw_sign_address=0x00000000;
uint32_t flash_address=0;
uint32_t flash_new_image_address_block=0x00000000;
uint32_t flash_old_image_address_block=0x00000000;
uint32_t fw_header_address=0x00000000;
uint32_t latest_fw_version=0x00000000;
uint8_t header_complete=0;

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
void Flash_Block_to_Update(void)
{
	/* Read Firmware 1st location*/
	fw_header_address=IMAGE1_HEADER_ADDRESS;
	fw_sign_address=fw_header_address+4092;
	if(*((uint32_t *)fw_sign_address)==0x55AA55AA)
	{
		/* Read image2 header */
		fw_header_address=IMAGE2_HEADER_ADDRESS;
		fw_sign_address=fw_header_address+4092;

		/* Check if image 2 header is valid */
		if(*((uint32_t *)fw_sign_address)==0x55AA55AA)
		{
			/*Compare firmware version*/
			if(*((uint32_t *)IMAGE1_HEADER_ADDRESS)>*((uint32_t *)IMAGE2_HEADER_ADDRESS))
			{
				flash_new_image_address_block=FLASH_BLOCK_B_ADDRESS;
				flash_old_image_address_block=FLASH_BLOCK_A_ADDRESS;
				latest_fw_version=(uint32_t)(*(uint32_t *)flash_old_image_address_block);
			}
			else
			{
				flash_new_image_address_block=FLASH_BLOCK_A_ADDRESS;
				flash_old_image_address_block=FLASH_BLOCK_B_ADDRESS;
				latest_fw_version=(uint32_t)(*(uint32_t *)flash_old_image_address_block);
			}
		}
		else
		{
			flash_new_image_address_block=FLASH_BLOCK_B_ADDRESS;
			flash_old_image_address_block=FLASH_BLOCK_A_ADDRESS;
			latest_fw_version=(uint32_t)(*(uint32_t *)flash_old_image_address_block);
		}
	}
	else
	{
		flash_new_image_address_block=FLASH_BLOCK_A_ADDRESS;
		flash_old_image_address_block=FLASH_BLOCK_B_ADDRESS;
		latest_fw_version=(uint32_t)(*(uint32_t *)flash_old_image_address_block);
	}
	/*Add image header offset */
	flash_new_image_address_block=flash_new_image_address_block+IMAGE_HEADER_BYTE_SIZE;
	flash_old_image_address_block=flash_old_image_address_block+IMAGE_HEADER_BYTE_SIZE;
}


uint8_t Update_State_Machine (uint8_t* tc_flag)
{
	uint8_t error_flag=0;

	switch (CAN_Rx_Message.data.RxID)
	{
		/* Check that the receive message has the correct ID that contains the start update*/
		case GATEWAY_START_ID:
			if(START_TRANSMISION_PAYLOAD==CAN_Rx_Message.data.RxDATA[0])
			{
				SET_MB_ID(CAN_RX_MB,MB_SIZE,GATEWAY_FW_ADDRESS_ID);    /* Update MB ID for Gateway Address frame */
			}
		break;

		/* Check that the receive message has the correct ID that contains the address*/
		case GATEWAY_FW_ADDRESS_ID :
			#if CSEC_ENABLE
			if(1==Verify_Received_Addr_Msg())
			{
				error_flag=1;
				break;
			}
			#endif
			/* Check if received termination word*/
			if(END_DATA_PAYLOAD==CAN_Rx_Message.data.RxDATA[0])
			{
				/* Check if header or fw is complete*/
				if(0==header_complete)
				{
					header_complete=1;
				}
				/* Check if termination word is from application*/
				else if(1== header_complete)
				{
					*tc_flag=1;
					header_complete = 0;
					SET_MB_ID(CAN_RX_MB,MB_SIZE,GATEWAY_START_ID);	/*Update MB to receive ID = GATEWAY_START_ID  */
				}
			}
			/* if termination word not received, then is address*/
			else
			{
				/* Check if header is completed*/
				if(0==header_complete)
				{
					Set_Header_Address();
				}
				else if(1==header_complete)
				{
					Set_Fwlogic_Address();
				}
				SET_MB_ID(CAN_RX_MB,MB_SIZE,GATEWAY_FW_DATA_ID);  /*Update MB to receive ID = GATEWAY_FW_DATA_ID  */
			}
		break;

		/* Check that the receive message has the correct ID that has the new header/application data*/
		case GATEWAY_FW_DATA_ID:
			#if CSEC_ENABLE
		    /* Verify random number and CMAC*/
			if(1==Verify_Received_Data_Msg())
			{
				error_flag=1;
				break;
			}
			#endif
			/* Check if header information is complete */
			if (0==header_complete)
			{
				Store_Received_Data_in_Header();
				// NOTE: Commented out for demo purposes. This function validates that the received FW is newer than the active one
				//error_flag=Validate_Received_FW_Version();
				//if(1==error_flag)
				//{
				//	break;
				//}
			 }
			/* In case header complete, then fill fw data */
			else if (1==header_complete)
			{
				Update_FW_Application();
			}
			SET_MB_ID(CAN_RX_MB,MB_SIZE,GATEWAY_FW_ADDRESS_ID);    /* Update MB ID for Gateway Address frame */
		break;

		default:
		break;
	}
	return error_flag;
}


void Set_Header_Address(void)
{
	header_address=CAN_Rx_Message.data.RxDATA[0];	/* Set the header address */
}

void Set_Fwlogic_Address(void)
{
	fw_logic_address=CAN_Rx_Message.data.RxDATA[0];	/* Set the fw logic address */
}



void Store_Received_Data_in_Header(void)
{
	Array_Copy(&CAN_Rx_Message.data.RxDATA[0], &fw_header[header_address],DATA_RX_WORDS);  /* Fill the payload with the data to send */
}

uint8_t Validate_Received_FW_Version(void)
{
	uint8_t fw_version_error=0;
	if (header_address==0 && latest_fw_version!=0xFFFFFFFF) /* Check header address is 0x0 loc of fw ver and that the latest fw version is not empty */
	{
		if(fw_header[header_address]<=latest_fw_version)  /* if the fw version is older or same as before */
		{
			fw_version_error=1;							  /* fw already in device or older revision, indicate error */
		}
	}
	return fw_version_error;
}

void Update_FW_Headers(void)
{
	fw_header[1023]=APP_KEY;  												/* Add signature to fw header */
	fw_header_address=flash_new_image_address_block-IMAGE_HEADER_BYTE_SIZE;	/* Set the new fw header address */
	if(*(uint32_t*)(fw_header_address) != 0xFFFFFFFF)
	{
		Erase_Flash(fw_header_address,IMAGE_HEADER_SECTOR_SIZE,pCmdSequence); 	/* Erase old fw header */
	}
	Download_Data_to_Flash(fw_header_address,(uint8_t*)&fw_header,IMAGE_HEADER_BYTE_SIZE,pCmdSequence);  /* Download the new fw header */
	fw_header_address=flash_old_image_address_block-IMAGE_HEADER_BYTE_SIZE;	/* Set the old fw header address */

	DISABLE_INTERRUPTS()
	Erase_Flash(fw_header_address,IMAGE_HEADER_SECTOR_SIZE,pCmdSequence); 	/* Erase old fw header */
	ENABLE_INTERRUPTS()
}


void Update_FW_Application (void)
{
	Array_Copy(&CAN_Rx_Message.data.RxDATA[0],&fw_data[0],DATA_RX_WORDS);     /* Store the app data in the  fw buffer */
	flash_address=fw_logic_address+flash_new_image_address_block;			  /* Update the flash address to program received data */

	if(fw_logic_address>=START_VTOR_TABLE && fw_logic_address<END_VTOR_TABLE) /* Check if the received address corresponds to VTOR table location */
	{
		if(START_VTOR_TABLE==fw_logic_address)
		{
			Relocate_VTOR_Table_Address(fw_data,FW_BUFFER_SIZE_WORDS,flash_new_image_address_block,1);
		}
		else
		{
			Relocate_VTOR_Table_Address(fw_data,FW_BUFFER_SIZE_WORDS,flash_new_image_address_block,0);
		}
	}

    if((flash_address % 4096) == 0)  /* Is address at beginning of sector? */
    {
    	Erase_Flash(flash_address, 1, pCmdSequence); 	/* Erase single Sector */
    }
	Download_Data_to_Flash(flash_address,(uint8_t*)&fw_data,FW_BUFFER_SIZE_BYTES,pCmdSequence);
}

void Relocate_VTOR_Table_Address(uint32_t *data_buffer_ptr, uint16_t data_size,uint32_t adrress_offset,uint8_t start_flag)
{
	uint16_t cntr=0;
	if(start_flag==1)
	{
		cntr=1;
		data_buffer_ptr++;
	}
	do
	{
		*data_buffer_ptr=*data_buffer_ptr+adrress_offset;
		data_buffer_ptr++;
		cntr++;
	}
	while(cntr<data_size);
}

void Init_Peripherals(void)
{
	PORT_init();                                        /* Initialize Ports*/
	SCG->FIRCDIV=SCG_FIRCDIV_FIRCDIV2(0b100);           /* FIRCDIV2 enable @ 48MHz/8=6MHz */
	SOSC_init_8MHz();    								/* Enable Oscillator, only for CAN/CANFD */
	SPLL_init_160MHz();     							/* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	NormalRUNmode_80MHz();  							/* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
	Communication_Init();								/* Init communication */
	Memory_Init();										/* Init Memory */
	LMEM->PCCCR = 0x85000001;							/* Invalidate cache & enable write buffer, cache */
#if CSEC_ENABLE
	Init_Security();									/* Init Rnd generator,and set keys */
#endif
}

void Restore_Peripherals(void)
{
	Communication_deinit();
	NormalRUNmode_48MHz();
	deinit_SPLL();
	deinit_SOSC();
	PORT_deinit();
	SCG->FIRCDIV=SCG_FIRCDIV_FIRCDIV2(0b000); /*FIRCDIV2 disabled */
}

void Jump_to_New_Fw(void)
{
    JumpToApplication(flash_new_image_address_block);
}

void Jump_to_Old_Fw(void)
{
    JumpToApplication(flash_old_image_address_block);
}

void JumpToApplication(uint32_t start_address)
{
    void (*entry)(void);
    uint32_t pc;
    uint32_t __attribute__((unused)) sp;

    S32_SCB->VTOR=(uint32_t)(start_address);      /*Relocate interrupt table ptr*/
    sp = *((volatile uint32_t*)start_address);
    asm(" ldr sp, [r0,#0]");
    pc = *((volatile uint32_t *)(start_address + 4));
    entry = (void (*)(void))pc;
    entry();
}

