/*
 * timer.h
 *
 * Created: 12.03.2020 09:31:42
 *  Author: gju
 */ 

#ifndef F_CPU
#define F_CPU 16000000	// Has to be here (not in .h or program.c) otherwise fucked up things happen
#endif

#ifndef _TIMER_H_
#define _TIMER_H_

#include <avr/interrupt.h>


//16000000 / 1024 =15.652 255-15 = 240 = 0xF0
#define TIMER_RESET  0xF1//0xF0
void init_timer();



#endif /* TIMER_H_ */