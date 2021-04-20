/*
 * Boot.h
 *
 *  Created on: Jan 7, 2017
 *      Author: Osvaldo
 */

#ifndef BOOT_H_
#define BOOT_H_

/*******************************************************************************
* Includes
********************************************************************************/
#include "S32K146.h"
#include "S32K146_features.h"
#include "fsl_flash_driver_c90tfs.h"
#include "Array.h"
#include "Comm.h"
#include "Secure_Comm.h"
#include "Bootloader_cfg.h"
#include "LPIT.h"
#include "clocks_and_modes.h"
#include "Memory.h"
#include "System.h"

/*******************************************************************************
* Constants
********************************************************************************/

/*******************************************************************************
* Macros
********************************************************************************/


#define IMAGE_SECTOR_SIZE				126			/* No of sectors in image, each sector is 4kB */
#define IMAGE_HEADER_SECTOR_SIZE		1			/* No of sectors in image header, each sector is 4kB */
#define IMAGE_HEADER_BYTE_SIZE			0x1000		/* 4kB header size */

#if CAN_BOOTLOADER
#define RX_BUFFER_SIZE 8 	/* 8 bytes */
#endif

#if CANFD_BOOTLOADER
#define RX_BUFFER_SIZE 64 //1*16=16, must be multiple of 8 bytes, Phrase size.
#endif

#define FLASH_BLOCK_A_ADDRESS 0x01000
#define FLASH_BLOCK_B_ADDRESS 0x81000

/*******************************************************************************
* Types
********************************************************************************/


/*******************************************************************************
* Variables
********************************************************************************/


/*******************************************************************************
* Global Variables
********************************************************************************/
extern uint32_t flash_new_image_address_block;
extern uint32_t latest_fw_version;


/*******************************************************************************
* Global Functions
********************************************************************************/

void Flash_Block_to_Update(void);
uint8_t Update_State_Machine (uint8_t* tc_flag);
void Set_Header_Address(void);
void Set_Fwlogic_Address(void);
void Store_Received_Data_in_Header(void);
uint8_t Validate_Received_FW_Version(void);
void Update_FW_Headers(void);
void Update_FW_Application (void);
void Relocate_VTOR_Table_Address(uint32_t *data_buffer_ptr, uint16_t data_size,uint32_t adrress_offset,uint8_t start_flag);
void Init_Peripherals(void);
void Restore_Peripherals(void);
void Jump_to_New_Fw(void);
void Jump_to_Old_Fw(void);
void JumpToApplication(uint32_t start_address);


#endif /* BOOT_H_ */
