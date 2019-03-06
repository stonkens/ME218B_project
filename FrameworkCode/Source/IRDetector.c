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
#include "EventCheckers.h"

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

#include "IRDetector.h"


#include "MasterHSM.h"


#include "DriveCommandModule.h"
/*----------------------------- Module Defines -----------------------------*/
#define MAX_PERIOD_us 820
#define MIN_PERIOD_us 480
//#define ns_PER_TICK 25
#define TICKS_PER_us 40
/*---------------------------- Module Functions ---------------------------*/

/*---------------------------- Module Variables ---------------------------*/
static uint32_t Validated_LastPeriod_us; //LastPeriod value used for consistant Period reading
static uint32_t Period_In_Ticks; //The Period of captured IR
static uint32_t Period_In_us;//
static uint32_t LastCapture=0; // Time of last rising edge for IR, captured in tick
static bool FirstEdge=true; // true if it is the first edge
static uint32_t RunCount=0;//counts up whenever it sees a valid period
static uint32_t SpecificBeaconCount = 0;
static bool SpecificBeaconFinder = false;
static uint32_t BeaconPeriod;
/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
   IR_getSamePeriodRunCount(void)

 Parameters
  nothing

 Returns
  nothing

 Description
  grab the SamePeriodRunCount
 
 Notes

 Author
  Hyun Joo Lee 03/01/2019
****************************************************************************/
uint32_t IRGetRunCount(void)
{
  return RunCount;
}
/****************************************************************************
 Function
   IR_resetSamePeriodRunCount(void)

 Parameters
  nothing

 Returns
  nothing

 Description
  resets the SamePeriodRunCount to 0
 
 Notes

 Author
  Hyun Joo Lee 03/01/2019
****************************************************************************/
void IRResetRunCount(void)
{
  RunCount=0;
}
/****************************************************************************
 Function
   IRInitInputCapture(void)

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
void IRInitInputCapture(void)
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
// right nibble for bit 1 (4 bits/nibble * 1 bits)
  HWREG(GPIO_PORTD_BASE + GPIO_O_PCTL) =
      (HWREG(GPIO_PORTD_BASE + GPIO_O_PCTL) & 0xffffff0f) + (7 << 4);
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

  HWREG(NVIC_PRI24) |= BIT30HI;
// make sure interrupts are enabled globally
// now kick the timer off by enabling it and enabling the timer to
// stall while stopped by the debugger
  HWREG(WTIMER2_BASE + TIMER_O_CTL) |= (TIMER_CTL_TBEN | TIMER_CTL_TBSTALL);
//global enable: doing it in Initialize Hardware 
  IRDisableInterrupt();
	//printf("\r\n InitInputCapture for IR complete");
}

/****************************************************************************
 Function
   IR_ISR(void)

 Parameters
  nothing

 Returns
  nothing

 Description
  clear interrupt of input capture
 
 Notes

 Author
Hyun Joo Lee 15:33 02/27/2019 Started function

****************************************************************************/
void IR_ISR(void){
  uint32_t ThisCapture; //Capture the tick when the interrupt occured
  //clear interrupt
  HWREG(WTIMER2_BASE + TIMER_O_ICR)=TIMER_ICR_CBECINT;
  //grab the captured tick value
  ThisCapture=HWREG(WTIMER2_BASE + TIMER_O_TBR);
  //if firstedge, don't calculate the period
  //printf(".");

  
  if (FirstEdge)
  {
    FirstEdge=false;
  }
  else
  {
    //calculate the period in ticks
    Period_In_Ticks=ThisCapture-LastCapture;
      
    //calculate period in us(microseconds)
    Period_In_us=Period_In_Ticks/TICKS_PER_us;
    
    //if Period is out of range, reset validated period to 0 and put up the FirstEdge flag
    if((Period_In_us >= MAX_PERIOD_us) | (Period_In_us <= MIN_PERIOD_us))
    {
      FirstEdge=true;
      Validated_LastPeriod_us=0;
    }
    else
    {
      RunCount++;
      Validated_LastPeriod_us=Period_In_us;
      if(SpecificBeaconFinder == true)
      {
        if((Period_In_us >= BeaconPeriod - DETECTION_TOLERANCE) && (Period_In_us <= BeaconPeriod + DETECTION_TOLERANCE))
        {
          printf("1");
          SpecificBeaconCount++;
          
          if(SpecificBeaconCount >= 15)
          {
            ES_Event_t BeaconEvent;
            BeaconEvent.EventType = EV_ALIGNED2BEACON;
            PostMasterSM(BeaconEvent);
          
            //ADDED FOR TESTING
            //printf("Found the beacon\r\n");
            StopDrive();
            SpecificBeaconCount = 0;
          }
        }
        else
        {
          SpecificBeaconCount = 0;
        }  
      }
    }
  }
  //Update LastCapture value
  LastCapture=ThisCapture;

}

/****************************************************************************
 Function
   IRGetPeriod(void)

 Parameters
  nothing

 Returns
  Period in us of IR inputCapture

 Description
  clear interrupt of input capture
 
 Notes

 Author
Hyun Joo Lee 15:33 02/27/2019 Started function

****************************************************************************/
uint32_t IRGetPeriod(void){
    return Validated_LastPeriod_us;
}

/****************************************************************************
 Function
   IR_disable(void)

 Parameters
  nothing

 Returns
  nothing

 Description
  disables the input capture
  called when not searching for landfill/recycling centor or navigating  
 Notes

 Author
Hyun Joo Lee 16:00 02/27/2019 Started function

****************************************************************************/
void IRDisableInterrupt(void){
    HWREG(WTIMER2_BASE +TIMER_O_IMR) &=~TIMER_IMR_CBEIM;
}

/****************************************************************************
 Function
   IR_enable(void)

 Parameters
  nothing

 Returns
  nothing

 Description
  enables the input capture
  called when searching for landfill/recycling centor or navigating  
 Notes

 Author
Hyun Joo Lee 16:10 02/27/2019 Started function

****************************************************************************/
void IREnableInterrupt(void){
  //Clear source of interrupt
  HWREG(WTIMER2_BASE+TIMER_O_ICR) = TIMER_ICR_CBECINT;
  //Enable interrupts
  HWREG(WTIMER2_BASE +TIMER_O_IMR) |=TIMER_IMR_CBEIM;
}
///****************************************************************************
// Function
//   IR_found(void)

// Parameters
//  nothing

// Returns
//  true when IR(recylcing center or ) is found 

// Description
//Modular: removable to another location
//    Event checker for
//WestRecycling_Found: 600us
//EastRecycling_Found: 500us
//SouthLandfill_Found: 700us
//NorthLandfill_Found: 800us

// Notes

// Author
// Hyun Joo Lee 15:18 02/28/2019 Started function

//****************************************************************************/
//bool IR_found(void){
//  bool ReturnVal=false;
//  uint32_t FoundPeriod;
//  FoundPeriod=IRGetPeriod();
//  if((FoundPeriod >790)&&(FoundPeriod<810))
//  {
//    ES_Event_t ThisEvent;
//    ThisEvent.EventType=ES_NORTHLANDFILL_FOUND;
//    ReturnVal =true;
//    PostDCMotorService(ThisEvent);
//    SamePeriodRunCount++;
//    printf("\r\n NorthLandfill_Found");
//  }else if((FoundPeriod >690)&&(FoundPeriod<710))
//  {
//    ES_Event_t ThisEvent;
//    ThisEvent.EventType=ES_SOUTHLANDFILL_FOUND;
//    ReturnVal =true;
//    PostDCMotorService(ThisEvent);
//    SamePeriodRunCount++;
//    printf("\r\n SouthLandfill_Found");
//  }else if((FoundPeriod >590)&&(FoundPeriod<610))
//  {
//    ES_Event_t ThisEvent;
//    ThisEvent.EventType=ES_WESTRECYCLING_FOUND;
//    ReturnVal =true;
//    PostDCMotorService(ThisEvent);
//    SamePeriodRunCount++;
//    printf("\r\n WestRecycling_Found");
//  }else if((FoundPeriod >490)&&(FoundPeriod<510))
//  {
//    ES_Event_t ThisEvent;
//    ThisEvent.EventType=ES_EASTRECYCLING_FOUND;
//    ReturnVal =true;
//    PostDCMotorService(ThisEvent);
//    SamePeriodRunCount++;
//    printf("\r\n EastRecycling_Found");
//  }
//  return ReturnVal;
//}


void ActivateBeaconFinder(uint32_t Period)
{
  SpecificBeaconFinder = true;
  BeaconPeriod = Period;
}

void DisableBeaconFinder(void)
{
  SpecificBeaconFinder = false;
  BeaconPeriod = 0;
}
