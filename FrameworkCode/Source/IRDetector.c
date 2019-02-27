/****************************************************************************
 Module
   IRDetector.c

 Revision
   1.0.1

 Description
   This is a simple service that calculates the period using interrupts(input capture mode)
   Generation of events(Found_WEST/FOUND_EAST etc) from a different service___NOT HERE!

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 2/27/19 14:19  Hyunjoo Started IRDetector      
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"
#include "ES_Events.h"

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_timer.h"
#include "inc/hw_nvic.h"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/interrupt.h"
#include "inc/tm4c123gh6pm.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>
#include "termio.h"

/*----------------------------- Module Defines -----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
//Function list
//InitInputCapture: rising edge
//IR_ISR: clear interrupt
//


/*---------------------------- Module Variables ---------------------------*/

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
   InitInputCapture(void)

 Parameters
  nothing

 Returns
  nothing

 Description
  Set PD1 to input capture mode
 
 Notes

 Author
  Hyun Joo Lee 02/27/2019
****************************************************************************/
void InitInputCapture(void)
{
// start by enabling the clock to the timer (Wide Timer 2)
  HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R2; 
// enable the clock to Port D
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3;

  //wait until the port D clock is enabled
  while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R3) != SYSCTL_PRGPIO_R3)
  {
    ;
  }
// make sure that timer (Timer B) is disabled before configuring
  HWREG(WTIMER2_BASE + TIMER_O_CTL) &= ~TIMER_CTL_TBEN;
// set it up in 32bit wide (individual, not concatenated) mode
// the constant name derives from the 16/32 bit timer, but this is a 32/64
// bit timer so we are setting the 32bit mode
  HWREG(WTIMER2_BASE + TIMER_O_CFG) = TIMER_CFG_16_BIT;
// we want to use the full 32 bit count, so initialize the Interval Load
// register to 0xffffffff (its default value :-)
  HWREG(WTIMER2_BASE + TIMER_O_TBILR) = 0xffffffff;
// set up timer B in capture mode (TBMR=3, TBAMS = 0),
// for edge time (TBCMR = 1) and up-counting (TBCDIR = 1)
  HWREG(WTIMER2_BASE + TIMER_O_TBMR) =
      (HWREG(WTIMER2_BASE + TIMER_O_TBMR) & ~TIMER_TBMR_TBAMS) |
      (TIMER_TBMR_TBCDIR | TIMER_TBMR_TBCMR | TIMER_TBMR_TBMR_CAP);
// To set the event to rising edge, we need to modify the TBEVENT bits
// in GPTMCTL. Rising edge = 00, so we clear the TBEVENT bits
  HWREG(WTIMER2_BASE + TIMER_O_CTL) &=~TIMER_CTL_TBEVENT_M; 
// Now set up the port to do the capture (clock was enabled earlier)
// start by setting the alternate function for Port D bit 1 (WT2CCP1)
  HWREG(GPIO_PORTD_BASE + GPIO_O_AFSEL) |= BIT1HI;
    
// Then, map bit 1's alternate function to WT2CCP1
// 7 is the mux value to select WT2CCP1, 16 to shift it over to the
// right nibble for bit 4 (4 bits/nibble * 4 bits)
  HWREG(GPIO_PORTD_BASE + GPIO_O_PCTL) =
      (HWREG(GPIO_PORTD_BASE + GPIO_O_PCTL) & 0xfff0ffff) + (7 << 16);
// Enable pin 1 on Port D for digital I/O
  HWREG(GPIO_PORTD_BASE + GPIO_O_DEN) |= BIT1HI;
// make pin 1 on Port D into an input
  HWREG(GPIO_PORTD_BASE + GPIO_O_DIR) &= BIT1LO;
// back to the timer to enable a local capture interrupt
  HWREG(WTIMER2_BASE + TIMER_O_IMR) |= TIMER_IMR_CBEIM;
// enable the Timer B in Wide Timer 2 interrupt in the NVIC
// it is interrupt number 99 so appears in EN3 at bit 3
// go to TIVA data sheet page 106 for checking interrupt number  
// go to TIVA data sheet page 134 for checking NVIC   
  HWREG(NVIC_EN3) |= BIT3HI;

// make sure interrupts are enabled globally
// now kick the timer off by enabling it and enabling the timer to
// stall while stopped by the debugger
  HWREG(WTIMER2_BASE + TIMER_O_CTL) |= (TIMER_CTL_TBEN | TIMER_CTL_TBSTALL);
//global enable: doing it in Initialize Hardware 
  
}