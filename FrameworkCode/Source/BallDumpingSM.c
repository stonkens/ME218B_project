/****************************************************************************
 Module
   BallDumpingSM.c

 Revision
   1.0.1

 Description
   This is a template file for implementing a simple service under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"

// the common headers for C99 types 
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"

// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "termio.h"

//#include "ADMulti.h" 
#include "BITDEFS.H"
#include "BallDumpingSM.h"
#include "hw_pwm.h"

#include "MasterHSM.h"

/*----------------------------- Module Defines ----------------------------*/
//#define TEST
#define ANY_BALL 0
#define RED 1 
#define ORANGE 2
#define YELLOW 3 
#define GREEN 4
#define BLUE 5
#define PINK 6 
#define DUMP_TIME 5000 //5s
#define SORT_TIME 3000 //3s 
#define PROCESSING_TIME 200 //200ms
#define BitsPerNibble 4
#define GenB_Normal (PWM_2_GENB_ACTCMPBU_ONE | PWM_2_GENB_ACTCMPBD_ZERO)
#define GenA_Normal (PWM_2_GENA_ACTCMPAU_ONE | PWM_2_GENA_ACTCMPAD_ZERO)
#define PeriodInMS 20 //50Hz 
#define PWMTicksPerMS 40000 / 32  //system clock frequency/32
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service. They should be functions
   relevant to the behavior of this service
*/

//SERVO STUFF  
//static void SetCookieStartingPosition(void);
//static void servoPWMInit(void);
//static void RestoreCookieDC(void); 
static void dumpServoPWMInit(void); 
static void setLandfillDuty(uint32_t duty);
static void setRecyclingDuty(uint32_t duty);
static void RestoreLandfillDC(void); 
static void RestoreRecyclingDC(void); 
static void SetServoStartingPositions(void); 
/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority; 
static DumpingState_t  CurrentState;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitBallDumpingSM
 Parameters
     uint8_t : the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any
     other required initialization for this service
 Notes

****************************************************************************/

bool InitBallDumpingSM(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
 
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  puts("Ball Dumping Init");
  dumpServoPWMInit(); 
  SetServoStartingPositions(); 
  
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
     PostBallDumpingSM

 Parameters
     ES_Event ThisEvent; the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

****************************************************************************/
bool PostBallDumpingSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunHourglassService 

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   This is a run function for the hourglass service. The hourglass is used as a
   creative way to display the passage of game time to the user.

 Notes

****************************************************************************/
ES_Event_t RunBallDumpingSM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  
   switch (CurrentState)
  {
    case Waiting2Dump:
    {
      printf("In waiting to dump\r\n");
      if(ThisEvent.EventType == EV_DUMP_RECYCLE)
      {
        openRecyclingDoor(); 
        ES_Timer_InitTimer(DUMP_TIMER, DUMP_TIME); 
        CurrentState = RecycleDumping; 
      }
       if(ThisEvent.EventType == EV_DUMP_LANDFILL){
        openLandfillDoor(); 
        ES_Timer_InitTimer(DUMP_TIMER, DUMP_TIME); 
        CurrentState = LandfillDumping; 
      }
  
    }
    break;
    case RecycleDumping:
    {
      printf("Recycling\r\n");
      if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == DUMP_TIMER){
        closeRecyclingDoor(); 
        ThisEvent.EventType = EV_RECYCLING_DONE; 
        PostMasterSM(ThisEvent);
        printf("PostMasterSM in recycling");
        CurrentState = Waiting2Dump; 
      }
    }
    break;
    case LandfillDumping:
    {
        printf("LandfillDumping\r\n");
        if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == DUMP_TIMER){
        closeLandfillDoor(); 
        ThisEvent.EventType = EV_LANDFILLING_DONE; 
        PostMasterSM(ThisEvent);
        //printf("PostMasterSM in landfilling");
        CurrentState = Waiting2Dump; 
      }

    }
    break;
  }
  
  return ReturnEvent;
  }

static void dumpServoPWMInit(void){
  //Init PE4 - Dump servo 1 [Landfill] - M0 PWM4
  //Init PE5 - Dump servo 2 [Recycling] - MO PWM5 
  
  // start by enabling the clock to the PWM Module (PWM0)
  HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;
// enable the clock to Port E
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R4;
// make sure that Port D is initialized
      while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R4) != BIT4HI)
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
  HWREG(PWM0_BASE + PWM_O_2_CTL) = 0;
// program generators to go to 1 at rising compare A/B, 0 on falling compare A/B
  HWREG(PWM0_BASE + PWM_O_2_GENA) = GenA_Normal;
  HWREG(PWM0_BASE + PWM_O_2_GENB) = GenB_Normal;
// Set the PWM period. Since we are counting both up & down, we initialize
// the load register to 1/2 the desired total period. We will also program
// the match compare registers to 1/2 the desired high time
  HWREG(PWM0_BASE + PWM_O_2_LOAD) = ((PeriodInMS * PWMTicksPerMS)) / 2;
// Set the initial Duty cycle on A to 50% by programming the compare value
// to 1/2 the period to count up (or down). Technically, the value to program
// should be Period/2 - DesiredHighTime/2, but since the desired high time is 1/2
// the period, we can skip the subtract
  HWREG(PWM0_BASE + PWM_O_2_CMPA) = (HWREG(PWM0_BASE + PWM_O_2_LOAD) - HWREG(PWM0_BASE + PWM_O_2_LOAD) * 7 / 100); //HWREG(PWM0_BASE + PWM_O_2_LOAD) >> 1;
  HWREG( PWM0_BASE+PWM_O_2_CMPB) = (HWREG(PWM0_BASE + PWM_O_2_LOAD) - HWREG(PWM0_BASE + PWM_O_2_LOAD) * 7 / 100);//(HWREG( PWM0_BASE+PWM_O_2_LOAD)) -
// enable the PWM outputs
  HWREG(PWM0_BASE + PWM_O_ENABLE) |= (PWM_ENABLE_PWM4EN | PWM_ENABLE_PWM5EN);
// now configure the Port E pins to be PWM outputs
// start by selecting the alternate function for PE4 & PE5 
  HWREG(GPIO_PORTE_BASE + GPIO_O_AFSEL) |= (BIT4HI|BIT5HI);
// now choose to map PWM to those pins, this is a mux value of 4 that we
// want to use for specifying the function on bits 4 and 5 
  HWREG(GPIO_PORTE_BASE + GPIO_O_PCTL) =
      (HWREG(GPIO_PORTE_BASE + GPIO_O_PCTL) & 0xff00ffff) + (4 << (4 * BitsPerNibble)) +
      (4 << (5 * BitsPerNibble));;
// Enable pins 4 & 5 on Port E for digital I/O
  HWREG(GPIO_PORTE_BASE + GPIO_O_DEN) |= (BIT4HI|BIT5HI);
// make pins 4 & 5 on Port E into outputs
  HWREG(GPIO_PORTE_BASE + GPIO_O_DIR) |= (BIT4HI|BIT5HI);
// set the up/down count mode, enable the PWM generator and make
// both generator updates locally synchronized to zero count
  HWREG(PWM0_BASE + PWM_O_2_CTL) = (PWM_2_CTL_MODE | PWM_2_CTL_ENABLE |
      PWM_2_CTL_GENAUPD_LS | PWM_2_CTL_GENBUPD_LS);
} 

static void setLandfillDuty(uint32_t duty) //PE4
{
  if (duty == 0)
  {
    HWREG(PWM0_BASE + PWM_O_2_GENA) = PWM_2_GENA_ACTZERO_ZERO;
  }
  else if (duty == 100)
  {
    HWREG(PWM0_BASE + PWM_O_2_GENA) = PWM_2_GENA_ACTZERO_ONE;
  }
  else
  {
    RestoreLandfillDC();
    HWREG(PWM0_BASE + PWM_O_2_CMPA) = (HWREG(PWM0_BASE + PWM_O_2_LOAD) - HWREG(PWM0_BASE + PWM_O_2_LOAD) * duty / 100);
  }
}

static void setRecyclingDuty(uint32_t duty) //PE5
{
  if (duty == 0)
  {
    HWREG(PWM0_BASE + PWM_O_2_GENB) = PWM_2_GENB_ACTZERO_ZERO;
  }
  else if (duty == 100)
  {
    HWREG(PWM0_BASE + PWM_O_2_GENB) = PWM_2_GENB_ACTZERO_ONE;
  }
  else
  {
    RestoreRecyclingDC();
    HWREG(PWM0_BASE + PWM_O_2_CMPB) = (HWREG(PWM0_BASE + PWM_O_2_LOAD) - HWREG(PWM0_BASE + PWM_O_2_LOAD) * duty / 100);
  }
}

static void RestoreLandfillDC(void)
{
// To restore the previos DC, simply set the action back to the normal actions
  HWREG(PWM0_BASE + PWM_O_2_GENA) = GenA_Normal;
}

static void RestoreRecyclingDC(void)
{
// To restore the previos DC, simply set the action back to the normal actions
  HWREG(PWM0_BASE + PWM_O_2_GENB) = GenB_Normal;
}

static void SetServoStartingPositions(void)
{
  printf("Reset servos for start \r\n");
  //Landfill servo 
  setLandfillDuty(8); 
  setRecyclingDuty(4);
  //Recycling servo 

}

//servo movement for opening the recycling door 
void openRecyclingDoor(void){
  printf("Open recycling door \r\n");
  setRecyclingDuty(9);
} 

//servo movement for closing the recycling door 
void closeRecyclingDoor(void){
  printf("Close recycling door \r\n");
  setRecyclingDuty(4);
}

//servo movement for opening the landfill door 
void openLandfillDoor(void){
  printf("Open landfill door \r\n");
  setLandfillDuty(3); 
}

//servo movement for closing the landfill door 
void closeLandfillDoor(void){
  printf("Close landfill door \r\n");
  setLandfillDuty(8); 
}
  
////moves cookie in the direction of recycling 
//void sortToRecycling(void){
//    setCookieDuty(1); //reset 
//  printf("Sort to recycling \r\n");
//}
////moves cookie in the direction of landfill 
//void sortToLandfill(void){
//  printf("Sort to trash \r\n"); 
//  setCookieDuty(11); //reset 
//}

//void resetCookie(void){
//  printf("Reset cookie \r\n"); 
//}
////#define TEST

//#ifdef TEST
//#include "termio.h"
//#include <stdio.h>

//uint16_t angle = 1500; // turns servo 90 from 0 
//void ServoHardwareInit();
//void ServoHardwareInit( void)
//{
//  //Port E
//  HWREG(SYSCTL_RCGCGPIO) |= BIT4HI;
//  while((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R4) != SYSCTL_PRGPIO_R4)
//  {}  

//  //PE4 digital enable
//  HWREG(GPIO_PORTE_BASE+GPIO_O_DEN) |= (BIT4HI);

//  //Initialize PWM
//  PWM_TIVA_Init(6); 
//  PWM_TIVA_SetPeriod(25000, 4); // 50Hz 
//}


//int main(void)
//{
//  TERMIO_Init();
//  ServoHardwareInit();
//  PWM_TIVA_SetPulseWidth(angle, 4);

//  while (true) {;}
//	
//  return 0;
//}

//#endif

////#define TEST

//#ifdef TEST
//#include "termio.h"
//#include <stdio.h>

//uint16_t angle = 1500; // turns servo 90 from 0 
//void ServoHardwareInit();
//void ServoHardwareInit( void)
//{
//  //Port E
//  HWREG(SYSCTL_RCGCGPIO) |= BIT4HI;
//  while((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R4) != SYSCTL_PRGPIO_R4)
//  {}  

//  //PE4 digital enable
//  HWREG(GPIO_PORTE_BASE+GPIO_O_DEN) |= (BIT4HI);

//  //Initialize PWM
//  PWM_TIVA_Init(6); 
//  PWM_TIVA_SetPeriod(25000, 4); // 50Hz 
//}


//int main(void)
//{
//  TERMIO_Init();
//  ServoHardwareInit();
//  PWM_TIVA_SetPulseWidth(angle, 4);

//  while (true) {;}
//	
//  return 0;
//}

//#endif
