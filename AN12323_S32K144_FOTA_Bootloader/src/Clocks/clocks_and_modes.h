#ifndef CLOCKS_AND_MODES_H_
#define CLOCKS_AND_MODES_H_

void SOSC_init_8MHz (void);
void SPLL_init_160MHz (void);
void NormalRUNmode_80MHz (void);
void NormalRUNmode_48MHz (void);
void deinit_SPLL(void);
void deinit_SOSC(void);

#endif /* CLOCKS_AND_MODES_H_ */

