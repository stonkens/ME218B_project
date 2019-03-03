/****************************************************************************
 Module
   GamePlaySM.c

 Revision
   2.0.1

 Description
   This is a template file for implementing state machines.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/27/17 09:48 jec      another correction to re-assign both CurrentEvent
                         and ReturnEvent to the result of the During function
                         this eliminates the need for the prior fix and allows
                         the during function to-remap an event that will be
                         processed at a higher level.
 02/20/17 10:14 jec      correction to Run function to correctly assign 
                         ReturnEvent in the situation where a lower level
                         machine consumed an event.
 02/03/16 12:38 jec      updated comments to reflect changes made in '14 & '15
                         converted unsigned char to bool where appropriate
                         spelling changes on true (was True) to match standard
                         removed local var used for debugger visibility in 'C32
                         commented out references to Start & RunLowerLevelSM so
                         that this can compile. 
 02/07/13 21:00 jec      corrections to return variable (should have been
                         ReturnEvent, not CurrentEvent) and several EV_xxx
                         event names that were left over from the old version
 02/08/12 09:56 jec      revisions for the Events and Services Framework Gen2
 02/13/10 14:29 jec      revised Start and run to add new kind of entry function
                         to make implementing history entry cleaner
 02/13/10 12:29 jec      added NewEvent local variable to During function and
                         comments about using either it or Event as the return
 02/11/10 15:54 jec      more revised comments, removing last comment in during
                         function that belongs in the run function
 02/09/10 17:21 jec      updated comments about internal transitions on During funtion
 02/18/09 10:14 jec      removed redundant call to RunLowerlevelSM in EV_Entry
                         processing in During function
 02/20/07 21:37 jec      converted to use enumerated type for events & states
 02/13/05 19:38 jec      added support for self-transitions, reworked
                         to eliminate repeated transition code
 02/11/05 16:54 jec      converted to implment hierarchy explicitly
 02/25/03 10:32 jec      converted to take a passed event parameter
 02/18/99 10:19 jec      built template from MasterMachine.c
 02/14/99 10:34 jec      Began Coding
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
// Basic includes for a program using the Events and Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"

/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "GamePlayHSM.h"

#include "CollectingSM.h"
#include "RecyclingSM.h"
#include "LandfillingSM.h"
#include "KeyMapperService.h"

#include "DriveCommandModule.h"

/*----------------------------- Module Defines ----------------------------*/
// define constants for the states for this machine
// and any other local defines

#define ENTRY_STATE CollectingGarbage

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event_t DuringCollectingGarbage( ES_Event_t Event);
static ES_Event_t DuringRecycling(ES_Event_t Event);
static ES_Event_t DuringLandfilling( ES_Event_t Event);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static GamePlayState_t CurrentState;

static bool BotDirection=FORWARDS;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    RunGamePlaySM

 Parameters
   ES_Event_t: the event to process

 Returns
   ES_Event_t: an event to return

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   J. Edward Carryer, 2/11/05, 10:45AM
****************************************************************************/
ES_Event_t RunGamePlaySM( ES_Event_t CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   GamePlayState_t NextState = CurrentState;
   ES_Event_t EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
   ES_Event_t ReturnEvent = CurrentEvent; // assume we are not consuming event

   switch ( CurrentState )
   {
       case CollectingGarbage :      
			 {
         ReturnEvent = CurrentEvent = DuringCollectingGarbage(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
							 
             case ES_TIMEOUT :
             {
               if (CurrentEvent.EventParam == BALL_COLLECTION_TIMER)
               {
                  if (QueryRecycleBalls() != 0)
                  {
                    // Execute action function for state one : event one
                    NextState = Recycling;//Decide what the next state will be
                    // for internal transitions, skip changing MakeTransition
                    MakeTransition = true; //mark that we are taking a transition
                    // if transitioning to a state with history change kind of entry
                    EntryEventKind.EventType = ES_ENTRY;
                    // optionally, consume or re-map this event for the upper
                    // level state machine
                    ReturnEvent.EventType = ES_NO_EVENT;
                  }
                  
                  else if(QueryLandFillBalls() != 0)
                  {
                    // Execute action function for state one : event one
                    NextState = Landfilling;//Decide what the next state will be
                    // for internal transitions, skip changing MakeTransition
                    MakeTransition = true; //mark that we are taking a transition
                    // if transitioning to a state with history change kind of entry
                    EntryEventKind.EventType = ES_ENTRY;
                    // optionally, consume or re-map this event for the upper
                    // level state machine
                    ReturnEvent.EventType = ES_NO_EVENT;
                  }
                  else
                  {
                    //This time we only allow for BALL_COLLECTION_TIME/2 seconds to query again
                    ES_Timer_InitTimer(BALL_COLLECTION_TIMER, BALL_COLLECTION_TIME/2);
                  }
                 
								 }
							 }
							 break;
							 
               
							 default:
							 {;
							 }
                // repeat cases as required for relevant events
            }

         }
       
      // repeat state pattern as required for other states
    }
		break;
		
		case Recycling:
		{
         ReturnEvent = CurrentEvent = DuringRecycling(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               
               case EV_RECYCLING_DONE :
               {
                  if(QueryLandFillBalls() != 0)
                  {
                    NextState = Landfilling;
                    // for internal transitions, skip changing MakeTransition
                    MakeTransition = true; //mark that we are taking a transition
                    // if transitioning to a state with history change kind of entry
                    EntryEventKind.EventType = ES_ENTRY;
                    // optionally, consume or re-map this event for the upper
                    // level state machine
                    ReturnEvent.EventType = ES_NO_EVENT;
                  }
                  
                  else
                  {

                    NextState = CollectingGarbage;
                    // for internal transitions, skip changing MakeTransition
                    MakeTransition = true; //mark that we are taking a transition
                    // if transitioning to a state with history change kind of entry
                    EntryEventKind.EventType = ES_ENTRY;
                    // optionally, consume or re-map this event for the upper
                    // level state machine
                    ReturnEvent.EventType = ES_NO_EVENT;	
                    //Restart Timer to check for balls
                    ES_Timer_InitTimer(BALL_COLLECTION_TIMER, BALL_COLLECTION_TIME);
                  }
               }
               break;
               
                
							 default:
               {;
							 }
                // repeat cases as required for relevant events
            }

         }
       
      // repeat state pattern as required for other states
		}
		break;
		
		case Landfilling:
		{
         ReturnEvent = CurrentEvent = DuringLandfilling(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
							 
               case EV_LANDFILLING_DONE:
               {
                  if(QueryRecycleBalls() != 0)
                  {
                    NextState = Recycling;
                    // for internal transitions, skip changing MakeTransition
                    MakeTransition = true; //mark that we are taking a transition
                    // if transitioning to a state with history change kind of entry
                    EntryEventKind.EventType = ES_ENTRY;
                    // optionally, consume or re-map this event for the upper
                    // level state machine
                    ReturnEvent.EventType = ES_NO_EVENT;
                    
                  }
                  
                  else
                  {

                    
                    NextState = CollectingGarbage;
                    // for internal transitions, skip changing MakeTransition
                    MakeTransition = true; //mark that we are taking a transition
                    // if transitioning to a state with history change kind of entry
                    EntryEventKind.EventType = ES_ENTRY;
                    // optionally, consume or re-map this event for the upper
                    // level state machine
                    ReturnEvent.EventType = ES_NO_EVENT;	
                    //Restart Timer to check for balls
                    ES_Timer_InitTimer(BALL_COLLECTION_TIMER, BALL_COLLECTION_TIME);
                  }
               }
               break;
							 
							 
							 default:
							{;
							}
                // repeat cases as required for relevant events
            }

         }
       
      // repeat state pattern as required for other states
		}
		break;
		default:
		{;
		}
	}
    //   If we are making a state transition
    if (MakeTransition == true)
    {
       //   Execute exit function for current state
       CurrentEvent.EventType = ES_EXIT;
       RunGamePlaySM(CurrentEvent);

       CurrentState = NextState; //Modify state variable

       //   Execute entry function for new state
       // this defaults to ES_ENTRY
       RunGamePlaySM(EntryEventKind);
     }
     return(ReturnEvent);
}
/****************************************************************************
 Function
     StartGamePlaySM

 Parameters
     None

 Returns
     None

 Description
     Does any required initialization for this state machine
 Notes

 Author
     J. Edward Carryer, 2/18/99, 10:38AM
****************************************************************************/
void StartGamePlaySM ( ES_Event_t CurrentEvent )
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
   RunGamePlaySM(CurrentEvent);
}

/****************************************************************************
 Function
     QueryTemplateSM

 Parameters
     None

 Returns
     GamePlayState_t The current state of the Template state machine

 Description
     returns the current state of the Template state machine
 Notes

 Author
     J. Edward Carryer, 2/11/05, 10:38AM
****************************************************************************/
GamePlayState_t QueryGamePlaySM(void)
{
   return CurrentState;
}

/***************************************************************************
 private functions
 ***************************************************************************/

static ES_Event_t DuringCollectingGarbage( ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
      StartCollectingSM(Event);
        // implement any entry actions required for this state machine
        
			  // Start turning DC Motors connected to pin PB6 & PB7
	
							
				// StartCollectingGarbageSM(Event);
			
				
        // after that start any lower level machines that run in this state
        //StartLowerLevelSM( Event );
        // repeat the StartxxxSM() functions for concurrent state machines
        // on the lower level
    }
    else if ( Event.EventType == ES_EXIT )
    {
      RunCollectingSM(Event);
      StopDrive();
        // on exit, give the lower levels a chance to clean up first
        //RunLowerLevelSM(Event);
        // repeat for any concurrently running state machines
        // now do any local exit functionality
			
			// Stop turning DC Motors connected to pin PB6 & PB7
			
      
    }else
    // do the 'during' function for this state
    {
      ReturnEvent = RunCollectingSM(Event);
        // run any lower level state machine
        // ReturnEvent = RunLowerLevelSM(Event);
      
        // repeat for any concurrent lower level machines
      
        // do any activity that is repeated as long as we are in this state
		}
    // return either Event, if you don't want to allow the lower level machine
    // to remap the current event, or ReturnEvent if you do want to allow it.
    return(ReturnEvent);
}


static ES_Event_t DuringRecycling( ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
      StartRecyclingSM(Event);
        // implement any entry actions required for this state machine
        
			
				
        // after that start any lower level machines that run in this state
        //StartLowerLevelSM( Event );
        // repeat the StartxxxSM() functions for concurrent state machines
        // on the lower level
    }
    else if ( Event.EventType == ES_EXIT )
    {
      RunRecyclingSM(Event);
      StopDrive();
      
        // on exit, give the lower levels a chance to clean up first
        //RunLowerLevelSM(Event);
        // repeat for any concurrently running state machines
        // now do any local exit functionality
			
      
    }else
    // do the 'during' function for this state
    {
      ReturnEvent = RunRecyclingSM(Event);
        // run any lower level state machine
        // ReturnEvent = RunLowerLevelSM(Event);
      
        // repeat for any concurrent lower level machines
      
        // do any activity that is repeated as long as we are in this state
    }
    // return either Event, if you don't want to allow the lower level machine
    // to remap the current event, or ReturnEvent if you do want to allow it.
    return(ReturnEvent);
}

static ES_Event_t DuringLandfilling( ES_Event_t Event)
{
   ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
      StartLandfillingSM(Event);
        // implement any entry actions required for this state machine
        
			
				
        // after that start any lower level machines that run in this state
        //StartLowerLevelSM( Event );
        // repeat the StartxxxSM() functions for concurrent state machines
        // on the lower level
    }
    else if ( Event.EventType == ES_EXIT )
    {
      StopDrive();
        // on exit, give the lower levels a chance to clean up first
      RunLandfillingSM(Event);
        // repeat for any concurrently running state machines
        // now do any local exit functionality
			
      
    }else
    // do the 'during' function for this state
    {
      ReturnEvent = RunLandfillingSM(Event);
        // run any lower level state machine
        // ReturnEvent = RunLowerLevelSM(Event);
      
        // repeat for any concurrent lower level machines
      
        // do any activity that is repeated as long as we are in this state
    }
    // return either Event, if you don't want to allow the lower level machine
    // to remap the current event, or ReturnEvent if you do want to allow it.
    return(ReturnEvent);
}

bool QueryBotDirection(void)
{
  return BotDirection; 
}

void SetBotDirection(bool Direction)
{
  if(Direction == FORWARDS)
  {
    BotDirection = FORWARDS;
  }
  else
  {
    BotDirection = BACKWARDS;
  }
}
