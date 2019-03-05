
/****************************************************************************
 Module
   TapeFollowingService.c

 Revision
   1.0.1

 Description
   Event Checking Module, checks whether a ball is detected and what color it is 
   
   Local ReturnVal = False
   Set the previous clear and RBG values to the current clear & RBG values 
   Set the tolerance value to 20
   Read the new color sensor values 
   If the clear values are above or below the previous clear values by at least the tolerance band
      Determine what color the ball is 
      Set the event type to ES_BALL_DETECTED
      Post an event with the color parameter to BallProcessingSM 
      Set the ReturnVal to true
    Return the ReturnVal
    
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/06/19 17:50 nm    ?? 
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
#define TEST
//#ifdef TEST

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "termio.h"

// Specific Hardware

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"
#include "EventCheckers.h"
//#include "MasterHSM.h"

// Project modules
#include "TapeFollowingChecker.h"
#include "DriveCommandModule.h"

// This module
#include "TapeFollowingService.h"

/*#ifdef TEST
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"
#include "termio.h"
#include "EnablePA25_PB23_PD7_PF0.h"

#include "InitializeHardware.h"
#include "TapeFollowingChecker.h"

#define clrScrn() printf("\x1b[2J")
#define goHome() printf("\x1b[1,1H")
#define clrLine() printf("\x1b[K")
#endif
*/

#define BBW 001
#define BWB 010 //impossibru
#define BWW 011 
#define WBB 100
#define WBW 101
#define WBB 100
#define WWB 110
#define WWW 111

#define RPM 100 // halfspeed
#define DIST 100 //1 inch
#define DEG 450 // 45 deg
#define LostTime 5000

/*---------------------------- Module Variables ---------------------------*/
static TapeState CurrentState = InitTapeState;
static TapeState NextState;
static TapeState NewState;

static uint8_t    MyPriority;

static uint8_t TapeVals;
static uint8_t LeftTape;
static uint8_t RightTape;
static uint8_t MidTape;


/*------------------------Function Prototypes------------------------------*/
int GetNextTapeState(int);

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitTapeFollowingService

 Parameters
     uint8_t: the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial state
     Initializes lines used for Tape Sensing.
 Notes

 Author
     
****************************************************************************/


bool InitTapeFollowingService(uint8_t Priority)
{
  ES_Event_t ThisEvent;
  printf("\r\n Initializing InitTapeFollowingService");
  MyPriority = Priority;

  // Initialize HW for PWM lines is done in InitializeHardware.c
  InitTapeHardware();
  
  /*
  void InitTapeHardware(void){
  //printf("Initializing tape hardware \r\n");
  //initialize PE1 & PE2 as digital inputs
     // Set bit 4 and enable port E
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R4;
  // Wait for peripheral E to be ready
  while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R4) != SYSCTL_PRGPIO_R4)
  {
    ;
  }
  // Set PE1, PE2 to usable pins 
  HWREG(GPIO_PORTE_BASE + GPIO_O_DEN) |= (BIT1HI | BIT2HI);
  // Set PE0, PE1, PE2 to input
  HWREG(GPIO_PORTE_BASE + GPIO_O_DIR) &= (BIT1LO & BIT2LO);
  // Set PB7 to usable pins
  HWREG(GPIO_PORTB_BASE + GPIO_O_DEN) |= (BIT7HI);
  // Set PE0, PE1, PE2 to input
  HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) &= (BIT7LO);
  disableTapeFollow();
}*/


  //InitializeHardware();

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
     PostTapeFollowingService

 Parameters
     EF_Event ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     Sander Tonkens, 02/05/2018, 18:11
****************************************************************************/

bool PostTapeFollowingService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunTapeFollowingService

 Parameters
   ES_Event: the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   Runs state machine for Tape Following
 Notes

 Author

****************************************************************************/
ES_Event_t RunTapeFollowingService(ES_Event_t ThisEvent){

  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  //printf("\r\n  CurrentTapeState is: %d", CurrentState);
  int val;
   ES_Timer_SetTimer(TAPE_TIMER, (LostTime));
        
  switch (CurrentState)
  {
    case InitTapeState:
    {
      if (ThisEvent.EventType == ES_INIT)
      {
        printf("\r\n In init state of TapeFollowingService\r\n");
        CurrentState = LostTape;
      }
    }
    case LostTape:
    {
      ES_Timer_StartTimer(TAPE_TIMER);
      printf("\r\n -------------------LostTape Tape State------------------- \r");
      // turn in small circle
      if(ThisEvent.EventType != (EV_TAPE_TIMEOUT))
      {
        if(ThisEvent.EventType == (EV_NEW_TAPE))
     { 
        val = ThisEvent.EventParam;
        NextState = GetNextTapeState(val);
     }
       
        if (NextState != CurrentState)
        {
          CurrentState = NextState;
        }
      }
      else
      {
        PostTapeFollowingService(ThisEvent);//Post to master Service about timeout to return to align with recycling;
       // CurrentState = ForwardTape;
      }
      
    } break;
    
    case ForwardTape:
    {
      printf("\r\n--------------- Forward---------------\r\n");
     StopDrive(); 
     DriveStraight(RPM,DIST);
     if(ThisEvent.EventType == (EV_NEW_TAPE))
     {
      val = ThisEvent.EventParam;
        NextState = GetNextTapeState(val);
      if (NextState != CurrentState)
      {
        CurrentState = NextState;
        
      }
     }
    }
    break;
    
    case RotateRight:
    {
      printf("\r\n ---------------RotateRight----------------\r\n");
      StopDrive(); 
      DriveRotate(RPM,DEG);
     
     if(ThisEvent.EventType == (EV_NEW_TAPE))
     {
      val = ThisEvent.EventParam;
        NextState = GetNextTapeState(val);
      if (NextState != CurrentState)
      {
        CurrentState = NextState;
       
      }
     }
    
    }
    break;
    
    case RotateLeft:
    {
      printf("\r\n--------------- RotateLeft-------------------------\r\n");
      StopDrive(); 
      DriveRotate(RPM,DEG * -1);
     
     if(ThisEvent.EventType == (EV_NEW_TAPE))
     {
      val = ThisEvent.EventParam;
        NextState = GetNextTapeState(val);
      if (NextState != CurrentState)
      {
        CurrentState = NextState;
        
      }
     }
    
    }
    break;
    
    case RotateRightFine:
    {
      printf("\r\n---------------RotateRightFine-------------------------\r\n");
      StopDrive(); 
      DriveRotate(RPM,DEG/2);
     
     if(ThisEvent.EventType == (EV_NEW_TAPE))
     {
      val = ThisEvent.EventParam;
        NextState = GetNextTapeState(val);
      if (NextState != CurrentState)
      {
        CurrentState = NextState;
       
      }
     }
    
    }
    break;
    
    case RotateLeftFine:
    {
      printf("\r\n------------------- RotateLeftFine--------------------------\r\n");
      StopDrive(); 
      DriveRotate(RPM,DEG/2 * -1);
     
     if(ThisEvent.EventType == (EV_NEW_TAPE))
     {
      val = ThisEvent.EventParam;
        NextState = GetNextTapeState(val);
      if (NextState != CurrentState)
      {
        CurrentState = NextState;
        
      }
     }
    
    }
    break;
  }
return ReturnEvent;
}

int GetNextTapeState(int val) {
   LeftTape = (HWREG(GPIO_PORTE_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT1HI);
   RightTape = (HWREG(GPIO_PORTE_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT2HI);
   MidTape = (HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT7HI);
  
  /*if ((HWREG(GPIO_PORTE_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT1HI) > 0)
  {
    LeftTape = 1;
  }
  if ((HWREG(GPIO_PORTE_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT2HI) > 0)
  {
    RightTape = 1;
  }
  
  if ((HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT7HI) > 0)
  {
    MidTape = 1; //note that this bit might be set to PWM somewhere;
  }*/
  
  
 //  printf("TapeValue = %d", TapeVals);
   TapeVals = LeftTape*100 + MidTape + RightTape*1;
   //TapeVals = val;
  // printf("TapeValue = %d", TapeVals);
  
   if (TapeVals == 111){
     NewState = LostTape;
   }
  
   else if (TapeVals == 110){
     NewState = RotateRight;
   }
   else if (TapeVals == 101){
     NewState = ForwardTape;
   }
   else if (TapeVals == 100) {
     NewState = RotateRightFine;
   }
   else if (TapeVals == 011) {
     NewState = RotateLeft;
   }
   else if (TapeVals == 010) { //IMPOSSIBRU state
     NewState = ForwardTape;
   }
   else if (TapeVals == 001) {
     NewState = RotateLeftFine;
   }
   else if (TapeVals == 000) {
     NewState = ForwardTape;
   }
   else
   {
     NewState = LostTape;
   }
   
  // printf("return in GetNextTapeState %d ", NewState);
   
   printf("\r\n return in GetNextTapeState %d ", NewState);
   return NewState;
}


