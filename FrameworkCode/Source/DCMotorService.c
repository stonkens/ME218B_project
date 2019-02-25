/****************************************************************************
 Module
   DCMotorService.c

 Revision
   1.0.1

 Description
   Runs the DC Motors associated with ball collection 
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/24/19 17:50 kchen    First pass 
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
// Hardware
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "termio.h"

// Specific Hardware
#include "hw_pwm.h"

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"

// Project modules
#include "PWM.h"

// This module
#include "DCMotorService.h"

/*----------------------------- Module Defines ----------------------------*/

#define BitsPerNibble 4
#define GenB_Normal (PWM_0_GENB_ACTCMPBU_ONE | PWM_0_GENB_ACTCMPBD_ZERO)
#define GenA_Normal (PWM_0_GENA_ACTCMPBU_ONE | PWM_0_GENA_ACTCMPBD_ZERO)
#define PeriodInMS 5 //200Hz 
#define PWMTicksPerMS 40000 / 32  //system clock frequency/32
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.
*/
//#define TEST
static void InitDCPWM(void);  
static void setPickupDuty(uint32_t duty); 
static void setTransportDuty(uint32_t duty); 
static void RestorePickupDC(void); 
static void RestoreTransportDC(void); 

static uint32_t Startup_Period_ms = 5;
static uint32_t Period_0; // ticks
static uint32_t Duty_0;   // percent

/*---------------------------- Module Variables ---------------------------*/
static uint8_t    MyPriority;
/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitDCMotorService

 Parameters
     uint8_t: the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and determines
     Drive type. Initializes PWM lines and sets enable lines as output.
 Notes

 Author

****************************************************************************/
bool InitDCMotorService(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;

  // Initialize HW for PWM lines will be done in InitializeHardware.c
  InitDCPWM();
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService(MyPriority, ThisEvent) == true)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/****************************************************************************
 Function
     PostDCMotorService

 Parameters
     EF_Event ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author

****************************************************************************/

bool PostDCMotorService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunDCMotorService

 Parameters
   ES_Event: the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   Regulates commands, and sets motors accordingly
 Notes

 Author
****************************************************************************/
ES_Event_t RunDCMotorService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  return ReturnEvent;
}

/****************************************************************************
 Function
    startPickupMotorService

 Parameters
   ES_Event: the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   Regulates commands, and sets motors accordingly
 Notes

 Author
****************************************************************************/
void startPickupMotor(uint32_t DutyCycle){
  setPickupDuty(DutyCycle); 
}

void stopPickupMotor(){
  setPickupDuty(0); 
} 

void startTransportMotor(uint32_t DutyCycle){
  setTransportDuty(DutyCycle); 
}

void stopTransportMotor(){
  setTransportDuty(0); 
} 

/***************************************************************************
 private functions
 ***************************************************************************/
/* Functions private to the module */

static void setPickupDuty(uint32_t duty) //PB7
{
  if (duty == 0)
  {
    HWREG(PWM0_BASE + PWM_O_0_GENB) = PWM_0_GENB_ACTZERO_ZERO;
  }
  else if (duty == 100)
  {
    HWREG(PWM0_BASE + PWM_O_0_GENB) = PWM_0_GENB_ACTZERO_ONE;
  }
  else
  {
    RestorePickupDC();
    HWREG(PWM0_BASE + PWM_O_0_CMPB) = (HWREG(PWM0_BASE + PWM_O_0_LOAD) - HWREG(PWM0_BASE + PWM_O_0_LOAD) * duty / 100);
  }
}

static void setTransportDuty(uint32_t duty) //PB6
{
  if (duty == 0)
  {
    HWREG(PWM0_BASE + PWM_O_0_GENA) = PWM_0_GENA_ACTZERO_ZERO;
  }
  else if (duty == 100)
  {
    HWREG(PWM0_BASE + PWM_O_0_GENA) = PWM_0_GENA_ACTZERO_ONE;
  }
  else
  {
    RestoreTransportDC();
    HWREG(PWM0_BASE + PWM_O_0_CMPA) = (HWREG(PWM0_BASE + PWM_O_0_LOAD) - HWREG(PWM0_BASE + PWM_O_0_LOAD) * duty / 100);
  }
}

static void RestorePickupDC(void)
{
// To restore the previos DC, simply set the action back to the normal actions
  HWREG(PWM0_BASE + PWM_O_0_GENB) = GenB_Normal;
}
static void RestoreTransportDC(void)
{
// To restore the previos DC, simply set the action back to the normal actions
  HWREG(PWM0_BASE + PWM_O_0_GENA) = GenA_Normal;
}

void PabloTeamsCode(void){
  {
      // start by enabling the clock to the PWM Module (PWM0)
      HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;

      // enable the clock to Port B
      HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;

      // make sure that Port B is initialized
      while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != BIT1HI)
      {}

      // Select the PWM clock as System Clock/32
      HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) |
          (SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);

      // make sure that the PWM module clock has gotten going
      while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0)
      {}

      // disable the PWM while initializing
      HWREG(PWM0_BASE + PWM_O_0_CTL) = 0;

      // program generators to go to 1 at rising compare A/B, 0 on falling
      // compare A/B
      HWREG(PWM0_BASE + PWM_O_0_GENA) = GenA_Normal;
      HWREG(PWM0_BASE + PWM_O_0_GENB) = GenB_Normal;

      // Set the PWM period. Since we are counting both up & down, we
      // initialize the load register to 1/2 the desired total period. We
      // will also program the match compare registers to 1/2 the desired
      // high time
      Period_0 = Startup_Period_ms * PWMTicksPerMS;
      HWREG(PWM0_BASE + PWM_O_0_LOAD) = ((Period_0)) >> 1;

      // Set the initial Duty cycle on A to 50% by programming the compare
      // value to 1/2 the period to count up (or down). Technically, the
      // value to program should be Period/2 - DesiredHighTime/2, but since
      // the desired high time is 1/2 the period, we can skip the subtract
      Duty_0 = 50;
      HWREG(PWM0_BASE + PWM_O_0_CMPA) = HWREG(PWM0_BASE + PWM_O_0_LOAD) >> 1;
      // Set the initial Duty cycle on B to 50% like A
      HWREG(PWM0_BASE + PWM_O_0_CMPB) = HWREG(PWM0_BASE + PWM_O_0_LOAD) >> 1;

      // enable the PWM outputs
      HWREG(PWM0_BASE + PWM_O_ENABLE) |= (PWM_ENABLE_PWM0EN | PWM_ENABLE_PWM1EN);

      // now configure the Port B pins to be PWM outputs
      // start by selecting the alternate function for PB6 and PB7
      HWREG(GPIO_PORTB_BASE + GPIO_O_AFSEL) |= (BIT6HI | BIT7HI);

      // now choose to map PWM to those pins, this is a mux value of 4 that
      // we want to use for specifying the function on bits 6 and 7
      HWREG(GPIO_PORTB_BASE + GPIO_O_PCTL) =
          (HWREG(GPIO_PORTB_BASE + GPIO_O_PCTL) & 0x00ffffff)
          + (4 << (6 * BitsPerNibble)) + (4 << (7 * BitsPerNibble));

      // Enable pins 6 and 7 on Port B for digital I/O
      HWREG(GPIO_PORTB_BASE + GPIO_O_DEN) |= (BIT6HI | BIT7HI);
      // make pins 6 and 7 on Port B an output
      HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) |= (BIT6HI | BIT7HI);

      // set the up/down count mode, enable the PWM generator and make
      // both generator updates locally synchronized to zero count
      HWREG(PWM0_BASE + PWM_O_0_CTL) = (PWM_0_CTL_MODE | PWM_0_CTL_ENABLE |
          PWM_0_CTL_GENAUPD_LS | PWM_0_CTL_GENBUPD_LS);
    }
}

void InitPWMDemo( void ){
// start by enabling the clock to the PWM Module (PWM0)
HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;
// enable the clock to Port B
HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
       while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != BIT1HI)
      {}; 
// Select the PWM clock as System Clock/32
HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) |
(SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);
// make sure that the PWM module clock has gotten going
while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0)
;
// disable the PWM while initializing
HWREG( PWM0_BASE+PWM_O_0_CTL ) = 0;
// program generators to go to 1 at rising compare A/B, 0 on falling compare A/B
// GenA_Normal = (PWM_0_GENA_ACTCMPAU_ONE | PWM_0_GENA_ACTCMPAD_ZERO )
HWREG( PWM0_BASE+PWM_O_0_GENA) = GenA_Normal;
// GenB_Normal = (PWM_0_GENB_ACTCMPBU_ONE | PWM_0_GENB_ACTCMPBD_ZERO )
HWREG( PWM0_BASE+PWM_O_0_GENB) = GenB_Normal;
// Set the PWM period. Since we are counting both up & down, we initialize
// the load register to 1/2 the desired total period. We will also program
// the match compare registers to 1/2 the desired high time
HWREG( PWM0_BASE+PWM_O_0_LOAD) = ((PeriodInMS * PWMTicksPerMS))>>1; 
// Set the initial Duty cycle on A to 50% by programming the compare value
// to 1/2 the period to count up (or down). Technically, the value to program
// should be Period/2 - DesiredHighTime/2, but since the desired high time is 1/2
// the period, we can skip the subtract
HWREG( PWM0_BASE+PWM_O_0_CMPA) = HWREG( PWM0_BASE+PWM_O_0_LOAD)>>1;
HWREG( PWM0_BASE+PWM_O_0_CMPB) = HWREG( PWM0_BASE+PWM_O_0_LOAD)>>1;
// Set the initial Duty cycle on B to 25% by programming the compare value
// to Period/2 - Period/8 (75% of the period)
//HWREG( PWM0_BASE+PWM_O_0_CMPB) = (HWREG( PWM0_BASE+PWM_O_0_LOAD)) -
//(((PeriodInMS * PWMTicksPerMS))>>3);
// enable the PWM outputs
HWREG( PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM1EN | PWM_ENABLE_PWM0EN);
// now configure the Port B pins to be PWM outputs
// start by selecting the alternate function for PB6 & 7
HWREG(GPIO_PORTB_BASE+GPIO_O_AFSEL) |= (BIT7HI | BIT6HI);
// now choose to map PWM to those pins, this is a mux value of 4 that we
// want to use for specifying the function on bits 6 & 7
HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) =
(HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) & 0x00ffffff) + (4<<(7*BitsPerNibble)) +
(4<<(6*BitsPerNibble));
// Enable pins 6 & 7 on Port B for digital I/O
HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (BIT7HI | BIT6HI);
// make pins 6 & 7 on Port B into outputs
HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (BIT7HI |BIT6HI);
// set the up/down count mode, enable the PWM generator and make
// both generator updates locally synchronized to zero count
HWREG(PWM0_BASE+ PWM_O_0_CTL) = (PWM_0_CTL_MODE | PWM_0_CTL_ENABLE |
PWM_0_CTL_GENAUPD_LS | PWM_0_CTL_GENBUPD_LS);
}

static void InitDCPWM(void)
{
// start by enabling the clock to the PWM Module (PWM0)
  HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;
// enable the clock to Port B
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
// make sure that Port B is initialized
      while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != BIT1HI)
      {}; 
// Select the PWM clock as System Clock/32
  HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) |
      (SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);
// make sure that the PWM module clock has gotten going
  while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0)
  {
    ;
  }
// disable the PWM while initializing
  HWREG(PWM0_BASE + PWM_O_0_CTL) = 0;
// program generators to go to 1 at rising compare A/B, 0 on falling compare A/B
  HWREG(PWM0_BASE + PWM_O_0_GENA) = GenA_Normal;
  HWREG(PWM0_BASE + PWM_O_0_GENB) = GenB_Normal;
// Set the PWM period. Since we are counting both up & down, we initialize
// the load register to 1/2 the desired total period. We will also program
// the match compare registers to 1/2 the desired high time
  HWREG(PWM0_BASE + PWM_O_0_LOAD) = ((PeriodInMS * PWMTicksPerMS)) / 2;
// Set the initial Duty cycle on A to 50% by programming the compare value
// to 1/2 the period to count up (or down). Technically, the value to program
// should be Period/2 - DesiredHighTime/2, but since the desired high time is 1/2
// the period, we can skip the subtract
  HWREG(PWM0_BASE + PWM_O_0_CMPA) = HWREG(PWM0_BASE + PWM_O_0_LOAD) >> 1;
  //HWREG(PWM0_BASE + PWM_O_0_CMPB) = HWREG(PWM0_BASE + PWM_O_0_LOAD) >> 1;
// enable the PWM outputs
  HWREG(PWM0_BASE + PWM_O_ENABLE) |= (PWM_ENABLE_PWM1EN | PWM_ENABLE_PWM0EN);
// now configure the Port B pins to be PWM outputs
// start by selecting the alternate function for PB6 & 7
  HWREG(GPIO_PORTB_BASE + GPIO_O_AFSEL) |= (BIT7HI | BIT6HI);
// now choose to map PWM to those pins, this is a mux value of 4 that we
// want to use for specifying the function on bits 6 & 7
  HWREG(GPIO_PORTB_BASE + GPIO_O_PCTL) =
      (HWREG(GPIO_PORTB_BASE + GPIO_O_PCTL) & 0x00ffffff) + (4 << (7 * BitsPerNibble)) +
      (4 << (6 * BitsPerNibble));
// Enable pins 6 & 7 on Port B for digital I/O
  HWREG(GPIO_PORTB_BASE + GPIO_O_DEN) |= (BIT7HI | BIT6HI);
// make pins 6 & 7 on Port B into outputs
  HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) |= (BIT7HI | BIT6HI);
// set the up/down count mode, enable the PWM generator and make
// both generator updates locally synchronized to zero count
  HWREG(PWM0_BASE + PWM_O_0_CTL) = (PWM_0_CTL_MODE | PWM_0_CTL_ENABLE |
      PWM_0_CTL_GENAUPD_LS | PWM_0_CTL_GENBUPD_LS);
}

//#ifdef TEST    
//#include <stdint.h>
//#include <stdbool.h>
//#include <stdio.h>

//#include "inc/hw_types.h"
//#include "inc/hw_memmap.h"
//#include "driverlib/sysctl.h"

//#include "ES_Configure.h"
//#include "ES_Framework.h"
//#include "ES_Port.h"
//#include "termio.h"
//#include "EnablePA25_PB23_PD7_PF0.h"

//#include "DCMotorService.h"
//#include "hw_pwm.h"
//#include "PWM.h"

//#define clrScrn() printf("\x1b[2J")
//#define goHome() printf("\x1b[1,1H")
//#define clrLine() printf("\x1b[K")

//int main(void)
//{
//  ES_Return_t ErrorType;

//  // Set the clock to run at 40MhZ using the PLL and 16MHz external crystal
//  SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN
//      | SYSCTL_XTAL_16MHZ);
//  TERMIO_Init();
//  clrScrn();

//  // When doing testing, it is useful to announce just which program
//  // is running.
//  puts("\rStarting Pickup Test Harness for \r");
//  printf( "the 2nd Generation Events & Services Framework V2.4\r\n");
//  printf( "%s %s\n", __TIME__, __DATE__);
//  printf( "\n\r\n");

//  // reprogram the ports that are set as alternate functions or
//  // locked coming out of reset. (PA2-5, PB2-3, PD7, PF0)
//  // After this call these ports are set
//  // as GPIO inputs and can be freely re-programmed to change config.
//  // or assign to alternate any functions available on those pins
//  PortFunctionInit();

//  // Your hardware initialization function calls go here
//  PabloTeamsCode(); 
//  //InitPWMDemo(); //this outputs two waveforms of 25% duty cycle (CMPB) CMPA NEVER works
//  //InitDCPWM();
//  //startPickupMotor(25); 
//  //printf("\r\n Started pickup motor");
//  //stopPickupMotor(); 
//  
//  
//  // now initialize the Events and Services Framework and start it running
//  ErrorType = ES_Initialize(ES_Timer_RATE_1mS);
//  if (ErrorType == Success)
//  {
//    ErrorType = ES_Run();
//  }
//  //if we got to here, there was an error
//  switch (ErrorType)
//  {
//    case FailedPost:
//    {
//      printf("Failed on attempt to Post\n");
//    }
//    break;
//    case FailedPointer:
//    {
//      printf("Failed on NULL pointer\n");
//    }
//    break;
//    case FailedInit:
//    {
//      printf("Failed Initialization\n");
//    }
//    break;
//    default:
//    {
//      printf("Other Failure\n");
//    }
//    break;
//  }
//  for ( ; ;)
//  {
//    ;
//  }
//}

//#endif

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
