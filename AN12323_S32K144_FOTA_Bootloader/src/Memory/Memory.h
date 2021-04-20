/*
 * Memory.h
 *
 *  Created on: Jul 19, 2017
 *      Author: B46911
 */

#ifndef MEMORY_MEMORY_H_
#define MEMORY_MEMORY_H_
/*******************************************************************************
* Includes
********************************************************************************/
#include "S32K144.h"
#include "S32K144_features.h"
#include "fsl_flash_driver_c90tfs.h"
#include "Array.h"
#include "Comm.h"
#include "Bootloader_cfg.h"


/*******************************************************************************
* Constants
********************************************************************************/

/*******************************************************************************
* Macros
********************************************************************************/

/*******************************************************************************
* Types
********************************************************************************/


/*******************************************************************************
* Variables
********************************************************************************/


/*******************************************************************************
* Global Variables
********************************************************************************/
/* Declare a FLASH config struct which initialized by FlashInit, and will be used by all flash operations */
extern flash_ssd_config_t flashSSDConfig;
extern const flash_user_config_t Flash_InitConfig0;
extern flash_drv_status_t ret;        /* Store the driver api return code */
extern flash_command_sequence_t pCmdSequence;           /* Set command sequence */;
extern flash_command_sequence_t pCmdSequence_RAM;  /* Set command sequence for function in RAM */



/*******************************************************************************
* Global Functions
********************************************************************************/

void Memory_Init(void);
void Download_Data_to_Flash(uint32_t flash_address,const uint8_t* pData, uint16_t data_size,flash_command_sequence_t pFlashCommandSequence);
void Erase_Flash(uint32_t flash_address,uint16_t image_size,flash_command_sequence_t pFlashCommandSequence);


#endif /* MEMORY_MEMORY_H_ */
