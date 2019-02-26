
/****************************************************************************
 Module
   MotorService.c

 Revision
   1.0.1

 Description
   Processes CommandGenerator commands, sets motors A and B accordingly

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/06/19 17:50 ston    Customized to Lab 8 functionality
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
#include "MotorService.h"
//#include "CommunicationSSI.h"
//#include "InitializeHardware.h"

/*----------------------------- Module Defines ----------------------------*/
#define MAX_SPEED_A 95
#define MAX_SPEED_B 100
#define HALF_SPEED_A 65
#define HALF_SPEED_B 65

// Defined in ms
#define QUARTER_TURN 3500
#define EIGHTH_TURN QUARTER_TURN / 2

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.
*/

/*---------------------------- Module Variables ---------------------------*/
static MotorState CurrentState = InitState;

static uint8_t    MyPriority;
/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitMotorService

 Parameters
     uint8_t: the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and determines
     Drive type. Initializes PWM lines and sets enable lines as output.
 Notes

 Author
     Sander Tonkens, 02/05/2018, 18:11
****************************************************************************/
bool InitMotorService(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;

  // Initialize HW for PWM lines is done in InitializeHardware.c
//  InitMotorGPIO();
//  InitPWM();


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
     PostMotorService

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

bool PostMotorService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunMotorService

 Parameters
   ES_Event: the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   Regulates commands, and sets motors accordingly
 Notes

 Author
     Sander Tonkens, 02/05/2018, 18:11
****************************************************************************/
ES_Event_t RunMotorService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  printf("\r\n  CurrentMotorState is: %d", CurrentState);
  switch (CurrentState)
  {
    case InitState:
    {
      if (ThisEvent.EventType == ES_INIT)
      {
        printf("\r\n In init state of MotorService\r\n");
        //Transitioning to RegularOperation state
        PWMSetDuty(0, 0, ADVANCE);
        CurrentState = Forward;
      }
    }
    
    case Forward:
    {
      if (ThisEvent.EventType == ES_CLEANING_UP)
      {
        PWMSetDuty(HALF_SPEED_A, HALF_SPEED_B, ADVANCE);
      
      /*  HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT0HI;
        HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT1HI;
      HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT7HI;
      printf("set bits 0,1,7 high");      
      
        HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT4LO;
        HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT5LO; */
      }
      
      if(ThisEvent.EventType == ES_GAME_OVER)
      {
        PWMSetDuty(0, 0, ADVANCE);
      }
      
      if(ThisEvent.EventType == ES_BUMPER_HIT)
      {
        printf("\r\n Changing direction from Forward to Backward");
        CurrentState = Backward;
        
      }break;
    }
    case Backward:
    {
       PWMSetDuty(HALF_SPEED_A, HALF_SPEED_B, REVERSE);
     /*   HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT4HI;
        HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT5HI;
        
        HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT0LO;
        HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT1LO;
    HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT7LO;
    printf("\r\n set bits 0,1,7 low"); */
    
      if(ThisEvent.EventType == ES_BUMPER_HIT)
      {
        printf("\r\n Changing direction from Backward to Forward");
        //PWMSetDuty(MAX_SPEED_A, MAX_SPEED_B, ADVANCE);
        CurrentState = Forward;
       
      } break;
    }
     
    

    /*case RegularOperation:
    {
      if (ThisEvent.EventType == ES_NEW_COMMAND)
      {
        printf("\r\n Received new command in MotorService. Command: %d\r\n", ThisEvent.EventParam);
        //Make sure old timing events are not generated anymore
        ES_Timer_StopTimer(RotateTimer);
        switch (ThisEvent.EventParam)
        {
          case 0x00:
          {
            PWMSetDuty(0, 0, ADVANCE);
            break;
          }
          case 0x02:
          {
            //PWMSetDuty(MAX_SPEED_A, MAX_SPEED_B, CW);
            PWMSetDuty(HALF_SPEED_A, HALF_SPEED_B, CW);
            ES_Timer_InitTimer(RotateTimer, QUARTER_TURN);
            break;
          }
          case 0x03:
          {
            //PWMSetDuty(MAX_SPEED_A, MAX_SPEED_B, CW);
            PWMSetDuty(HALF_SPEED_A, HALF_SPEED_B, CW);
            ES_Timer_InitTimer(RotateTimer, EIGHTH_TURN);
            break;
          }
          case 0x04:
          {
            //PWMSetDuty(MAX_SPEED_A, MAX_SPEED_B, CCW);
            PWMSetDuty(HALF_SPEED_A, HALF_SPEED_B, CCW);
            ES_Timer_InitTimer(RotateTimer, QUARTER_TURN);
            break;
          }
          case 0x05:
          {
            //PWMSetDuty(MAX_SPEED_A, MAX_SPEED_B, CCW);
            PWMSetDuty(HALF_SPEED_A, HALF_SPEED_B, CCW);
            ES_Timer_InitTimer(RotateTimer, EIGHTH_TURN);
            break;
          }
          case 0x08:
          {
            PWMSetDuty(HALF_SPEED_A, HALF_SPEED_B, ADVANCE);
            break;
          }
          case 0x09:
          {
            PWMSetDuty(MAX_SPEED_A, MAX_SPEED_B, ADVANCE);
            break;
          }
          case 0x10:
          {
            PWMSetDuty(HALF_SPEED_A, HALF_SPEED_B, REVERSE);
            break;
          }
          case 0x11:
          {
            PWMSetDuty(MAX_SPEED_A, MAX_SPEED_B, REVERSE);
            break;
          }
          case 0x20: //Align with beacon
          {
            //PWMSetDuty(MAX_SPEED_A, MAX_SPEED_B, CW);
            clearBeaconAlignedFlag();
            clearfirstRisingEdgeAlreadySeen();
            PWMSetDuty(MAX_SPEED_A, MAX_SPEED_B, CCW);
            CurrentState = AlignBeacon;
            //Align with beacon, go into new state which checks this
            break;
          }
          case 0x40:
          {
            PWMSetDuty(HALF_SPEED_A, HALF_SPEED_B, ADVANCE);
            CurrentState = FindBoundary;
            //Drive forward until tape is detected (go into new state which checks this)
            break;
          }
        }
      }
      else if ((ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == RotateTimer))
      {
        PWMSetDuty(0, 0, ADVANCE);
      }
      break;
    }

    case AlignBeacon:
    {
      if (ThisEvent.EventType == ES_BEACON_DETECTED)
      {
        PWMSetDuty(0, 0, ADVANCE);
        setBeaconAlignedFlag();
        CurrentState = RegularOperation;
      }
      break;
    }

    case FindBoundary:
    {
      if (ThisEvent.EventType == ES_TAPE_DETECTED)
      {
        PWMSetDuty(0, 0, ADVANCE);
        CurrentState = RegularOperation;
      }
      break;
    }
    */
  }

  //printf("\r\n Ran through MotorService\r\n");
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
