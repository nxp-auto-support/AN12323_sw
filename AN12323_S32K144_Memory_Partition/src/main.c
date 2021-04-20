/*
 * main implementation: use this 'C' sample to create your own application
 *
 */


#include "S32K144.h" /* include peripheral declarations S32K144 */

/* Enables CSEc by issuing the Program Partition Command, procedure. Configures for all 24 Keys */
uint8_t configure_part_CSEc(void)
{
	uint8_t flash_error_status = 0;

    while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); /* Wait until any ongoing flash operation is completed */
    FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  /* Write 1 to clear error flags */

    FTFC->FCCOB[3] = 0x80; /* FCCOB0 = 0x80, program partition command */
    FTFC->FCCOB[2] = 0x03; /* FCCOB1 = 2b11, 24 keys */
    FTFC->FCCOB[1] = 0x00; /* FCCOB2 = 0x00, SFE = 0, VERIFY_ONLY attribute functionality disable */
    FTFC->FCCOB[0] = 0x00; /* FCCOB3 = 0x00, FlexRAM will be loaded with valid EEPROM data during reset sequence */
    FTFC->FCCOB[7] = 0x02; /* FCCOB4 = 0x02, 4k EEPROM Data Set Size */
    FTFC->FCCOB[6] = 0x0A; /* FCCOB5 = 0x0A, 16K data flash, 48k EEPROM backup */

    FTFC->FSTAT = FTFC_FSTAT_CCIF_MASK; /* Start command execution by writing 1 to clear CCIF bit */

    while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); /* Wait until ongoing flash operation is completed */

    flash_error_status = FTFC->FSTAT; /* Read the flash status register for any Execution Error */

    return flash_error_status;
}


int main(void)
{
	uint8_t __attribute__((unused)) flash_error = 0;

	flash_error = configure_part_CSEc();

	for(;;) {}

    /* to avoid the warning message for GHS and IAR: statement is unreachable*/
#if defined (__ghs__)
#pragma ghs nowarning 111
#endif
#if defined (__ICCARM__)
#pragma diag_suppress=Pe111
#endif
	return 0;
}
