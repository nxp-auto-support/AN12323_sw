/*
 * Memory.c
 *
 *  Created on: Jul 19, 2017
 *      Author: B46911
 */



/**********************************************************************************************
* Includes
**********************************************************************************************/
#include "Memory.h"

/**********************************************************************************************
* Global variables
**********************************************************************************************/
const flash_user_config_t Flash_InitConfig0 = {
    .PFlashBase  = 0x00000000U,
	.PFlashSize  = 0x00080000U,
    .DFlashBase  = 0x10000000U,
    .EERAMBase   = 0x14000000U,
    .CallBack    = NULL_CALLBACK
};
/* Declare a FLASH config struct which initialized by FlashInit, and will be used by all flash operations */
flash_ssd_config_t flashSSDConfig;
flash_drv_status_t ret;        /* Store the driver api return code */
flash_command_sequence_t pCmdSequence= (flash_command_sequence_t)FlashCommandSequence;           /* Set command sequence */;
flash_command_sequence_t pCmdSequence_RAM=(flash_command_sequence_t)FlashCommandSequence_RAM;   /* Set command sequence for function in RAM */

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

void Memory_Init(void)
{

	 /* Always initialize the driver before calling other functions */
		ret = FlashInit(&Flash_InitConfig0, &flashSSDConfig);
		while (ret != FTFx_OK);
		/* FlexRAM is already configured as EEPROM */
		/* Make FlexRAM available for EEPROM, make sure that FlexNVM and FlexRAM
		 * are already partitioned successfully before */
		ret = SetFlexRamFunction(&flashSSDConfig, EEE_ENABLE, 0x00u, NULL, pCmdSequence_RAM);
		while (ret != FTFx_OK);

}

void Download_Data_to_Flash(uint32_t flash_address,const uint8_t* pData, uint16_t data_size,flash_command_sequence_t pFlashCommandSequence)
{
	flash_drv_status_t ret;        /* Store the driver api return code */
	uint32_t failAddr;
	ret = FlashProgram(&flashSSDConfig, flash_address, data_size, pData, pFlashCommandSequence);
	while (ret != FTFx_OK);
	/* Verify the program operation at margin level value of 1, user margin */
	ret = FlashProgramCheck(&flashSSDConfig, flash_address, data_size, pData, &failAddr, 1u, pFlashCommandSequence);
	while (ret != FTFx_OK);
}


void Erase_Flash(uint32_t flash_address,uint16_t image_size,flash_command_sequence_t pFlashCommandSequence)
{

	uint32_t flash_temp_address=0;
    flash_drv_status_t ret;        /* Store the driver api return code */
    uint8_t i=0;
	flash_temp_address=flash_address;
	/*Erase oldest image*/
	for(i=0;i<image_size;i++)
	{
		ret=FlashEraseSector(&flashSSDConfig,flash_temp_address, FEATURE_FLS_PF_BLOCK_SECTOR_SIZE, pFlashCommandSequence);
		while (ret != FTFx_OK);
		 /* Verify the erase operation at margin level value of 1, user read */
		ret = FlashVerifySection(&flashSSDConfig, flash_temp_address, FEATURE_FLS_PF_BLOCK_SECTOR_SIZE / FTFx_DPHRASE_SIZE, 1u, pFlashCommandSequence);
		while (ret != FTFx_OK);
		flash_temp_address=flash_temp_address +FEATURE_FLS_PF_BLOCK_SECTOR_SIZE;
	}
}

