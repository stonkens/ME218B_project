/****************************************************************************
 Module
   MotorSpeedControl.c

 Revision
   1.0.1

 Description
   Motor speed control module for 218b project drive motors

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/16/16 15:05 lxw		 first pass
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for the framework and this service
*/

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"
#include "BITDEFS.h"

#include "MotorSpeedControl.h"
#include "EncoderCapture.h"


#include "inc/hw_timer.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"

/*----------------------------- Module Defines ----------------------------*/
#define LEFT_A 1
#define RIGHT_A 2
#define LEFT_B 3
#define RIGHT_B 4
#define BOTH	0

#define TICKS_PER_SECOND	 40000000
#define TICKS_PER_MS 40000
#define GEAR_RATIO 50
#define PULSES_PER_REV 5

#define MIN_ERROR	0
#define MIN_TICKS	0

#define KPM	2
#define KDM 10
#define KPD	5
#define KDD	10

#define RPM_P_GAIN	2
#define RPM_I_GAIN	0.3

#define STRAIGHT	0
#define TURN			1

#define UPDATE_TIME 	2			//Adjusted every 2 ms

/*---------------------------- Module Functions ---------------------------*
  prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

//static void SetRPM(uint8_t wheel, float newSpeed);
static float Clamp(float, float, float);

/*---------------------------- Module Variables ---------------------------*/

//Data private to the module
static float DesiredDistance;
static float DesiredHeading;
static float MuError;
static float DeltaError;
static float MuV;
static float DeltaV;
static float LastMuError;
static float LastDeltaError;

static int LastTickCount_1;
static int LastTickCount_2;
static float DesiredSpeed_1;
static float DesiredSpeed_2;
static float LastRecordedSpeed_1;
static float LastRecordedSpeed_2;
static float RequestedDuty_1;
static float RequestedDuty_2;
static float RPMError_1;
static float RPMError_2;
static float IntegralTerm_1;
static float IntegralTerm_2;

static bool Driving;
static float ClampRPM;

static uint32_t runCount;
static uint32_t checkSpeed_2;

static float ClampPWM = 100;

/*------------------------------ Module Code ------------------------------*/

uint32_t GetRunCount(void){
	return runCount;
}

uint16_t GetDuty2(void){
	return RequestedDuty_1;
}

uint16_t GetDuty1(void){
	return RequestedDuty_2;
}

uint32_t checkSpeed(void){
	return checkSpeed_2;
}

float GetMuV(void){
	return MuV;
}

float GetDeltaV(void){
	return DeltaV;
}

float GetMuError(void){
	return MuError;
}

float GetDeltaError(void){
	return DeltaError;
}

/****************************************************************************
 Function
     Drive_SpeedControl_Init

 Parameters
     void

 Returns
     void

 Description
     initializes speed control for drive motors
 Notes

 Author
     LXW, 02/16/16, 15:20
****************************************************************************/
void Drive_SpeedControl_Init(void){
	 //initialize drive motor control
	Drive_Motor_Init();
	
	 //initialize drive motor encoders
	Enc_Sense_Init();
	
	 //initialize the periodic speed update timer
	Drive_SpeedUpdate_Timer_Init(UPDATE_TIME);
}

void Drive_Stop(void){
	Driving = false;
	 //reset control variables
	DesiredDistance = 0;
	DesiredHeading = 0;
	Enc_ResetTickCount(BOTH);
	LastTickCount_1 = 0;
	LastTickCount_2 = 0;
	IntegralTerm_1 = 0;
	IntegralTerm_2 = 0;
}

void Drive_SetDistance(float newLimit){
	 //reset control variables
	Enc_ResetTickCount(BOTH);
	LastTickCount_1 = 0;
	LastTickCount_2 = 0;
	IntegralTerm_1 = 0.0;
	IntegralTerm_2 = 0.0;
	 //set new distance setpoint
	DesiredHeading = 0;
	DesiredDistance = newLimit;
	Driving = true;
}

void Drive_SetHeading(float newLimit){
	 //reset control variables
	Enc_ResetTickCount(BOTH);
	LastTickCount_1 = 0;
	LastTickCount_2 = 0;
	IntegralTerm_1 = 0.0;
	IntegralTerm_2 = 0.0;
	 //set new distance setpoint
	DesiredHeading = newLimit;
	DesiredDistance = 0;
	Driving = true;
}

/****************************************************************************
 Function
   Drive_SetClampRPM

 Parameters
float : new clamping rpm

 Returns
   void

 Description
	set new clamp RPM
 Notes
   
 Author
   lxw, 02/17/16, 13:50
****************************************************************************/
void Drive_SetClampRPM(float newRPM){
	ClampRPM = newRPM;
}

/****************************************************************************
 Function
   Drive_GetRPM

 Parameters
	uint8_t : selected wheel

 Returns
	uint16_t : last measured RPM of selected wheel

 Description
	return last recorded RPM for a drive wheel
 Notes
   
 Author
   lxw, 02/16/16, 16:15
****************************************************************************/
float Drive_GetRPM(uint8_t wheel){
	if(wheel == LEFT_A){
		return LastRecordedSpeed_1;
	}
	else if(wheel == RIGHT_A){
		return LastRecordedSpeed_2;
	}
	else{
		return 0;
	}
}

/****************************************************************************
 Function
   Drive_SpeedControlISR

 Parameters
	void

 Returns
   void

 Description
	interrupt response for DC motor control loop
 Notes
   
 Author
   lxw, 01/28/16, 19:00
****************************************************************************/
void Drive_SpeedControlISR(void){
	 //control loop variables
	
	runCount++;
	
	 //start by clearing the source of the interrupt
	HWREG(WTIMER1_BASE+TIMER_O_ICR) = TIMER_ICR_TATOCINT;
	
	//****************************************************************NEW INFO GATHERING
	//for wheel 1 (LEFT)
	 //if not enough new ticks have not been found
	if(abs(Enc_GetTickCount(1) - LastTickCount_1) <= MIN_TICKS){
		 //set last recorded encoder RPM to 0
		LastRecordedSpeed_1 = 0;
	}
	 //else
	else{
		 //calculate RPM from current period
		LastRecordedSpeed_1 = ((TICKS_PER_SECOND/(Enc_GetPeriod(LEFT_A))*60)/(PULSES_PER_REV*GEAR_RATIO));
		 //capture new tick count
		LastTickCount_1 = Enc_GetTickCount(1);
	}
	
	//for wheel 2 (RIGHT)
	 //if not enough new ticks have not been found
	if(abs(Enc_GetTickCount(2) - LastTickCount_2) <= MIN_TICKS){
		 //set last recorded encoder RPM to 0
		LastRecordedSpeed_2 = 0;
	}
	 //else
	else{
		 //calculate RPM from current period
		LastRecordedSpeed_2 = ((TICKS_PER_SECOND/(Enc_GetPeriod(RIGHT_A))*60)/(PULSES_PER_REV*GEAR_RATIO));
		 //capture new tick count
		LastTickCount_2 = Enc_GetTickCount(2);
	}
	
	//****************************************************************POSITION CONTROL
	
	MuError = (DesiredDistance - ((LastTickCount_1+LastTickCount_2)/2)); //taking average
	DeltaError = (DesiredHeading- ((LastTickCount_2-LastTickCount_1)/2));
	MuV = KPM*MuError + KDM*(MuError-LastMuError);
	DeltaV = KPD*DeltaError + KDD*(DeltaError-LastDeltaError); // pos delta is right wheel neg is left wheel
	DesiredSpeed_1 = Clamp(MuV - DeltaV, -ClampRPM, ClampRPM);
	DesiredSpeed_2 = Clamp(MuV + DeltaV, -ClampRPM, ClampRPM);
	LastMuError = MuError;
	LastDeltaError = DeltaError;
 
	//****************************************************************SPEED CONTROL FOR LEFT MOTOR (1)
	
	RPMError_1 = DesiredSpeed_1 - LastRecordedSpeed_1;
	
	IntegralTerm_1 += RPMError_1;
	IntegralTerm_1 = Clamp(IntegralTerm_1, -ClampPWM, ClampPWM); /* anti-windup */
	
	RequestedDuty_1 = RPM_P_GAIN*RPMError_1 + RPM_I_GAIN*IntegralTerm_1;
	RequestedDuty_1 = Clamp(RequestedDuty_1, -ClampPWM, ClampPWM);
	
	Drive_SetDuty_Left(RequestedDuty_1); // output calculated control
	
	//****************************************************************SPEED CONTROL FOR RIGHT MOTOR (2)
	
	RPMError_2 = DesiredSpeed_2 - LastRecordedSpeed_2;
	
	IntegralTerm_2 += RPMError_2;
	IntegralTerm_2 = Clamp(IntegralTerm_2, -ClampPWM, ClampPWM); /* anti-windup */
	
	RequestedDuty_2 = RPM_P_GAIN*RPMError_2 + RPM_I_GAIN*IntegralTerm_2;
	RequestedDuty_2 = Clamp(RequestedDuty_2, -ClampPWM, ClampPWM);
	
	Drive_SetDuty_Right(RequestedDuty_2); // output calculated control
	
	//****************************************************************TICK COUNT MONITOR
	
	 //if position is within error bounds
	if( (abs(DeltaError) <= MIN_ERROR) && (abs(MuError) <= MIN_ERROR) && Driving == true){
		Driving = false;
		
		//post event to Master SM indicating that target has been reached
		//ES_Event doneEvent;
		//doneEvent.EventType = ES_MOVE_COMPLETE;
		//PostCampaignService(doneEvent);
	}
	
}

/****************************************************************************
 Function
  clamp

 Parameters
	3x float

 Returns
	input value, clamped between upper and lower input bounds

 Description
	add your description here
 Notes
   
 Author
	Sander Tonkens
****************************************************************************/
static float Clamp(float x, float LowBound, float HighBound){
	if(x >= HighBound){
		x = HighBound;
	}
	else if(x <= LowBound){
		x = LowBound;
	}
	return x;
}