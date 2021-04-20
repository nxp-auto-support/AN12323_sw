/*
 * Secure_Comm.c
 *
 *  Created on: Jul 19, 2017
 *      Author: B46911
 */



/*******************************************************************************
* Includes
********************************************************************************/
#include "Secure_Comm.h"
#include "Comm.h"
/**********************************************************************************************
* Global variables
**********************************************************************************************/
uint32_t random_number[4];
uint32_t init_vector[4]={0x00000000,0x00000000,0x00000000,0x00000000};
uint16_t __attribute__((unused)) csec_error = 0;
uint32_t test_key[4]={0xc13dd10d,0xbfb5e142,0x316c6d5c,0x2e903ec9};

/**********************************************************************************************
* Constants and macros
**********************************************************************************************/
#define RND_NUM_SIZE_WORDS 4

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
void Init_Security (void)
{

    csec_error = INIT_RNG();    /* Init rnd number generator */
	LOAD_RAM_KEY(test_key);     /* Load RAM key into RAM */

}

void Generate_Rnd_Ack_Msg (void)
{
	// Generate RND for ACK
	GENERATE_RANDOM_NUMBER(random_number);
	Array_Copy(&random_number[0],&CAN_ackdata.TxDATA[1],RND_NUM_SIZE_WORDS); /* Fill ack frame with rnd number */

}

uint8_t Verify_Received_Addr_Msg (void)
{
	uint8_t verif_error=0;
	csec_error=DEC_CBC(CAN_Rx_Message.data.RxDATA,init_vector,RAM_KEY,CAN_Rx_Message.data.RxDATA,2);
	 verif_error=Compare_Arrays(&CAN_Rx_Message.data.RxDATA[1], &random_number[0],RND_NUM_SIZE_WORDS);	 /* Compare send and received random number */
	return verif_error;
}

uint8_t Verify_Received_Data_Msg (void)
{
	 uint16_t verification_status;
	uint8_t verif_error=0;
	csec_error=DEC_CBC(CAN_Rx_Message.data.RxDATA,init_vector,RAM_KEY,CAN_Rx_Message.data.RxDATA,4);
	verif_error=Compare_Arrays(&CAN_Rx_Message.data.RxDATA[12], &random_number[0],RND_NUM_SIZE_WORDS);	 /* Compare send and received random number */
	csec_error=CMAC_VERIFY(&verification_status,CAN_Rx_Message.data.RxDATA,RAM_KEY,256);        				 /* Verify that received CMAC match with the data */
	if(verification_status!=0)
	{
		verif_error=1;
	}
	else
	{
		verif_error|=0;
	}

	return verif_error;
}

uint8_t Verify_Received_Data_Msg_2 (void)
{
	uint8_t verif_error=0;
	csec_error=DEC_CBC(CAN_Rx_Message.data.RxDATA,init_vector,RAM_KEY,CAN_Rx_Message.data.RxDATA,3);
	verif_error=Compare_Arrays(&CAN_Rx_Message.data.RxDATA[8], &random_number[0],RND_NUM_SIZE_WORDS);       /* Check if the random number is the same, to check authenticity */

	return verif_error;
}

uint8_t Verify_Received_CMAC_Msg(void)
{
	uint16_t verification_status;
	uint8_t verif_error=0;
	csec_error=DEC_CBC(CAN_Rx_Message.data.RxDATA,init_vector,RAM_KEY,CAN_Rx_Message.data.RxDATA,2);
	verif_error=Compare_Arrays(&CAN_Rx_Message.data.RxDATA[4], &random_number[0],RND_NUM_SIZE_WORDS);       /* Check if the random number is the same, to check authenticity */
	csec_error=CMAC_VERIFY_POINTER(&verification_status,CAN_Rx_Message.data.RxDATA,(flash_new_image_address_block),RAM_KEY,FW_SIZE_BITS);         /* Verify that received CMAC match with the data */
	if(verification_status!=0)
	{
		verif_error=1;
	}
	else
	{
		verif_error|=0;
	}
	return verif_error;

}
