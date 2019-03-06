/****************************************************************************
 Module
   CollectingV2SM.c

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

#include "CollectingV2SM.h"
#include "DriveCommandModule.h"

#include "IRDetector.h"
#include "GamePlayHSM.h"
#include "MasterHSM.h"
#include "TapeFollowingChecker.h"

#include <math.h> //for acos and atan
/*----------------------------- Module Defines ----------------------------*/
// define constants for the states for this machine
// and any other local defines

#define ENTRY_STATE StraightDrive
#define IR_FIRST_DELAY 100

#define COLLECTSTOP_TIME 3000
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event_t DuringAlign2Landfill (ES_Event_t Event);
static ES_Event_t DuringPrepare4Harvesting( ES_Event_t Event);
static ES_Event_t DuringFindingTape (ES_Event_t Event);
static ES_Event_t DuringStraightDrive( ES_Event_t Event);
static ES_Event_t DuringTurnDrive(ES_Event_t Event);




/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static CollectingState_t CurrentState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    RunCollectingV2SM

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
ES_Event_t RunCollectingV2SM( ES_Event_t CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   CollectingState_t NextState = CurrentState;
   ES_Event_t EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
   ES_Event_t ReturnEvent = CurrentEvent; // assume we are not consuming event

   switch ( CurrentState )
   {
       case Align2Landfill :      
			 {
         ReturnEvent = CurrentEvent = DuringAlign2Landfill(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
              case EV_ALIGNED2BEACON:
              {

                  //printf("got there\r\n");
                  StopDrive();
                IRDisableInterrupt();
                  ES_Timer_InitTimer(COLLECTSTOP_TIMER, COLLECTSTOP_TIME);
                  ReturnEvent.EventType = ES_NO_EVENT;                  
                  //Current placeholder event
                
              }
							break;
              
              case ES_TIMEOUT:
              {
                if(CurrentEvent.EventParam == COLLECTSTOP_TIMER)
                {
                  StopDrive();
                  // Execute action function for state one : event one
                  NextState = Prepare4Harvesting;//Decide what the next state will be
                  // for internal transitions, skip changing MakeTransition
                  MakeTransition = true; //mark that we are taking a transition
                  // if transitioning to a state with history change kind of entry
                  EntryEventKind.EventType = ES_ENTRY;
                  //printf("\r\n In here");
                  ReturnEvent.EventType = ES_NO_EVENT;                 
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
       case Prepare4Harvesting :      
			 {
         ReturnEvent = CurrentEvent = DuringPrepare4Harvesting(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
              case EV_MOVE_COMPLETED:
              {

                  //printf("got there\r\n");
                  StopDrive();
                  ES_Timer_InitTimer(COLLECTSTOP_TIMER, COLLECTSTOP_TIME);
                  ReturnEvent.EventType = ES_NO_EVENT;                  
              }
							break;
              
              case ES_TIMEOUT:
              {
                if(CurrentEvent.EventParam == COLLECTSTOP_TIMER)
                {
                  StopDrive();
                  // Execute action function for state one : event one
                  NextState = StraightDrive;//Decide what the next state will be
                  // for internal transitions, skip changing MakeTransition
                  MakeTransition = true; //mark that we are taking a transition
                  // if transitioning to a state with history change kind of entry
                  EntryEventKind.EventType = ES_ENTRY;
                  //printf("\r\n In here");
                  ReturnEvent.EventType = ES_NO_EVENT;                 
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
/*       case FindingTape :      
			 {
         ReturnEvent = CurrentEvent = DuringFindingTape(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
              case EV_EQUATOR_DETECTED:
              {

                  //printf("got there\r\n");
                  StopDrive();
                  ES_Timer_InitTimer(COLLECTSTOP_TIMER, COLLECTSTOP_TIME);
                  ReturnEvent.EventType = ES_NO_EVENT;                  
              }
							break;

              
              case ES_TIMEOUT:
              {
                if(CurrentEvent.EventParam == COLLECTSTOP_TIMER)
                {
                  StopDrive();
                  // Execute action function for state one : event one
                  NextState = StraightDrive;//Decide what the next state will be
                  // for internal transitions, skip changing MakeTransition
                  MakeTransition = true; //mark that we are taking a transition
                  // if transitioning to a state with history change kind of entry
                  EntryEventKind.EventType = ES_ENTRY;
                  //printf("\r\n In here");
                  ReturnEvent.EventType = ES_NO_EVENT;                 
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
*/    
    

   case StraightDrive :      
   {
     ReturnEvent = CurrentEvent = DuringStraightDrive(CurrentEvent);
     //process any events
     if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
     {
        switch (CurrentEvent.EventType)
        {
          case EV_MOVE_COMPLETED:
          {

              //printf("got there\r\n");
              StopDrive();
              ES_Timer_InitTimer(COLLECTSTOP_TIMER, COLLECTSTOP_TIME);
              ReturnEvent.EventType = ES_NO_EVENT;                  
            
          }
          break;
          
          case ES_TIMEOUT:
          {
            if(CurrentEvent.EventParam == COLLECTSTOP_TIMER)
            {
              StopDrive();
              // Execute action function for state one : event one
              NextState = TurnDrive;//Decide what the next state will be
              // for internal transitions, skip changing MakeTransition
              MakeTransition = true; //mark that we are taking a transition
              // if transitioning to a state with history change kind of entry
              EntryEventKind.EventType = ES_ENTRY;
              //printf("\r\n In here");
              ReturnEvent.EventType = ES_NO_EVENT;                 
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
    
    
		case TurnDrive:
		{
         ReturnEvent = CurrentEvent = DuringTurnDrive(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
              
             case EV_MOVE_COMPLETED:
              {

                  //printf("got there\r\n");
                  StopDrive();
                  ES_Timer_InitTimer(COLLECTSTOP_TIMER, COLLECTSTOP_TIME);
                  ReturnEvent.EventType = ES_NO_EVENT;                  
                
              }
							break;
              
              case ES_TIMEOUT:
              {
                if(CurrentEvent.EventParam == COLLECTSTOP_TIMER)
                {
                  StopDrive();
                  // Execute action function for state one : event one
                  NextState = StraightDrive;//Decide what the next state will be
                  // for internal transitions, skip changing MakeTransition
                  MakeTransition = true; //mark that we are taking a transition
                  // if transitioning to a state with history change kind of entry
                  EntryEventKind.EventType = ES_ENTRY;
                  //printf("\r\n In here");
                  ReturnEvent.EventType = ES_NO_EVENT;                 
                }
              }
              break;
							 
              default:
              {;
              }
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
       RunCollectingV2SM(CurrentEvent);

       CurrentState = NextState; //Modify state variable

       //   Execute entry function for new state
       // this defaults to ES_ENTRY
       RunCollectingV2SM(EntryEventKind);
     }
     return(ReturnEvent);
}
/****************************************************************************
 Function
     StartCollectingV2SM

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
void StartCollectingV2SM ( ES_Event_t CurrentEvent )
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
   RunCollectingV2SM(CurrentEvent);
}

/****************************************************************************
 Function
     QueryTemplateSM

 Parameters
     None

 Returns
     CollectingState_t The current state of the Template state machine

 Description
     returns the current state of the Template state machine
 Notes

 Author
     J. Edward Carryer, 2/11/05, 10:38AM
****************************************************************************/
CollectingState_t QueryCollectingV2SM(void)
{
   return CurrentState;
}

/***************************************************************************
 private functions
 ***************************************************************************/
static ES_Event_t DuringAlign2Landfill( ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {

      if(QueryTeam() == TEAM_NORTH)
      {
        ActivateBeaconFinder(NORTH_LANDFILL_PERIOD);
      }
      else
      {
        ActivateBeaconFinder(SOUTH_LANDFILL_PERIOD);
      }
      IREnableInterrupt();
      //Start any lower level machines that run in this state
      SetBotDirection(FORWARDS);
      DriveRotate(TURNING_SPEED, 3600);
      
      //Potentially testing
      //DriveStraight(STRAIGHT_SPEED, 4800);
        
        
    }
    else if ( Event.EventType == ES_EXIT )
    {
        // on exit, give the lower levels a chance to clean up first
        StopDrive();
        IRDisableInterrupt();
        //RunLowerLevelSM(Event);
        // repeat for any concurrently running state machines
        // now do any local exit functionality
			
			// Stop turning DC Motors connected to pin PB6 & PB7
			
      
    }else
    // do the 'during' function for this state
    {
        // run any lower level state machine
        // ReturnEvent = RunLowerLevelSM(Event);
        // repeat for any concurrent lower level machines
        
        // do any activity that is repeated as long as we are in this state
		}
    // return either Event, if you don't want to allow the lower level machine
    // to remap the current event, or ReturnEvent if you do want to allow it.
    return(ReturnEvent);
}

static ES_Event_t DuringPrepare4Harvesting( ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {

      DriveRotate(TURNING_SPEED, 900); 
      //Start any lower level machines that run in this state
      
        
    }
    else if ( Event.EventType == ES_EXIT )
    {
        // on exit, give the lower levels a chance to clean up first
        StopDrive();
        SetBotDirection(BACKWARDS);
        //RunLowerLevelSM(Event);
        // repeat for any concurrently running state machines
        // now do any local exit functionality
			
			// Stop turning DC Motors connected to pin PB6 & PB7
			
      
    }else
    // do the 'during' function for this state
    {
        // run any lower level state machine
        // ReturnEvent = RunLowerLevelSM(Event);
        // repeat for any concurrent lower level machines
        
        // do any activity that is repeated as long as we are in this state
		}
    // return either Event, if you don't want to allow the lower level machine
    // to remap the current event, or ReturnEvent if you do want to allow it.
    return(ReturnEvent);
}


/*static ES_Event_t DuringFindingTape( ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {

      DriveStraight(STRAIGHT_SPEED, -4800); 
      //Start any lower level machines that run in this state
      enableTapeFollow();
        
        
    }
    else if ( Event.EventType == ES_EXIT )
    {
        // on exit, give the lower levels a chance to clean up first
      StopDrive();
      disableTapeFollow();
        //RunLowerLevelSM(Event);
        // repeat for any concurrently running state machines
        // now do any local exit functionality
			
			// Stop turning DC Motors connected to pin PB6 & PB7
			
      
    }else
    // do the 'during' function for this state
    {
        // run any lower level state machine
        // ReturnEvent = RunLowerLevelSM(Event);
        // repeat for any concurrent lower level machines
        
        // do any activity that is repeated as long as we are in this state
		}
    // return either Event, if you don't want to allow the lower level machine
    // to remap the current event, or ReturnEvent if you do want to allow it.
    return(ReturnEvent);
}
*/

static ES_Event_t DuringStraightDrive( ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {

      DriveStraight(STRAIGHT_SPEED,-8000); 
      //Start any lower level machines that run in this state

        
        
    }
    else if ( Event.EventType == ES_EXIT )
    {
        // on exit, give the lower levels a chance to clean up first
        StopDrive();
        //RunLowerLevelSM(Event);
        // repeat for any concurrently running state machines
        // now do any local exit functionality
			
			// Stop turning DC Motors connected to pin PB6 & PB7
			
      
    }else
    // do the 'during' function for this state
    {
        // run any lower level state machine
        // ReturnEvent = RunLowerLevelSM(Event);
        // repeat for any concurrent lower level machines
        
        // do any activity that is repeated as long as we are in this state
		}
    // return either Event, if you don't want to allow the lower level machine
    // to remap the current event, or ReturnEvent if you do want to allow it.
    return(ReturnEvent);
}
static ES_Event_t DuringTurnDrive(ES_Event_t Event)
{
   ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
      DriveRotate(TURNING_SPEED, 900); 
          
        
			
				
        // after that start any lower level machines that run in this state
        //StartLowerLevelSM( Event );
        // repeat the StartxxxSM() functions for concurrent state machines
        // on the lower level
    }
    else if ( Event.EventType == ES_EXIT )
    {
      StopDrive();
        // on exit, give the lower levels a chance to clean up first
        //RunLowerLevelSM(Event);
        // repeat for any concurrently running state machines
        // now do any local exit functionality
			
      
    }else
    // do the 'during' function for this state
    {
        // run any lower level state machine
        // ReturnEvent = RunLowerLevelSM(Event);
      
        // repeat for any concurrent lower level machines
      
        // do any activity that is repeated as long as we are in this state
    }
    // return either Event, if you don't want to allow the lower level machine
    // to remap the current event, or ReturnEvent if you do want to allow it.
    return(ReturnEvent);
}  





