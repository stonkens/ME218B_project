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



#include "GP_Display.h"
#include "BallProcessingSM.h"
#include "MasterHSM.h"
//#include "CollectingSM.h" Triangulation method
#include "CollectingV2SM.h"

/*----------------------------- Module Defines ----------------------------*/
#define DISPLAY_UPDATE_TIME 100
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;


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
  ES_Timer_InitTimer(DISPLAY_TIMER, DISPLAY_UPDATE_TIME);
  
  return true;
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

    if (ES_NEW_KEY == ThisEvent.EventType) // there was a key pressed
    {
        switch ( toupper(ThisEvent.EventParam))
        {
            case 'S' : 
            {
              ThisEvent.EventType = EV_COMPASS_CLEANING_UP;
            }
            break;
            case 'C' : 
            {
              ThisEvent.EventType = EV_COMPASS_RECYCLE_CHANGE; 
            }
            break;
            case 'G' :
            {
              ThisEvent.EventType = EV_COMPASS_GAME_OVER;
            }
            break;
            case 'B' :
            {
              ThisEvent.EventType = EV_BUMPER_HIT;
              ThisEvent.EventParam = 2;
            }
            break;
            case 'N' :
            {
              ThisEvent.EventType = EV_BUMPER_HIT;
              ThisEvent.EventParam = 3;
            }
            break;
            case 'M' :
            {
              ThisEvent.EventType = EV_BUMPER_HIT;
              ThisEvent.EventParam = 4;
            }
            break;
            case 'V' :
            { 
              ThisEvent.EventType = EV_BUMPER_HIT;
              ThisEvent.EventParam = 1;
            }
            break;
            case 'D' :
            {
              clearRecycleBalls();
              ThisEvent.EventType = EV_RECYCLING_DONE;
            }
            break;
            case 'L' :
            {
              clearLandfillBalls();
              ThisEvent.EventType = EV_LANDFILLING_DONE;
            }
            break;
            case 'Y' :
            {
              ThisEvent.EventType = EV_MOVED_BACK;
            }
            break;
            case 'P' :
            {
              ThisEvent.EventType = EV_MOVE_COMPLETED;
            }
            break;
            case 'T' :
            {
              ThisEvent.EventType = EV_TAPE_DETECTED;
            }
            break;
            case 'Z':
            {
              ThisEvent.EventType = EV_ALIGNED2BEACON;
            }
            break;
            case '1':
            {
              AddRecycleBalls();
            }
            break;
            case '0':
            {
              AddLandFillBalls();
            }
            break;
            case 'X' :
            {
              SetPositionAwareness(true);
            }
            break;
            
              
        }
        PostMasterSM(ThisEvent);
    }
    
    if ((ES_TIMEOUT == ThisEvent.EventType) && (ThisEvent.EventParam == DISPLAY_TIMER))
    {
      UpdateDisplay();
      ES_Timer_InitTimer(DISPLAY_TIMER, DISPLAY_UPDATE_TIME);
      
    }
  return ReturnEvent;
}


/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

