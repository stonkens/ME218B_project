/****************************************************************************
 Module
   KeyMapperService.c

 Revision
   1.0.1

 Description
   This module implements a simple debugging tool that translates key strokes
   into Events and posts them to the relevant services.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 10/18/15 17:28 jec      began conversion from TemplateFSM.c
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "KeyMapperService.h"
#include "I2CService.h"

// to get toupper()
#include <ctype.h>

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static uint8_t RecycleBalls = 0;
static uint8_t LandFillBalls = 0;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitKeyMapperService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

 Author
     J. Edward Carryer, 10/18/15, 17:30
****************************************************************************/
bool InitKeyMapperService ( uint8_t Priority )
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  /********************************************
   in here you write your initialization code
   *******************************************/
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService( MyPriority, ThisEvent) == true)
  {
      return true;
  }else
  {
      return false;
  }
}

/****************************************************************************
 Function
     PostKeyMapperService

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/18/15, 17:31
****************************************************************************/
bool PostKeyMapperService( ES_Event_t ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunKeyMapperService

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   responds to NewKey events by creating & poosting events
 Notes
   
 Author
   J. Edward Carryer, 10/18/15, 17:32
****************************************************************************/
ES_Event_t RunKeyMapperService( ES_Event_t ThisEvent )
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    if ( ThisEvent.EventType == ES_NEW_KEY) // there was a key pressed
    {
        switch ( toupper(ThisEvent.EventParam))
        {
            case '.' : ThisEvent.EventType = EV_I2C_ReadClear;  break;
            case ',' : ThisEvent.EventType = EV_I2C_StepFinished; break;
        }
        PostI2CService(ThisEvent);
    }
    
  return ReturnEvent;
}

uint8_t QueryRecycleBalls(void)
{
  return RecycleBalls;
}

uint8_t QueryLandFillBalls(void)
{
  return LandFillBalls;
}

/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

