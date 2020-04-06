/*
 * timer.c
 *
 * Created: 12.03.2020 09:32:13
 *  Author: gju
 */ 

#include "../includes/timer.h"


void init_timer()
{
  //++++++++++ Initial ATMega368 Timer/Counter0 Peripheral ++++++++//
  //Timer anhalten, Prescaler Reset
  GTCCR |= (1 << TSM) | (1 << PSRASY);	

  TCCR0A=0x00;                  // Normal Timer0 Operation
  TCCR0B=(1<<CS02)|(1<<CS00);   // Use maximum prescaller: Clk/1024

  GTCCR &= ~(1 << TSM);//Timer starten

  TCNT0=TIMER_RESET;                   // Start counter from 0x94, overflow at 10 mSec

  //TCNT0=15;                   // Start counter from 0x94, overflow at 10 mSec
  TIMSK0=(1<<TOIE0);            // Enable Counter Overflow Interrupt
 
 
 sei();                        // Enable Interrupt
  
}  