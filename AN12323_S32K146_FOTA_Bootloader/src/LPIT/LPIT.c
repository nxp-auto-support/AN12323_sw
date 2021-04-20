/*
 * LPIT.c
 *
 *  Created on: Dec 29, 2016
 *      Author: Osvaldo
 */


#include "S32K146.h" /* include peripheral declarations S32K146 */
#include "LPIT.h"


void init_LPIT(void)
{
  PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(3);    /* Clock Src = 3 (FIRCDIV2_CLK)*/
  PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clk to LPIT0 regs */
  LPIT0->MCR = 0x00000001;    /* DBG_EN-0: Timer chans stop in Debug mode */
                              /* DOZE_EN=0: Timer chans are stopped in DOZE mode */
                              /* SW_RST=0: SW reset does not reset timer chans, regs */
                              /* M_CEN=1: enable module clk (allows writing other LPIT0 regs) */
  LPIT0->TMR[0].TVAL = TWO_SECONDS;         		 	 /* PIT0: Load value to count 12M bus clocks */
  LPIT0->MIER |= LPIT_MIER_TIE0_MASK; 				 /* Enable interrupt */
  LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;  /* Enable (start) timer */
}
void deinit_LPIT (void)
{
    LPIT0->TMR[0].TCTRL &= ~LPIT_TMR_TCTRL_T_EN_MASK;  /* Disable  timer */
	LPIT0->MIER = 0x00000000;
	PCC->PCCn[PCC_LPIT_INDEX] &= ~PCC_PCCn_CGC_MASK;
	PCC->PCCn[PCC_LPIT_INDEX] &= ~PCC_PCCn_PCS_MASK;
}
void Reset_LPIT(void)
{
	LPIT0->TMR[0].TCTRL &= ~LPIT_TMR_TCTRL_T_EN_MASK;  /* Disable  timer */
	LPIT0->TMR[0].TVAL = TWO_SECONDS;         		 /* Reload value*/
	LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;  /* Enable (start) timer */
}
void stop_LPIT(void)
{
	LPIT0->TMR[0].TCTRL &= ~LPIT_TMR_TCTRL_T_EN_MASK;  /* Disable  timer */
}
