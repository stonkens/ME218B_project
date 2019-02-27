
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

#include "DriveCommandModule.h"
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
	if (ThisEvent.EventType == ES_NEW_KEY)
	{
		printf("ES_NEW_KEY received with -> %c <- in Service 0\r\n",
          (char)ThisEvent.EventParam);
		if('a' == ThisEvent.EventParam)
		{
			printf("Commanded Motor to drive forward 1 feet\r\n");
			Drive_Straight(1200);
		}
		else if('s' == ThisEvent.EventParam)
		{
			printf("Commanded Motor to drive backward 1 feet \r\n");
			Drive_Straight(-1200);
		}
		else if('d' == ThisEvent.EventParam)
		{
			printf("Commanded Motor to turn 90 degrees \r\n");
			Drive_Turn(900);
		}
		else if('f' == ThisEvent.EventParam)
		{
			printf("Command Motor to turn -90 degrees \r\n");
			Drive_Turn(-900);
		}
		else if('q' == ThisEvent.EventParam)
		{
			printf("Stop MOTOR from moving");
		}
	}
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
