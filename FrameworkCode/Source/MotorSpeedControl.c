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
 02/16/16 15:05 ST		 first pass
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
#include "DriveMotorPWM.h"

#include "MotorService.h"

#include <math.h>
#include "inc/hw_timer.h"
#include "inc/hw_memmap.h"
#include "hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"

/*----------------------------- Module Defines ----------------------------*/
#define TICKS_PER_SECOND	 40000000
#define TICKS_PER_MS 40000
#define GEAR_RATIO 50
#define PULSES_PER_REV 3

#define MIN_ERROR	2
#define MIN_TICKS	0

#define KPM	2
#define KDM 10
#define KPD	5
#define KDD	10

#define RPM_P_GAIN 2
#define RPM_I_GAIN 0.3


#define UPDATE_TIME 2			//Adjusted every 2 ms

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
static float DistanceError;
static float HeadingError;
static float DistancePDTerm;
static float HeadingPDTerm;
static float LastDistanceError;
static float LastHeadingError;

static int LastTickCount_1;
static int LastTickCount_2;
static float DesiredSpeed_1;
static float DesiredSpeed_2;
static float LastRecordedSpeed_1;
static float LastRecordedSpeed_2;
static float UpdatedDutyCycle_1;
static float UpdatedDutyCycle_2;
static float RPMError_1;
static float RPMError_2;
static float IntegralTerm_1;
static float IntegralTerm_2;

static bool Driving;
static float ClampRPM;

static uint32_t ControlLoopCount;


static float ClampPWM = 100;

/*------------------------------ Module Code ------------------------------*/

uint32_t QueryRunCount(void){
	return ControlLoopCount;
}

uint16_t QueryDuty2(void){
	return UpdatedDutyCycle_1;
}

uint16_t QueryDuty1(void){
	return UpdatedDutyCycle_2;
}


float QueryDistancePDTerm(void){
	return DistancePDTerm;
}

float QueryHeadingPDTerm(void){
	return HeadingPDTerm;
}

float QueryDistanceError(void){
	return DistanceError;
}

float QueryHeadingError(void){
	return HeadingError;
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
     Sander Tonkens
****************************************************************************/
void Drive_SpeedControl_Init(void){
	//All initializations done in InitializeHardware.c
	//Initialize drive motor control
	//InitDrivePWM();
	//InitDriveMotor(GPIO);
	
	//initialize drive motor encoders
	//Enc_Sense_Init();
	
	 //initialize the periodic speed update timer
	Drive_SpeedUpdateTimer_Init(UPDATE_TIME);
}

void Drive_Stop(void){
	Driving = false;
	 //reset control variables
	DesiredDistance = 0;
	DesiredHeading = 0;
	//Reset tick count of both encoders
	ResetEncoderTickCount(BOTH_WHEELS);
	
	//Reset number of wheel spins
	LastTickCount_1 = 0;
	LastTickCount_2 = 0;
	//Reset integral term of controller
	IntegralTerm_1 = 0;
	IntegralTerm_2 = 0;
}

void Drive_SetDistance(float newLimit){
	//reset control variables
	//Reset tick count of both encoders
	ResetEncoderTickCount(BOTH_WHEELS);
	//Reset number of wheel spins
	LastTickCount_1 = 0;
	LastTickCount_2 = 0;
	//Reset integral term of controller
	IntegralTerm_1 = 0.0;
	IntegralTerm_2 = 0.0;
	 //set new distance setpoint
	DesiredHeading = 0;
	DesiredDistance = newLimit;
	Driving = true;
}

void Drive_SetHeading(float newLimit){
	 //reset control variables
	//Reset tick count of both encoders
	ResetEncoderTickCount(BOTH_WHEELS);
	//Reset number of wheel spins
	LastTickCount_1 = 0;
	LastTickCount_2 = 0;
	//Reset integral term of controller
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
   Sander Tonkens
****************************************************************************/
void Drive_SetClampRPM(float MaxRPM){
	ClampRPM = MaxRPM;
}

/****************************************************************************
 Function
  QueryDriveRPM

 Parameters
	uint8_t : selected wheel

 Returns
	uint16_t : last measured RPM of selected wheel

 Description
	return last recorded RPM for a drive wheel
 Notes
   
 Author
   Sander Tonkens
****************************************************************************/
float QueryDriveRPM(uint8_t wheel){
	if(wheel == WHEEL1A){
		return LastRecordedSpeed_1;
	}
	else if(wheel == WHEEL2A){
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
   Sander Tonkens
****************************************************************************/
void Drive_SpeedControlISR(void){
	
	ControlLoopCount++;
	//printf("Timer interrupt\r\n");
	 //start by clearing the source of the interrupt
	HWREG(WTIMER5_BASE+TIMER_O_ICR) = TIMER_ICR_TATOCINT;
	
	//***Gather new info from DriveMotorPWM module***//
	
	//Determine Tick Counts for Motor 1
	//If not enough new ticks have not been registered (i.e. Motor is at standstill)
	if(fabsf(QueryEncoderTickCount(1) - LastTickCount_1) <= MIN_TICKS)
	{
		//Set last recorded Motor RPM to 0
		LastRecordedSpeed_1 = 0;
	}

	else
	{
		//Calculate RPM from current period
		LastRecordedSpeed_1 = ((TICKS_PER_SECOND/(QueryEncoderPeriod(WHEEL1A))*60)/(PULSES_PER_REV*GEAR_RATIO));
		//Query new tick count
		LastTickCount_1 = QueryEncoderTickCount(1);
	}
	
	//Determine Tick Counts for Motor 2
	//If not enough new ticks have not been registered (i.e. Motor is at standstill)
	if(fabsf(QueryEncoderTickCount(2) - LastTickCount_2) <= MIN_TICKS)
	{
		//Set last recorded Motor RPM to 0
		LastRecordedSpeed_2 = 0;
	}
	else{
		//Calculate current RPM from current period
		LastRecordedSpeed_2 = ((TICKS_PER_SECOND/(QueryEncoderPeriod(WHEEL2A))*60)/(PULSES_PER_REV*GEAR_RATIO));
		//Capture new tick count
		LastTickCount_2 = QueryEncoderTickCount(2);
	}
	
	//***Position and Heading control***//
	
	//Based on PD controller
	DistanceError = (DesiredDistance - ((LastTickCount_1+LastTickCount_2)/2)); //taking average of wheel 1 and 2 when driving straight
  //printf("E:%f\r \n", DistanceError);
	//printf("1:%d\r\n", LastTickCount_1);
  //printf("2:%d\r\n", LastTickCount_2);
  HeadingError = (DesiredHeading- ((LastTickCount_2-LastTickCount_1)/2)); //Subtracting both to take average when turning
  //printf("HeadingError:%f", HeadingError);
	DistancePDTerm = KPM*DistanceError + KDM*(DistanceError-LastDistanceError);
	HeadingPDTerm = KPD*HeadingError + KDD*(HeadingError-LastHeadingError); //Positive HeadingError is wheel 2, negative wheel 1
	DesiredSpeed_1 = Clamp(DistancePDTerm - HeadingPDTerm, -ClampRPM, ClampRPM);
	DesiredSpeed_2 = Clamp(DistancePDTerm + HeadingPDTerm, -ClampRPM, ClampRPM);
	LastDistanceError = DistanceError;
	LastHeadingError = HeadingError;
 
	//***Speed control for Motor 1***//
	
	RPMError_1 = DesiredSpeed_1 - LastRecordedSpeed_1;
	
	//Add RPM Error to integral term
	IntegralTerm_1 += RPMError_1;
	//Include Anti-windup for integral term
	IntegralTerm_1 = Clamp(IntegralTerm_1, -ClampPWM, ClampPWM); 
	
  //Compute UpdatedDutyCycle based on PI controller
	UpdatedDutyCycle_1 = RPM_P_GAIN*RPMError_1 + RPM_I_GAIN*IntegralTerm_1;
  
	//Include anti-windup for full term
	//Positive = Turn CW, Negative = Turn CCW (To update if necessary)
	UpdatedDutyCycle_1 = Clamp(UpdatedDutyCycle_1, -ClampPWM, ClampPWM); 
	//printf("1:%f \r\n", UpdatedDutyCycle_1);
	//Set Duty Cycle for Motor 1
	PWMSetDutyCycle_1(UpdatedDutyCycle_1); //REMOVED FOR TESTING
  //PWMSetDutyCycle_1(ClampPWM); //ADDED FOR TESTING
	 
	//***Speed control for Motor 2***//
	
	RPMError_2 = DesiredSpeed_2 - LastRecordedSpeed_2;
	
	//Add RPM Error to integral term
	IntegralTerm_2 += RPMError_2;
	//Include Anti-windup for integral term
	IntegralTerm_2 = Clamp(IntegralTerm_2, -ClampPWM, ClampPWM);
	
	//Compute UpdatedDutyCycle based on PI controller
	UpdatedDutyCycle_2 = RPM_P_GAIN*RPMError_2 + RPM_I_GAIN*IntegralTerm_2;
	//Include anti-windup for full term
	//Positive = Turn CW, Negative = Turn CCW (To update if necessary)
	UpdatedDutyCycle_2 = Clamp(UpdatedDutyCycle_2, -ClampPWM, ClampPWM);
	//printf("2:%f \r\n", UpdatedDutyCycle_2);
	//Set Duty Cycle for Motor 2
	PWMSetDutyCycle_2(UpdatedDutyCycle_2); //REMOVED FOR TESTING
  //PWMSetDutyCycle_2(ClampPWM); //ADDED FOR TESTING
	
	//***Desired geolocation monitor***//
	
	 //if Distance Error and Heading Error is within error bounds
	if((fabsf(DistanceError) <= MIN_ERROR) && (fabsf(HeadingError) <= MIN_ERROR) && Driving == true){
		printf("Amount of ticks executed: %d", LastTickCount_1);
		
		Driving = false;
		
		//post event to Master SM indicating that target has been reached
		ES_Event_t doneEvent;
		doneEvent.EventType = EV_MOVE_COMPLETED;
		PostMotorService(doneEvent);
	}
	
}

/****************************************************************************
 Function
    Drive_SpeedUpdateTimer_Init

 Parameters
 uint16_t : speed update delay in ms

 Returns
   void

 Description
   initializes periodic timer functionality for wide timer 5A
 Notes
   
 Author
   Sander Tonkens
****************************************************************************/
void Drive_SpeedUpdateTimer_Init(uint16_t updateTime){
	//initialization for Periodic Timer (Timer 5-A)
	printf("Update control loop \r\n");
	//start by enabling the clock to the timer (Wide Timer 5)
	HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R5;
	
	//Ensure Peripheral is ready
  while ((HWREG(SYSCTL_RCGCWTIMER) & SYSCTL_RCGCWTIMER_R5) != SYSCTL_RCGCWTIMER_R5)
  {}
	
	//make sure that timer (Timer A) is disabled before configuring
	HWREG(WTIMER5_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
	//set it up in 32bit wide (individual, not concatenated) mode
	HWREG(WTIMER5_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;
	
	//set up timer B in periodic mode so that it repeats the time-outs
	HWREG(WTIMER5_BASE+TIMER_O_TAMR) = (HWREG(WTIMER5_BASE+TIMER_O_TAMR)& ~TIMER_TAMR_TAMR_M)| TIMER_TAMR_TAMR_PERIOD;
	
	//set Periodic timeout rate
	HWREG(WTIMER5_BASE+TIMER_O_TAILR) = TICKS_PER_MS * updateTime; //***************
	
	//enable a local timeout interrupt
	HWREG(WTIMER5_BASE+TIMER_O_IMR) |= TIMER_IMR_TATOIM;
	
	//enable the Timer A in Wide Timer 1 interrupt in the NVIC
	//it is interrupt number 95 so appears in EN3 at bit 0  //***************************
	HWREG(NVIC_EN3) |= (BIT8HI);
	
	//make sure interrupts are enabled globally (Check whether this should be done in InitializeHardware)
	__enable_irq();
	
	//now kick the timer off by enabling it and enabling the timer to stall while stopped by the debugger
	HWREG(WTIMER5_BASE+TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);
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

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
