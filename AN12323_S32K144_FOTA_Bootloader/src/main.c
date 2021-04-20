/**********************************************************************************************
* External objects
**********************************************************************************************/
#include "S32K144.h" /* include peripheral declarations S32K144 */
#include "S32K144_features.h"
#include "fsl_flash_driver_c90tfs.h"
#include "LPIT.h"
#include "Boot.h"
#include "CSEc_functions.h"
#include "CSEc_macros.h"
#include "FlexCAN.h"
#include "Bootloader_cfg.h"
#include "clocks_and_modes.h"
#include "System.h"
#include "Array.h"
#include "Comm.h"
#include "Secure_Comm.h"
#include "Memory.h"

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
static uint8_t error_flag = 0;
static uint8_t reset_lpit = 0;
static uint8_t timeout_flag = 0;
static uint8_t transmission_complete = 0;

/**********************************************************************************************
* Local functions
**********************************************************************************************/
void loader_code(void);

/**********************************************************************************************
* Global functions
**********************************************************************************************/

/***********************************************************************************************
*
* @brief    main() - Program entry function
* @param    none
* @return   none
*
************************************************************************************************/
int main(void)
{
	Init_Peripherals();

	Flash_Block_to_Update(); 							/* Locate the flash area with the oldest fw */

	for(;;)
	{
		if(latest_fw_version != 0xFFFFFFFF)
		{
			init_IRQs();    									/*Init interrupts for LPIT*/
			init_LPIT();   										/*Init LPIT timer*/

			/* Wait for transmission to complete or timeout */
			while (0==transmission_complete && 0==timeout_flag)
			{
				loader_code();

				if(1 == reset_lpit)
				{
					Reset_LPIT();		/* Reset Timer  */
					reset_lpit = 0;
				}
			}

			deinit_LPIT();
			deinit_IRQs();

			Restore_Peripherals();		/* Set peripherals and clocks to default state */

			/* Check if the transmission was completed */
			if(transmission_complete == 1)
			{
				Jump_to_New_Fw();		/* Jump to flash location with new fw */
			}
			else if (transmission_complete == 0)
			{
				Jump_to_Old_Fw();		/* Jump to flash location with old firmware */
			}
		}
		else
		{
			/* No valid application present. Stay in bootloader */
			while(1)
			{
				loader_code();

				/* Jump to new firmware when transmission is complete */
				if(transmission_complete == 1)
				{
					Restore_Peripherals();
					Jump_to_New_Fw();
				}
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


void loader_code(void)
{
	/* Wait for message on MB 4 */
	if (RX_NEW_MSG == Comm_New_Msg_Rx())
	{
		Comm_Rx_Data();		/* Receive the new data */
		if(0==Comm_CRC_Verif())
		{
			reset_lpit = 1;
			error_flag=Update_State_Machine(&transmission_complete);		  /* Enter State Machine to update */
			Comm_Transmit_Ack (error_flag);								      /* Reply message */
		}
	}

	/* Check if the transmission was completed */
	if(transmission_complete == 1)
	{
		/* If transmission completed */
		Update_FW_Headers();	/* Update the new fw header, erase the old fw header*/
	}
}

void LPIT0_Ch0_IRQHandler (void)
{
  LPIT0->MSR|= LPIT_MSR_TIF0_MASK; /* Clear PIT0 flag */
  timeout_flag=1;				   /* Set timeout flag */
}
