/****************************************************************************
 Module
   Drive_Control.c

 Revision
   1.0.1

 Description
   Module for top-level control of driving

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for the framework and this service
*/
#include "DriveCommandModule.h"
#include "MotorSpeedControl.h"
#include "BITDEFS.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_pwm.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

/*----------------------------- Module Defines ----------------------------*/
// Readability defines:
#define BitsPerNibble 4

#define LEFT 1
#define RIGHT 2
#define BOTH 0

#define PI 3.14159265358979

#define DEG_RAD PI/180

#define STRAIGHT	0
#define TURN			1

#define ROTATION_RADIUS 5.12 //In inches (IF RECALIBRATED UPDATED BELOW)

#define TICKS_PER_INCHx10	606.38/4 //TO BE RECALIBRATED
#define TICKS_PER_DEGREEx100	720/4//Somewhere between 700 and 740 (approx)
//541.87/4 //TO BE RECALIBRATED

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// Data private to the module



/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
    Drive_Control_Init

 Parameters
   void

 Returns
   void

 Description
   initializes PWM functionality for PB4 & PB5
 Notes
   
 Author
   Sander Tonkens
****************************************************************************/
void Drive_Control_Init(void){
	
	Drive_SpeedControl_Init();
	
	//printf("\r\n Drive Control Initialized \r\n"); PRINTF REMOVED
	
}

/****************************************************************************
 Function
   DriveStraight

 Parameters
	float : desired distance (in inches*100)

 Returns
   void

 Description
   set new speed and distance for driving straight (Forward >0, Backward < 0)
 Notes
   
 Author
   Sander Tonkens
****************************************************************************/
void DriveStraight(float MaxRPM, float distancex100){
	Drive_SetClampRPM(MaxRPM);
	Drive_SetDistance(distancex100*TICKS_PER_INCHx10/1000);
}

/****************************************************************************
 Function
   DriveRotate

 Parameters
	float : desired angle (in degrees * 10)

 Returns
   void

 Description
   set new speed and angle for turning (left(+angle) or right(-angle))
 Notes
   
 Author
   Sander Tonkens
****************************************************************************/
void DriveRotate(float MaxRPM, float degreesx10){
	Drive_SetClampRPM(MaxRPM);
	Drive_SetHeading(degreesx10*TICKS_PER_DEGREEx100/1000);
}

void StopDrive(void)
{
  printf("Commanded motor to stop driving\r\n");
  Drive_Stop();
}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
