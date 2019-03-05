
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


#define RPM 50 // halfspeed
#define DIST 100 //1 inch
#define DEG 450 // 45 deg
#define LOSTTIME 5000

#define ENTRY_STATE LostTape
/*---------------------------- Module Variables ---------------------------*/
static TapeState_t CurrentState;
static TapeState_t NextState;
static uint8_t TapeVals;
static uint8_t LeftTape;
static uint8_t RightTape;
static uint8_t MidTape;
static uint8_t TapeStatus;


/*------------------------Function Prototypes------------------------------*/
static uint8_t GetNextTapeState(void);

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
void StartTapeFollowingSM ( ES_Event_t CurrentEvent )
{
   // to implement entry to a history state or directly to a substate
   // you can modify the initialization of the CurrentState variable
   // otherwise just start in the entry state every time the state machine
   // is started
   if ( ES_ENTRY_HISTORY != CurrentEvent.EventType )
   {
        CurrentState = ENTRY_STATE;
   }
   // call the entry function (if any) for the ENTRY_STATE
   RunTapeFollowingSM(CurrentEvent);
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
ES_Event_t RunTapeFollowingSM(ES_Event_t CurrentEvent)
{
  TapeState_t NextState = CurrentState;
  ES_Event_t EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
  ES_Event_t ReturnEvent = CurrentEvent; // assume we are not consuming event
  //printf("\r\n  CurrentTapeState is: %d", CurrentState);
  switch (CurrentState)
  {
    
    case LostTape:
    {
      if((CurrentEvent.EventType == (EV_NEW_TAPE)) || (CurrentEvent.EventType == ES_ENTRY) ||(CurrentEvent.EventType == ES_ENTRY_HISTORY))
      {
        ReturnEvent.EventType = ES_NO_EVENT;
        TapeStatus = GetNextTapeState();
        
        if(TapeStatus == 110)
        {
          StopDrive(); 
          DriveRotate(RPM,DEG);
        }
        else if(TapeStatus == 101)
        {
          StopDrive();
          DriveStraight(RPM,DIST);
        }
        else if(TapeStatus == 100)
        {
          StopDrive(); 
          DriveRotate(RPM,DEG/2);
        }
        else if (TapeStatus == 011)
        {
          StopDrive();
          DriveRotate(RPM,-DEG);
        }
        else if(TapeStatus == 010)
        {
          StopDrive();
          DriveStraight(RPM,DIST);
          
        }
        else if(TapeStatus == 001)
        {
          StopDrive();
          DriveRotate(RPM,-DEG/2);          
        }
        else if(TapeStatus == 000)
        {
          StopDrive();
          DriveStraight(RPM,DIST);
        }
        NextState = TapeFollowing;
      }
    }
    break;
    
    case TapeFollowing:
    {
      if(CurrentEvent.EventType == (EV_NEW_TAPE))
      {
        ReturnEvent.EventType = ES_NO_EVENT;
        TapeStatus = GetNextTapeState();
        
        if(TapeStatus == 110)
        {
          StopDrive(); 
          DriveRotate(RPM,DEG);
        }
        else if(TapeStatus == 101)
        {
          StopDrive();
          DriveStraight(RPM,DIST);
        }
        else if(TapeStatus == 100)
        {
          StopDrive(); 
          DriveRotate(RPM,DEG/2);
        }
        else if (TapeStatus == 011)
        {
          StopDrive();
          DriveRotate(RPM,-DEG);
        }
        else if(TapeStatus == 010)
        {
          StopDrive();
          DriveStraight(RPM,DIST);
          
        }
        else if(TapeStatus == 001)
        {
          StopDrive();
          DriveRotate(RPM,-DEG/2);          
        }
        else if(TapeStatus == 000)
        {
          StopDrive();
          DriveStraight(RPM,DIST);
        }
        else if(TapeStatus == 111)
        {
          StopDrive();
          DriveRotate(RPM, 3600);
          ES_Timer_InitTimer(TAPE_TIMER, LOSTTIME);
          NextState = LostTape;
        }
      }
    }
    break;
    
    default:
    {;
    }
  }
  CurrentState = NextState;
  return ReturnEvent;
}

static uint8_t GetNextTapeState(void) {
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
   TapeVals = LeftTape*100 + MidTape*10 + RightTape*1;
   //TapeVals = val;
  // printf("TapeValue = %d", TapeVals);
  
 
   
  // printf("return in GetNextTapeState %d ", NewState);
   
   //printf("\r\n return in GetNextTapeState %d ", NewState);
   return TapeVals;
}


