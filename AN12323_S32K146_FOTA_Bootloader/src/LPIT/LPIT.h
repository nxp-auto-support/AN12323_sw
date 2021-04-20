/*
 * LPIT.c
 *
 *  Created on: Dec 29, 2016
 *      Author: Osvaldo
 */

#ifndef LPIT_H_
#define LPIT_H_

#define ONE_SECOND 5999999
#define TWO_SECONDS 11999999
#define THREE_SECONDS 17999999
#define FOUR_SECONDS 23999999
#define EIGHT_SECONDS 47999999

void init_LPIT(void);
void deinit_LPIT (void);
void Reset_LPIT(void);
void stop_LPIT(void);

#endif /* LPIT_H_ */
