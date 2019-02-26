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
#include "DriveMotorPWM.h"

// This module
#include "DCMotorService.h"

/*----------------------------- Module Defines ----------------------------*/

#define BitsPerNibble 4
#define GenB_Normal (PWM_0_GENB_ACTCMPBU_ONE | PWM_0_GENB_ACTCMPBD_ZERO)
#define GenA_Normal (PWM_0_GENA_ACTCMPAU_ONE | PWM_0_GENA_ACTCMPAD_ZERO)
#define PeriodInMS 5 //200Hz 
#define PWMTicksPerMS 40000 / 32  //system clock frequency/32
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.
*/ 
static void setPickupDuty(uint32_t duty); 
static void setTransportDuty(uint32_t duty); 
static void RestorePickupDC(void); 
static void RestoreTransportDC(void); 



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
  //InitDCPWM();
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

void InitDCPWM(void)
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
  HWREG( PWM0_BASE+PWM_O_0_CMPB) = (HWREG( PWM0_BASE+PWM_O_0_LOAD)) -
(((PeriodInMS * PWMTicksPerMS))>>3);
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


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
