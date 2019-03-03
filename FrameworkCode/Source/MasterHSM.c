/****************************************************************************
 Module
   MasterHSM.c

 Revision
   2.0.1

 Description
   This is a template for the top level Hierarchical state machine

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/20/17 14:30 jec      updated to remove sample of consuming an event. We 
                         always want to return ES_NO_EVENT at the top level 
                         unless there is a non-recoverable error at the 
                         framework level
 02/03/16 15:27 jec      updated comments to reflect small changes made in '14 & '15
                         converted unsigned char to bool where appropriate
                         spelling changes on true (was True) to match standard
                         removed local var used for debugger visibility in 'C32
                         removed Microwave specific code and replaced with generic
 02/08/12 01:39 jec      converted from MW_MasterMachine.c
 02/06/12 22:02 jec      converted to Gen 2 Events and Services Framework
 02/13/10 11:54 jec      converted During functions to return Event_t
                         so that they match the template
 02/21/07 17:04 jec      converted to pass Event_t to Start...()
 02/20/07 21:37 jec      converted to use enumerated type for events
 02/21/05 15:03 jec      Began Coding
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "InitializeHardware.h"
#include "MasterHSM.h"

#include "SPISM.h"

// Header files for state machines at the next lower level in the hierarchy
#include "GamePlayHSM.h"
#include "CollisionAvoidanceHSM.h"



// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"

// the headers to access the TivaWare Library
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
static ES_Event_t DuringWaitingForStart( ES_Event_t Event);
static ES_Event_t DuringGamePlay (ES_Event_t Event);
static ES_Event_t DuringGameEnded (ES_Event_t Event);
static ES_Event_t DuringCollisionAvoidance (ES_Event_t Event);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, though if the top level state machine
// is just a single state container for orthogonal regions, you could get
// away without it
static MasterState_t CurrentState;
// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;
static uint8_t OurTeam;
/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitMasterSM

 Parameters
     uint8_t : the priorty of this service

 Returns
     boolean, False if error in initialization, True otherwise

 Description
     Saves away the priority,  and starts
     the top level state machine
 Notes

 Author
     J. Edward Carryer, 02/06/12, 22:06
****************************************************************************/
bool InitMasterSM ( uint8_t Priority )
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;  // save our priority

  ThisEvent.EventType = ES_ENTRY;
  // Start the Master State machine

	//Initialize North/South team designation
	OurTeam = HWREG(GPIO_PORTE_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT3HI;
  
  if (OurTeam == TEAM_NORTH)
  {
    printf("\r\n Go Team North\r\n");
  }
  else
  {
    printf("\r\n Go Team South\r\n");
  }
  
  //The SSI communication service can start
  SetReady2Communicate(true);
  
	//Stop all motors
	StopAllMovingParts();
	//"Close" all servos: TO BE DONE
	
  
  StartMasterSM( ThisEvent );

  return true;
}

/****************************************************************************
 Function
     PostMasterSM

 Parameters
     ES_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the post operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostMasterSM( ES_Event_t ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunMasterSM

 Parameters
   ES_Event: the event to process

 Returns
   ES_Event: an event to return

 Description
   the run function for the top level state machine 
 Notes
   uses nested switch/case to implement the machine.
 Author
   J. Edward Carryer, 02/06/12, 22:09
****************************************************************************/
ES_Event_t RunMasterSM( ES_Event_t CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   MasterState_t NextState = CurrentState;
   ES_Event_t EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
   ES_Event_t ReturnEvent = { ES_NO_EVENT, 0 }; // assume no error

    switch ( CurrentState )
   {
       case WaitingForStart :       // If current state is state one
			 {
         // Execute During function for state one. ES_ENTRY & ES_EXIT are
         // processed here allow the lowere level state machines to re-map
         // or consume the event
         CurrentEvent = DuringWaitingForStart(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case EV_COMPASS_CLEANING_UP : //If event is event one
							 {
                 printf("\r\n Master Acknowledges Start of Game\r\n");
                  // Execute action function for state one : event one
                  NextState = GamePlay;//Decide what the next state will be
                  // for internal transitions, skip changing MakeTransition
                  MakeTransition = true; //mark that we are taking a transition
                  // if transitioning to a state with history change kind of entry
                  EntryEventKind.EventType = ES_ENTRY_HISTORY;
								 
								  //Start Timer which periodically checks how many balls have been recycled
								  ES_Timer_InitTimer(BALL_COLLECTION_TIMER, BALL_COLLECTION_TIME); //Every 20 seconds (To be modified
                  
							 }
							 break;
							 
							 case EV_COMPASS_GAME_OVER:
							 {
								 NextState = GameEnded;
								 MakeTransition = true;
								 EntryEventKind.EventType = ES_ENTRY;
							 }
							 break;
                // repeat cases as required for relevant events
							 default:
							 {;
							 }								 
						}
         }
			 }
       break;
      
			 case GamePlay:
			 {
				 CurrentEvent = DuringGamePlay(CurrentEvent);
				 
				 if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case EV_BUMPER_HIT : //If event is event one
							 {
                  // Execute action function for state one : event one
                  NextState = CollisionAvoidance;//Decide what the next state will be
                  // for internal transitions, skip changing MakeTransition
                  MakeTransition = true; //mark that we are taking a transition
                  // if transitioning to a state with history change kind of entry
                  EntryEventKind.EventType = ES_ENTRY;
                  
							 }
							 
							 break;
                // repeat cases as required for relevant events
							 default:
							 {;
							 }								 
						}
         }
			 }
       break;
			 
			 case GameEnded:
			 {
				 CurrentEvent = DuringGameEnded(CurrentEvent);
			 }
       break;	

			 case CollisionAvoidance:
			 {
				 CurrentEvent = DuringCollisionAvoidance(CurrentEvent);
				 
				 if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case EV_MOVED_BACK : //If event is event one
							 {
                  // Execute action function for state one : event one
                  NextState = GamePlay;//Decide what the next state will be
                  // for internal transitions, skip changing MakeTransition
                  MakeTransition = true; //mark that we are taking a transition
                  // if transitioning to a state with history change kind of entry
                  EntryEventKind.EventType = ES_ENTRY_HISTORY;
                  
							 }
							 break;
							 
							 case EV_COMPASS_GAME_OVER :
							 {
								 NextState = GameEnded;
								 MakeTransition = true;
								 EntryEventKind.EventType = ES_ENTRY;
							 }
                // repeat cases as required for relevant events
							 default:
							 {;
							 }								 
						}
         }				 
			 }
			 
			 
			 
			 default:
			 {;
			 }
				 
    }
    //   If we are making a state transition
    if (MakeTransition == true)
    {
       //   Execute exit function for current state
       CurrentEvent.EventType = ES_EXIT;
       RunMasterSM(CurrentEvent);

       CurrentState = NextState; //Modify state variable

       // Execute entry function for new state
       // this defaults to ES_ENTRY
       RunMasterSM(EntryEventKind);
     }
   // in the absence of an error the top level state machine should
   // always return ES_NO_EVENT, which we initialized at the top of func
   return(ReturnEvent);
}
/****************************************************************************
 Function
     StartMasterSM

 Parameters
     ES_Event_t CurrentEvent

 Returns
     nothing

 Description
     Does any required initialization for this state machine
 Notes

 Author
     J. Edward Carryer, 02/06/12, 22:15
****************************************************************************/
void StartMasterSM ( ES_Event_t CurrentEvent )
{
  // if there is more than 1 state to the top level machine you will need 
  // to initialize the state variable
  CurrentState = WaitingForStart;
  // now we need to let the Run function init the lower level state machines
  // use LocalEvent to keep the compiler from complaining about unused var
  RunMasterSM(CurrentEvent);
  return;
}


/***************************************************************************
 private functions
 ***************************************************************************/

static ES_Event_t DuringWaitingForStart( ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event; // assme no re-mapping or comsumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
        // implement any entry actions required for this state machine
        
        // after that start any lower level machines that run in this state
        //StartLowerLevelSM( Event );
        // repeat the StartxxxSM() functions for concurrent state machines
        // on the lower level
			
    }
    else if ( Event.EventType == ES_EXIT )
    {
			
        // on exit, give the lower levels a chance to clean up first
        //RunLowerLevelSM(Event);
        // repeat for any concurrently running state machines
        // now do any local exit functionality
			
      //if NextState
    }
		
		else
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

static ES_Event_t DuringGamePlay(ES_Event_t Event)
{
	ES_Event_t ReturnEvent = Event;
	
  // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
  if ((Event.EventType == ES_ENTRY) ||
      (Event.EventType == ES_ENTRY_HISTORY))
  {
		
		StartGamePlaySM(Event);
	}
	else if (Event.EventType == ES_EXIT)
	{
		RunGamePlaySM(Event);
	}
	//During function for this state
	else
	{
		ReturnEvent = RunGamePlaySM(Event);
	}
	
	return ReturnEvent;
}

static ES_Event_t DuringCollisionAvoidance(ES_Event_t Event)
{
	ES_Event_t ReturnEvent = Event;
	
  // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
  if ((Event.EventType == ES_ENTRY) ||
      (Event.EventType == ES_ENTRY_HISTORY))
  {
		
		StartCollisionAvoidanceSM(Event);
	}
	else if (Event.EventType == ES_EXIT)
	{
		RunCollisionAvoidanceSM(Event);
	}
	//During function for this state
	else
	{
		ReturnEvent = RunCollisionAvoidanceSM(Event);
	}
	return ReturnEvent;
	
}

static ES_Event_t DuringGameEnded (ES_Event_t Event)
{
	ES_Event_t ReturnEvent = Event;
	
  // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
  if ((Event.EventType == ES_ENTRY) ||
      (Event.EventType == ES_ENTRY_HISTORY))
  {
    //Stop moving all parts (set all moving parts down)	
    StopAllMovingParts();
    
		//Turn off all LEDs
    HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT0HI;
    HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT1HI;  
	}
	else if (Event.EventType == ES_EXIT)
	{
	}
	//During function for this state
	else
	{
	}
	
	return ReturnEvent;
		
}


uint8_t QueryTeam(void)
{
  return OurTeam;
}


