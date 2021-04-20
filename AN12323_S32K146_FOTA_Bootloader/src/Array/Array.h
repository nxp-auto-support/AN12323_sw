/*
 * Array.h
 *
 *  Created on: Jul 18, 2017
 *      Author: B46911
 */

#ifndef ARRAY_ARRAY_H_
#define ARRAY_ARRAY_H_


/*******************************************************************************
* Includes
********************************************************************************/
#include "S32K146.h"
#include "S32K146_features.h"
#include "fsl_flash_driver_c90tfs.h"
#include "Bootloader_cfg.h"


/*******************************************************************************
* Constants
********************************************************************************/

/*******************************************************************************
* Macros
********************************************************************************/


#define IMAGE_SECTOR_SIZE				126		/* No of sectors in image, each sector is 4kB */
#define IMAGE_HEADER_SECTOR_SIZE		1		/* No of sectors in image header, each sector is 4kB */
#define IMAGE_HEADER_BYTE_SIZE			0x1000	/* 4kB header size */

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


/*******************************************************************************
* Global Functions
********************************************************************************/

void Clean_Array (uint32_t *data_buffer_ptr,uint16_t buffer_size);
void Array_Copy (uint32_t *data_buffer1_ptr, uint32_t *data_buffer2_ptr,uint16_t array_size);
uint8_t Compare_Arrays(uint32_t *data_buffer1_ptr, uint32_t *data_buffer2_ptr,uint16_t array_size);



#endif /* ARRAY_ARRAY_H_ */
