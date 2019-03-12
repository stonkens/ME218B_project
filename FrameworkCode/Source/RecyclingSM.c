/****************************************************************************
 Module
   RecyclingSM.c

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
#include "RecyclingSM.h"
#include "TapeFollowingChecker.h"

//Enabling it to drive
#include "DriveCommandModule.h"
#include "SPISM.h"
#include "IRDetector.h"
#include "IREmitter.h"
#include "ReflectiveTapeChecker.h"
#include "MasterHSM.h"
#include "BallDumpingSM.h"
/*----------------------------- Module Defines ----------------------------*/
// define constants for the states for this machine
// and any other local defines

#define ENTRY_STATE Orienting2LandfillR
#define KISS_THRESHOLD 3
#define COLLECTSTOP_TIME 1000
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event_t DuringOrienting2Recycle( ES_Event_t Event);
static ES_Event_t DuringDriving2Recycle(ES_Event_t Event);
static ES_Event_t DuringPreparing4Recycle (ES_Event_t Event);
static ES_Event_t DuringDumpingRecycle (ES_Event_t Event);
static ES_Event_t DuringRecoveringFromRecycle (ES_Event_t Event);
static ES_Event_t DuringOrienting2Landfill (ES_Event_t Event); 
static ES_Event_t DuringDriving2Landfill (ES_Event_t Event);
static ES_Event_t DuringMovingBodyRotation ( ES_Event_t Event);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static RecyclingState_t CurrentState;
static uint8_t NumberofKisses;
static bool Align2North;
static uint16_t OurRecyclePeriod;
static uint16_t OppRecyclePeriod;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    RunRecyclingSM

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
ES_Event_t RunRecyclingSM( ES_Event_t CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   RecyclingState_t NextState = CurrentState;
   ES_Event_t EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
   ES_Event_t ReturnEvent = CurrentEvent; // assume we are not consuming event

   switch ( CurrentState )
   {
     case Orienting2LandfillR:
     {
       ReturnEvent = CurrentEvent = DuringOrienting2Landfill(CurrentEvent);
       if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
              case EV_ALIGNED2BEACON:
              {
                StopDrive();
                IRDisableInterrupt();
                ES_Timer_InitTimer(COLLECTSTOP_TIMER, COLLECTSTOP_TIMER);
                ReturnEvent.EventType = ES_NO_EVENT;
              }
              break;
              
              case ES_TIMEOUT:
              {
                if(CurrentEvent.EventParam == COLLECTSTOP_TIMER)
                {
                //Stop driving motors
                StopDrive();
                // Execute action function for state one : event one
                NextState = Driving2LandfillR;//Decide what the next state will be
                // for internal transitions, skip changing MakeTransition
                MakeTransition = true; //mark that we are taking a transition
                // if transitioning to a state with history change kind of entry
                EntryEventKind.EventType = ES_ENTRY;                              
                //Select new move to start up (Idea: Start from one point and go to others)
                ReturnEvent.EventType = ES_NO_EVENT;
                }
              }
              break;
              
              case EV_MOVE_COMPLETED: //Couldn't find the beacon
              {
                NextState = CurrentState;//Decide what the next state will be
                // for internal transitions, skip changing MakeTransition
                MakeTransition = true; //mark that we are taking a transition
                // if transitioning to a state with history change kind of entry
                EntryEventKind.EventType = ES_ENTRY;                              
                //Select new move to start up (Idea: Start from one point and go to others)
                ReturnEvent.EventType = ES_NO_EVENT;
              }
              break;
                
              
              default:
              {;
              }
            }
          }
                
     }
     break;
      
     case Driving2LandfillR:
     {
       ReturnEvent = CurrentEvent = DuringDriving2Landfill(CurrentEvent);
       if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
              case EV_EQUATOR_DETECTED:
              {
                StopDrive();
                //disableTapeFollow();
                disableEquatorDetection();
                // Execute action function for state one : event one
                NextState = MovingBodyRotation;//Decide what the next state will be
                // for internal transitions, skip changing MakeTransition
                MakeTransition = true; //mark that we are taking a transition
                // if transitioning to a state with history change kind of entry
                EntryEventKind.EventType = ES_ENTRY;                              
                //Select new move to start up (Idea: Start from one point and go to others)
                ReturnEvent.EventType = ES_NO_EVENT;
              }
              break;              


              
              case EV_BUMPER_HIT:
              {
                if ((CurrentEvent.EventParam == 1) || (CurrentEvent.EventParam == 2))
                {
                  //Stop driving motors
                  StopDrive();
                  // Execute action function for state one : event one
                  NextState = Orienting2LandfillR;//Decide what the next state will be
                  // for internal transitions, skip changing MakeTransition
                  MakeTransition = true; //mark that we are taking a transition
                  // if transitioning to a state with history change kind of entry
                  EntryEventKind.EventType = ES_NO_EVENT;                              
                  //Select new move to start up (Idea: Start from one point and go to others)
                  ReturnEvent = CurrentEvent;
                }
              } 
              break;
              
              default:
              {;
              }
            }
          }
              
     }
     break;
     
     
      case MovingBodyRotation:
      {
          ReturnEvent = CurrentEvent = DuringMovingBodyRotation(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            { 
              case EV_MOVE_COMPLETED: 
              {
                NextState = Orienting2Recycle;//Decide what the next state will be
                // for internal transitions, skip changing MakeTransition
                MakeTransition = true; //mark that we are taking a transition
                // if transitioning to a state with history change kind of entry
                EntryEventKind.EventType = ES_ENTRY;                              
                //Select new move to start up (Idea: Start from one point and go to others)
                ReturnEvent.EventType = ES_NO_EVENT;
              }                
              break;
              default:
              {
                ;
              }
            }
          }
        }
      break;
              
     
       case Orienting2Recycle :      
			 {
         ReturnEvent = CurrentEvent = DuringOrienting2Recycle(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
              case EV_ALIGNED2BEACON:
              {

                IRDisableInterrupt();
                  //Stop driving motors
                  StopDrive();
                  // Execute action function for state one : event one
                  NextState = Driving2Recycle;//Decide what the next state will be
                  // for internal transitions, skip changing MakeTransition
                  MakeTransition = true; //mark that we are taking a transition
                  // if transitioning to a state with history change kind of entry
                  EntryEventKind.EventType = ES_ENTRY;                              
                  //Select new move to start up (Idea: Start from one point and go to others)
                  ReturnEvent.EventType = ES_NO_EVENT;
                
              }
              break;
              
              case EV_COMPASS_RECYCLE_CHANGE:
              {
                StopDrive();
                // Execute action function for state one : event one
                NextState = Orienting2Recycle;//Decide what the next state will be
                // for internal transitions, skip changing MakeTransition
                MakeTransition = true; //mark that we are taking a transition
                // if transitioning to a state with history change kind of entry
                EntryEventKind.EventType = ES_ENTRY;                              
                //Consuming event  
                ReturnEvent.EventType = ES_NO_EVENT;
                
              }
             	break;
              
              case EV_MOVE_COMPLETED: //Couldn't find the beacon
              {
                NextState = CurrentState;//Decide what the next state will be
                // for internal transitions, skip changing MakeTransition
                MakeTransition = true; //mark that we are taking a transition
                // if transitioning to a state with history change kind of entry
                EntryEventKind.EventType = ES_ENTRY;                              
                //Select new move to start up (Idea: Start from one point and go to others)
                ReturnEvent.EventType = ES_NO_EVENT;
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
		
		case Driving2Recycle:
		{
         ReturnEvent = CurrentEvent = DuringDriving2Recycle(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
              
              case EV_MOVE_COMPLETED:
              {
                    NextState = Orienting2Recycle;
                    
                    
                    // for internal transitions, skip changing MakeTransition
                    MakeTransition = true; //mark that we are taking a transition
                    // if transitioning to a state with history change kind of entry
                    EntryEventKind.EventType = ES_ENTRY;                              
                    //Consuming event  
                    ReturnEvent.EventType = ES_NO_EVENT;                 
              }
              break;
               
              case EV_BUMPER_HIT: //This can either be a bot or the recycling center 
              {
                if((CurrentEvent.EventParam == 1) || (CurrentEvent.EventParam == 2))
                {
                  ES_Timer_InitTimer(COLLISION_TIMER, COLLISION_TIME);
                  ReturnEvent.EventType = ES_NO_EVENT;
  

                }
              }
              break;
              
              
              case ES_TIMEOUT:
              {
                if(CurrentEvent.EventParam == COLLISION_TIMER)
                {
                  NumberofKisses++;
                  
                  //It can't have been a bot
                  if (NumberofKisses >= KISS_THRESHOLD)
                  {
                    NextState = DumpingRecycle;
                    MakeTransition = true;
                    
                    EntryEventKind.EventType = ES_ENTRY;
                    ReturnEvent.EventType = ES_NO_EVENT;
                  }
                  else //Still investigating whether it was a bot
                  {
                  
                    NextState = Preparing4Recycle;
                    
                    
                    // for internal transitions, skip changing MakeTransition
                    MakeTransition = true; //mark that we are taking a transition
                    // if transitioning to a state with history change kind of entry
                    EntryEventKind.EventType = ES_ENTRY;                              
                    //Consuming event  
                    ReturnEvent.EventType = ES_NO_EVENT;  
                  }                    
                }
              }
              break;

              case EV_COMPASS_RECYCLE_CHANGE:
              {
                
                StopDrive();
                NumberofKisses = 0;
                // Execute action function for state one : event one
                NextState = Orienting2Recycle;//Decide what the next state will be
                // for internal transitions, skip changing MakeTransition
                MakeTransition = true; //mark that we are taking a transition
                // if transitioning to a state with history change kind of entry
                EntryEventKind.EventType = ES_ENTRY;                              
                //Consuming event  
                ReturnEvent.EventType = ES_NO_EVENT;
                
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
		
		case Preparing4Recycle:
		{
         ReturnEvent = CurrentEvent = DuringPreparing4Recycle(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
              case EV_MOVE_COMPLETED:
              {
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
                  NextState = Driving2Recycle;//Decide what the next state will be
                  // for internal transitions, skip changing MakeTransition
                  MakeTransition = true; //mark that we are taking a transition
                  // if transitioning to a state with history change kind of entry
                  EntryEventKind.EventType = ES_ENTRY;                              
                  //Consume event 
                  ReturnEvent.EventType = ES_NO_EVENT;
                }
                
              }
              break;							 
              case EV_COMPASS_RECYCLE_CHANGE:
              {
                NumberofKisses = 0;
                StopDrive();
                // Execute action function for state one : event one
                NextState = Orienting2Recycle;//Decide what the next state will be
                // for internal transitions, skip changing MakeTransition
                MakeTransition = true; //mark that we are taking a transition
                // if transitioning to a state with history change kind of entry
                EntryEventKind.EventType = ES_ENTRY;                              
                //Consuming event  
                ReturnEvent.EventType = ES_NO_EVENT;
                
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
    
		case DumpingRecycle:
		{
         ReturnEvent = CurrentEvent = DuringDumpingRecycle(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {

              case EV_RECYCLING_DONE:
              {
                StopDrive();
                //Close the Recycling Door: TO BE DONE
                NextState = RecoveringFromRecycle;//Decide what the next state will be
                // for internal transitions, skip changing MakeTransition
                MakeTransition = true; //mark that we are taking a transition
                // if transitioning to a state with history change kind of entry
                EntryEventKind.EventType = ES_ENTRY;                              
                //Consuming event  
                ReturnEvent.EventType = ES_NO_EVENT;                 
                //Process event at a higher level
                               
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
    
    
		case RecoveringFromRecycle:
		{
         ReturnEvent = CurrentEvent = DuringRecoveringFromRecycle(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {

              case EV_MOVE_COMPLETED:
              {
                StopDrive();
                
                //Process event at a higher level
                ReturnEvent.EventType = EV_RECYCLING_DONE;
                
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
       RunRecyclingSM(CurrentEvent);

       CurrentState = NextState; //Modify state variable

       //   Execute entry function for new state
       // this defaults to ES_ENTRY
       RunRecyclingSM(EntryEventKind);
     }
     return(ReturnEvent);
}
/****************************************************************************
 Function
     StartRecyclingSM

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
void StartRecyclingSM ( ES_Event_t CurrentEvent )
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
   RunRecyclingSM(CurrentEvent);
}

/****************************************************************************
 Function
     QueryTemplateSM

 Parameters
     None

 Returns
     RecyclingState_t The current state of the Template state machine

 Description
     returns the current state of the Template state machine
 Notes

 Author
     J. Edward Carryer, 2/11/05, 10:38AM
****************************************************************************/
RecyclingState_t QueryRecyclingSM(void)
{
   return CurrentState;
}

/***************************************************************************
 private functions
 ***************************************************************************/
static ES_Event_t DuringOrienting2Landfill (ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption
    
    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
      // implement any entry actions required for this state machine
      // Set frequency to detect for IR
      // Set Posting possibility of IR to true
      // Enable IR interrupts
      // implement any entry actions required for this state machine
      // Set frequency to detect for IR
      // Set Posting possibility of IR to true
      // Enable IR interrupts      
      if(Align2North == true)
      {
        printf("Trying to find North beacon \r\n");
        ActivateBeaconFinder(NORTH_LANDFILL_PERIOD);
        
      }
      else //Align2North = false
      {
        printf("Trying to find South beacon \r\n");
        ActivateBeaconFinder(SOUTH_LANDFILL_PERIOD);
      }
      //Next time we will align with the other beacon
      Align2North = ! Align2North;
      
      IREnableInterrupt();
      
      // Start rotating (360 degrees but can be interferred)
      DriveRotate(LOCALIZATION_SPEED, -3600);			
				
      // after that start any lower level machines that run in this state
      //StartLowerLevelSM( Event );
      // repeat the StartxxxSM() functions for concurrent state machines
      // on the lower level
    }
    else if ( Event.EventType == ES_EXIT )
    {
      IRDisableInterrupt();
      //Stop Motors (should already be the case but to be sure)
      StopDrive();
        // on exit, give the lower levels a chance to clean up first
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
  

static ES_Event_t DuringDriving2Landfill ( ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
      enableEquatorDetection();
        // implement any entry actions required for this state machine
        //Basically drive and never stop until we hit a beacon
        DriveStraight(STRAIGHT_SPEED, 14400);
				
        // after that start any lower level machines that run in this state
        //StartLowerLevelSM( Event );
        // repeat the StartxxxSM() functions for concurrent state machines
        // on the lower level
    }
    else if ( Event.EventType == ES_EXIT )
    {
      StopDrive();
      //Stop Motors
      disableEquatorDetection();
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
static ES_Event_t DuringMovingBodyRotation ( ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
        
      // Start rotating (360 degrees but can be interferred)
      DriveStraight(STRAIGHT_SPEED, 1000); //8 inches: KRISTINE TO MODIFY	
				
      // after that start any lower level machines that run in this state
      //StartLowerLevelSM( Event );
      // repeat the StartxxxSM() functions for concurrent state machines
      // on the lower level
    }
    else if ( Event.EventType == ES_EXIT )
    {
      //Stop Motors (should already be the case but to be sure)
      StopDrive();
        // on exit, give the lower levels a chance to clean up first
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

static ES_Event_t DuringOrienting2Recycle( ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
      // implement any entry actions required for this state machine
      // Set frequency to detect for IR
      // Set Posting possibility of IR to true
      // Enable IR interrupts
      
      
      /*if(OurRecyclePeriod == EAST_RECYCLE)
      {
        ActivateBeaconFinder(EAST_RECYCLING_PERIOD);
      }
      else
      {
        ActivateBeaconFinder(WEST_RECYCLING_PERIOD);
      }
      //ActivateBeaconFinder(EAST_RECYCLING_PERIOD);
      IREnableInterrupt();
      */
      //HARDCODING RECYCLING
      if(QueryTeam() == TEAM_NORTH)
      {
        ActivateBeaconFinder(EAST_RECYCLING_PERIOD);
      }
      else
      {
        ActivateBeaconFinder(WEST_RECYCLING_PERIOD);
      }
      IREnableInterrupt();
        
        


      // Start rotating (360 degrees but can be interferred)
      DriveRotate(LOCALIZATION_SPEED, 3600);			
				
      // after that start any lower level machines that run in this state
      //StartLowerLevelSM( Event );
      // repeat the StartxxxSM() functions for concurrent state machines
      // on the lower level
    }
    else if ( Event.EventType == ES_EXIT )
    {
      IRDisableInterrupt();
      //Stop Motors (should already be the case but to be sure)
      StopDrive();
        // on exit, give the lower levels a chance to clean up first
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


static ES_Event_t DuringDriving2Recycle( ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
        // implement any entry actions required for this state machine
        DriveStraight(STRAIGHT_SPEED, 1200);
        //HARDCODING RECYCLING
        //UpdateEmitterPeriod(GetAssignedPeriod());

      

				
        // after that start any lower level machines that run in this state
        //StartLowerLevelSM( Event );
        // repeat the StartxxxSM() functions for concurrent state machines
        // on the lower level
    }
    else if ( Event.EventType == ES_EXIT )
    {
      StopDrive();
      //Stop Motors
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

static ES_Event_t DuringPreparing4Recycle( ES_Event_t Event)
{
   ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
        
      DriveStraight(PREPARE4DUMP_SPEED, -PREPARE4DUMP_BACKUPDISTANCE);
				
        // after that start any lower level machines that run in this state
        //StartLowerLevelSM( Event );
        // repeat the StartxxxSM() functions for concurrent state machines
        // on the lower level
    }
    else if ( Event.EventType == ES_EXIT )
    {
      //Stop Motors
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

static ES_Event_t DuringDumpingRecycle( ES_Event_t Event)
{
   ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
        // implement any entry actions required for this state machine
        StopDrive();
        ES_Event_t DumpEvent;
        DumpEvent.EventType = EV_DUMP_RECYCLE;
        PostBallDumpingSM(DumpEvent);
			
				
        // after that start any lower level machines that run in this state
        //StartLowerLevelSM( Event );
        // repeat the StartxxxSM() functions for concurrent state machines
        // on the lower level
    }
    else if ( Event.EventType == ES_EXIT )
    {
      DisableEmitterPWM();
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

static ES_Event_t DuringRecoveringFromRecycle( ES_Event_t Event)
{
   ES_Event_t ReturnEvent = Event; // assume no re-mapping or consumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
        // implement any entry actions required for this state machine
        DriveRotate(PREPARE4DUMP_SPEED, -900); 
			
				
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

void SetRecycleCenter(uint8_t RecycleCenter)
{
  if (RecycleCenter == EAST_RECYCLE)
  {
    OurRecyclePeriod = EAST_RECYCLE_FREQUENCY;
    OppRecyclePeriod = WEST_RECYCLE_FREQUENCY;
  }
  else
  {
    OurRecyclePeriod = WEST_RECYCLE_FREQUENCY;
    OppRecyclePeriod = EAST_RECYCLE_FREQUENCY;
  }
}
