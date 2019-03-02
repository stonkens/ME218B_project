/****************************************************************************
 Module
   OrientingSM.c

 Revision
   2.0.1

 Description
   Low-level state machine implementing triangulation with IR beacons

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 03/06/16 23:45 ston     Final revision for 218B project
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

//#include "IR_Sense.h"
//#include "Enc_Sense.h"
#include "Triangulation.h"

#include "OrientingSM.h"

/*----------------------------- Module Defines ----------------------------*/
// define constants for the states for this machine
// and any other local defines

#define ENTRY_STATE Measuring

#define	BEACON_IR		0
#define FIRST_EDGE	1
#define	LAST_EDGE	2
#define AVE_EDGE	3

#define MAX_FREQ_ERROR	15

#define IR_MEASURE_DELAY 2

#define pi 3.14159265358979323846f

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/

static void CaptureAngle(uint32_t frequency);
static void CalculatePosition(void);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static OrientingState_t CurrentState;
static float Current_X;
static float Current_Y;
static float Current_Heading;
static uint32_t LastRunCount;

static float Angle_Measurements[4][4] = {
	{1950,	0,	0,	0},
	{1250,	0,	0,	0},
	{1700,	0,	0,	0},
	{1450,	0,	0,	0}};

static uint32_t LastFrequency;
static uint8_t	MissedBeacons;
	
/*------------------------------ Module Code ------------------------------*/

/***************************************************************************
 getters and/or setters for module variables
 ***************************************************************************/
float GetCurrent_X(void){
	return Current_X;
}

float GetCurrent_Y(void){
	return Current_Y;
}

float GetCurrent_Heading(void){
	return Current_Heading;
}

void ResetOrientingRunCount(void){
	LastRunCount = 0;
	IR_ResetRunCount();
}

void ClearMeasurements(void){
	for(int i=0;i<4;i++){
		Angle_Measurements[i][FIRST_EDGE] = 0;
		Angle_Measurements[i][LAST_EDGE] = 0;
		Angle_Measurements[i][AVE_EDGE] = 0;
	}
}

/****************************************************************************
 Function
    RunOrientingSM

 Parameters
   ES_Event: the event to process

 Returns
   ES_Event: an event to return

 Description
   run function for low-level state machine implementing localization
   
 Notes
   LxW - Revised for OrientingSM
   uses nested switch/case to implement the machine.
   
 Author
   Sander Tonkens
****************************************************************************/
ES_Event_t RunOrientingSM( ES_Event_t CurrentEvent )
{
	bool MakeTransition = false;/* are we making a state transition? */
	OrientingState_t NextState = CurrentState;
	ES_Event_t EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
	ES_Event_t ReturnEvent = CurrentEvent; // assume we are not consuming event

	switch ( CurrentState ){
		 //if current state is Measuring
		case Measuring :
			 //if an event is active
			if ( CurrentEvent.EventType != ES_NO_EVENT ){
				 //process it
				switch (CurrentEvent.EventType){
					 //if event is ES_IR_FOUND
					case ES_TIMEOUT:
          {
						 //consume the event
            if (CurrentEvent.EventParam == LOCALIZE_TIMER)
            {
              ReturnEvent.EventType = ES_NO_EVENT;
               //restart measurement delay timer
              ES_Timer_InitTimer(LOCALIZE_TIMER,IR_MEASURE_DELAY);
               //record found frequency
              LastFrequency = IR_GetFrequency();
               //if non-zero, record measurement at current angle
              if( LastRunCount != IR_GetRunCount() )
              {
                CaptureAngle(LastFrequency);
              }
               //store last run count
              LastRunCount = IR_GetRunCount();
            }
          }
					break;
							
					 //if event is ES_MOVE_COMPLETE
					case ES_MOVE_COMPLETE:
          {
            //calculate current Position on the field: X, Y, Orientation/Heading
						CalculatePosition();
          }
					break;
				}
			}
		break;
    }
	
    //   If we are making a state transition
    if (MakeTransition == true)
    {
       //   Execute exit function for current state
       CurrentEvent.EventType = ES_EXIT;
       RunOrientingSM(CurrentEvent);
       CurrentState = NextState; //Modify state variable
       //   Execute entry function for new state
       // this defaults to ES_ENTRY
       RunOrientingSM(EntryEventKind);
    }
	
	 //return ReturnEvent
    return(ReturnEvent);
}
/****************************************************************************
 Function
     StartOrientingSM

 Parameters
     None

 Returns
     None

 Description
     Does the required initialization for the SM
	 
 Notes
	

 Author
  Sander Tonkens
****************************************************************************/
void StartOrientingSM ( ES_Event_t CurrentEvent )
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
   RunOrientingSM(CurrentEvent);
}

/****************************************************************************
 Function
     QueryOrientingSM

 Parameters
     None

 Returns
     OrientingState_t The current state of the OrientingSM state machine

 Description
     returns the current state of the OrientingSM state machine
	 
 Notes

 Author
   Sander Tonkens
****************************************************************************/
OrientingState_t QueryOrientingSM ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/

 //capture the measured frequency and its current angle for use in triangulation
static void CaptureAngle(uint32_t frequency){
	uint8_t index;
	 //The angle measured is the average of both encoders (absolute values)
	float measuredAngle = ( ((float)abs(Enc_GetTickCount(1)))*135.5/100 + ((float)abs(Enc_GetTickCount(2)))*135.5/100)/2;
	 //for every beacon in Angle_Measurements[]
	for(int i=0;i < 4;i++){
		 //if frequency is the same (within bounds) as current position's contents in Angle_Measurements[]
		if( (frequency <= Angle_Measurements[i][BEACON_IR] + MAX_FREQ_ERROR) && (frequency >= Angle_Measurements[i][BEACON_IR] - MAX_FREQ_ERROR)){			
			//set the index to current IR Beacon it is aligned with
			index = i;
		}
	}
	
	 //if a first angle has not been captured
	if(Angle_Measurements[index][FIRST_EDGE] == 0){
		 //capture angle as a FIRST_EDGE
		Angle_Measurements[index][FIRST_EDGE] = measuredAngle;
	}
	else{
		 //capture angle as a LAST_EDGE
		Angle_Measurements[index][LAST_EDGE] = measuredAngle;
	}
}

 //use triangulation to calculate the current position off 3 beacons
static void CalculatePosition(void){
	float MinAngleWidth = 360;
	float BeaconSpread;
	uint8_t ExcludedAngle = 0;
	
	 //reset MissedBeacons
	MissedBeacons = 0;
	
	 //get average angle measurements for each beacon
	for(int i=0;i <= 3;i++){
		 //If the angle measurement wraps around 60 degrees we need to measure the average angle in a different way
		if(Angle_Measurements[i][FIRST_EDGE] <= 60 && Angle_Measurements[i][LAST_EDGE] >= 300){
			Angle_Measurements[i][AVE_EDGE] = ((Angle_Measurements[i][LAST_EDGE]-360) + Angle_Measurements[i][FIRST_EDGE])/2 + 360;
		}
		 //Else just calculate the average angle normally (by (x1+x2)/2))
		else{
			Angle_Measurements[i][AVE_EDGE] = (Angle_Measurements[i][LAST_EDGE] + Angle_Measurements[i][FIRST_EDGE])/2;
		}
			//printf("\r\nAngle %d average : %f",i,Angle_Measurements[i][AVE_EDGE]);
	}
	
	//Check which angle should be ignored
	for(int i=0;i <= 3;i++){
		BeaconSpread = abs(Angle_Measurements[i][LAST_EDGE] - Angle_Measurements[i][FIRST_EDGE]);
		if(BeaconSpread < MinAngleWidth){
			ExcludedAngle = i;
			MinAngleWidth = BeaconSpread;
		}
	}
	
	//Check if a beacon has missed, if yes: ExcludedAngle = MissedBeaconNbr
	for(int i=0; i <= 3;i++){ 
		if(Angle_Measurements[i][AVE_EDGE] == 0){
			MissedBeacons++;
			ExcludedAngle = i;
		}
	}
	 //if more than 1 beacon has been missed
	if(MissedBeacons > 1){
		 //Robot has to do another turn to relocalize
		SetPositionKnownStatus(false);
	}
	else{
		 //do next objective
		SetPositionKnownStatus(true);
	}
	
	 //Ignore angle that has most potential error
	Angle_Measurements[ExcludedAngle][AVE_EDGE] = -1;
	
	 //triangulate current position
	Triangulate(Angle_Measurements[0][AVE_EDGE],Angle_Measurements[1][AVE_EDGE],Angle_Measurements[2][AVE_EDGE],Angle_Measurements[3][AVE_EDGE]);
	Current_X = Get_X();
	Current_Y = Get_Y();
	Current_Heading = Get_Heading();
}