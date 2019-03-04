/****************************************************************************
 Module
   CollectingSM.c

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
#include "OrientingSM.h"

#include "CollectingSM.h"
#include "DriveCommandModule.h"

#include "IRDetector.h"
#include "GamePlayHSM.h"

#include <math.h> //for acos and atan
/*----------------------------- Module Defines ----------------------------*/
// define constants for the states for this machine
// and any other local defines

#define ENTRY_STATE Orienting
#define IR_FIRST_DELAY 100

#define REORIENTATION_DELAY 10000
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event_t DuringOrienting( ES_Event_t Event);
static ES_Event_t DuringRoaming(ES_Event_t Event);
static ES_Event_t DuringDriving2Target(ES_Event_t Event);



/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static CollectingState_t CurrentState;
static bool PositionAwareness = false;
static float XPosition;
static float YPosition;
static float Heading;

static float DriveDistance;
static bool RotatedFlag;
static uint8_t TargetPoint;
static float TargetDistance;

static float TargetPoints[4][2] = {
	{XTARGET,	YTARGET},
	{-XTARGET, YTARGET},
	{-XTARGET,	-YTARGET},
	{XTARGET,	-YTARGET}};
/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    RunCollectingSM

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
ES_Event_t RunCollectingSM( ES_Event_t CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   CollectingState_t NextState = CurrentState;
   ES_Event_t EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
   ES_Event_t ReturnEvent = CurrentEvent; // assume we are not consuming event

   switch ( CurrentState )
   {
       case Orienting :      
			 {
         ReturnEvent = CurrentEvent = DuringOrienting(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
              case EV_MOVE_COMPLETED:
              {
                //printf("EV_MOVE_COMPLETED in Orienting\r\n");
                //printf("PositionAwareness: %d \r\n", PositionAwareness);
                if (PositionAwareness == true)
                {
                  //printf("got here\r\n");
                  StopDrive();
                  // Execute action function for state one : event one
                  NextState = Driving2Target;//Decide what the next state will be
                  // for internal transitions, skip changing MakeTransition
                  MakeTransition = true; //mark that we are taking a transition
                  // if transitioning to a state with history change kind of entry
                  EntryEventKind.EventType = ES_ENTRY;
                  //printf("\r\n In here");
                  //ReturnEvent.EventType = ES_NO_EVENT;
                  DriveStraight(STRAIGHT_SPEED, 10000);
                  
                }
                else
                {
                  //printf("got there\r\n");
                  StopDrive();
                  ES_Timer_InitTimer(REORIENTATION_TIMER, REORIENTATION_DELAY);
                  ReturnEvent.EventType = ES_NO_EVENT;                  
                }
                //Current placeholder event
                PositionAwareness = false;
                
              }
							break;
              
              case ES_TIMEOUT:
              {
                if(CurrentEvent.EventParam == REORIENTATION_TIMER)
                {
                  StopDrive();
                  // Execute action function for state one : event one
                  NextState = Orienting;//Decide what the next state will be
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
    
    
		case Driving2Target:
		{
         ReturnEvent = CurrentEvent = DuringDriving2Target(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
              
              
              case EV_MOVE_COMPLETED:
              {
                if (RotatedFlag == true)
                {
                  StopDrive();
                  DriveStraight(STRAIGHT_SPEED, TargetDistance);
                  RotatedFlag = false;
                }
                else
                {
                  StopDrive();
                  // Execute action function for state one : event one
                  NextState = Roaming;//Decide what the next state will be
                  // for internal transitions, skip changing MakeTransition
                  MakeTransition = true; //mark that we are taking a transition
                  // if transitioning to a state with history change kind of entry
                  EntryEventKind.EventType = ES_ENTRY;                              
                  //Select new move to start up (Idea: Start from one point and go to others)
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
				
		case Roaming:
		{
         ReturnEvent = CurrentEvent = DuringRoaming(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
              case EV_MOVE_COMPLETED:
              {
                if(RotatedFlag == true)
                {
                  StopDrive();
                  DriveDistance = -fabsf(TargetPoints[TargetPoint+1][0]-TargetPoints[TargetPoint][0]);
                  DriveStraight(STRAIGHT_SPEED, DriveDistance);
                  RotatedFlag = false;
                  TargetPoint++;
                  if (TargetPoint>3)
                  {
                    TargetPoint=0;
                  }
                }
                else //if RotatedFlag == False
                {
                  StopDrive();
                  DriveRotate(TURNING_SPEED, QUARTER_TURN);
                  RotatedFlag = true;
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
       RunCollectingSM(CurrentEvent);

       CurrentState = NextState; //Modify state variable

       //   Execute entry function for new state
       // this defaults to ES_ENTRY
       RunCollectingSM(EntryEventKind);
     }
     return(ReturnEvent);
}
/****************************************************************************
 Function
     StartCollectingSM

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
void StartCollectingSM ( ES_Event_t CurrentEvent )
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
   RunCollectingSM(CurrentEvent);
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
CollectingState_t QueryCollectingSM(void)
{
   return CurrentState;
}

/***************************************************************************
 private functions
 ***************************************************************************/

static ES_Event_t DuringOrienting( ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
      // implement any entry actions required for this state machine

      ES_Timer_InitTimer(LOCALIZE_TIMER,IR_FIRST_DELAY);
      
      //Clear previous IR measurements that have been made
      ClearMeasurements();
      //Enable IR readings
      IREnableInterrupt();
      //Rotate 360 degrees
      DriveRotate(LOCALIZATION_SPEED, 3600);
      
      //Start any lower level machines that run in this state
      StartOrientingSM(Event);
        
        
    }
    else if ( Event.EventType == ES_EXIT )
    {
        // on exit, give the lower levels a chance to clean up first
        RunOrientingSM(Event);
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
        ReturnEvent = RunOrientingSM(Event);
        // repeat for any concurrent lower level machines
        
        // do any activity that is repeated as long as we are in this state
		}
    // return either Event, if you don't want to allow the lower level machine
    // to remap the current event, or ReturnEvent if you do want to allow it.
    return(ReturnEvent);
}
static ES_Event_t DuringDriving2Target(ES_Event_t Event)
{
   ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
      //Determine to which target point we are closest
      XPosition = GetCurrentXPosition();
      YPosition = GetCurrentYPosition();
      Heading = GetCurrentHeading();
      if(XPosition>=0)
      {
        if(YPosition>=0)
        {
          //Drive to TargetPoints, TargetPoints
          TargetDistance = sqrtf((XPosition - TargetPoints[0][0])*(XPosition - TargetPoints[0][0]) + (YPosition - TargetPoints[0][1])*(YPosition - TargetPoints[0][1]));
          //Find angle to rotate
        }
        else
        {
          //Drive to TargetPoints, -TargetPoints
          TargetDistance = sqrtf((XPosition - TargetPoints[1][0])*(XPosition - TargetPoints[1][0]) + (YPosition - TargetPoints[1][1])*(YPosition - TargetPoints[1][1]));
          //Find angle to rotate
        }
      }
      else
      {
        if(YPosition>=0)
        {
          //Drive to -TargetPoints, TargetPoints
          TargetDistance = sqrtf((XPosition - TargetPoints[2][0])*(XPosition - TargetPoints[2][0]) + (YPosition - TargetPoints[2][1])*(YPosition - TargetPoints[2][1]));
          //Find angle to rotate
        }
        else
        {
          //Drive to -TargetPoints, -TargetPoints
          TargetDistance = sqrtf((XPosition - TargetPoints[3][0])*(XPosition - TargetPoints[3][0]) + (YPosition - TargetPoints[3][1])*(YPosition - TargetPoints[3][1]));
          //Find angle to rotate
        }
      }
      
      //Set rotation equal to angle that we need to rotate
      DriveRotate(TURNING_SPEED, PLACEHOLDER_ANGLE);
          
        
			
				
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

static ES_Event_t DuringRoaming( ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
      SetBotDirection(BACKWARDS);
      //Based on the target we drove and our heading towards it, determine by what angle to turn
      //DriveRotate(TURN_SPEED, Angle);
        // implement any entry actions required for this state machine
        
			
				
        // after that start any lower level machines that run in this state
        //StartLowerLevelSM( Event );
        // repeat the StartxxxSM() functions for concurrent state machines
        // on the lower level
    }
    else if ( Event.EventType == ES_EXIT )
    {
      
      StopDrive();
      SetBotDirection(FORWARDS);
      
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

void SetPositionAwareness(bool PositionStatus)
{
  PositionAwareness = PositionStatus;
}

bool QueryPositionAwareness(void)
{
  return PositionAwareness;
}




