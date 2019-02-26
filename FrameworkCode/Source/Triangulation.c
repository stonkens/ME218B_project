/****************************************************************************
 Module
   Localization.c

 Revision
   1.0.1

 Description
   Returns coordinates based on angles from all beacons

 Notes
	
   
Author
   Sander Tonkens
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for the framework and this service
*/
#include "Localization.h"

#include <math.h> //for acos and atan

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_timer.h"
#include "inc/hw_nvic.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

/*----------------------------- Module Defines ----------------------------*/
// Readability defines:
#define pi 3.14159265358979

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/


/*---------------------------- Module Variables ---------------------------*/
// Data private to the module

static float Xf;
static float Yf;
static float theta;
static float OriginAngle;

static float X = 0; //Data to return, coordinates of bot
static float Y = 0;
static float Heading = 0;

static float XA; //Data of beacons for calculation
static float XB;
static float XC;
static float YA;
static float YB;
static float YC;

// Data for angles
static float ANG_A; // interior angles of traingle ABC
static float ANG_B;
static float ANG_C;
static float ANG_ALPHA;  // angles from bot perspective, between beacons B,C
static float ANG_BETA; // between beacons A,C
static float ANG_GAMMA; // between beacons A,B
static float ALPHA; //IN DEGREES
static float GAMMA;

//cotangents of angles
static float COT_A;
static float COT_B;
static float COT_C;
static float COT_ALPHA;
static float COT_BETA;
static float COT_GAMMA;

//Tienstra scalers
static float KA;
static float KB;
static float KC;
static float K;

//intermediate angles
static float firstAngle;
static float secondAngle;
static float thirdAngle;

/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
   Triangulate

 Parameters
	Angles to 3 of 4 beacons, ordered by their position and frequency:
	AngleA @ 2000Hz, AngleB @ 1667Hz, AngleC @ 1429Hz, and AngleD @ 1200Hz

 Returns
   Void. Saves X,Y, and Heading

 Description
   Application of Tienstra's Algorithm/Formula (see https://en.wikipedia.org/wiki/Tienstra_formula)

 Notes
   If one angle has not been observed, pass anglex = -1
   
 Author
   Sander Tonkens
****************************************************************************/
void Triangulate(float AngleA, float AngleB, float AngleC, float AngleD) {
	
	if (AngleA == -1) 
	{	
		//theta = 90;
		firstAngle = AngleB;
		secondAngle = AngleC;
		thirdAngle = AngleD;
		XA = -48;
		YA = -24;
		XB = -30;
		YB = -58.8;
		XC = 30;
		YC = 58.8;
		//Define ANG_A, ANG_B, ANG_C
		//Repeat for others
	} 

	else if (AngleB == -1) //
	{	
		//theta = 180;
		firstAngle = AngleC;
		secondAngle = AngleD;
		thirdAngle = AngleA;	

	}

	else if (AngleC == -1) 
	{	
		//theta = 270;
		firstAngle = AngleD;
		secondAngle = AngleA;
		thirdAngle = AngleB;
	} 

	else //AngleD is not considered
	{	
		//theta = 0;
		firstAngle = AngleA;
		secondAngle = AngleB;
		thirdAngle = AngleC;
	}
	
	 //define beacon locations
	//XA = -48;
	//YA = -48;
	//XB = 48;
	//YB = -48;
	//XC = 48;
	//YC = 48;
	

	ALPHA = thirdAngle - secondAngle + 360; //angles between beacons ASSUMES AngleA < AngleB AND
	GAMMA = secondAngle - firstAngle + 360; //WELL BEHAVED ANGLES!!!!!!!!!
	ANG_ALPHA = ALPHA*pi/180;
	ANG_GAMMA = GAMMA*pi/180;
	ANG_BETA=2*pi - ANG_ALPHA - ANG_GAMMA; //Alpha + Beta + Gamma = 360 degrees (ensures correct behaviour)
	ANG_A = pi/4;
	ANG_B = pi/2;
	ANG_C = pi/4;
	
	//Application of Tienstra's algorithm

	//COTANGENTS
	COT_A = 1/tan(ANG_A);
	COT_B = 1/tan(ANG_B);
	COT_C = 1/tan(ANG_C);
	COT_ALPHA = 1/tan(ANG_ALPHA);
	COT_BETA = 1/tan(ANG_BETA);
	COT_GAMMA = 1/tan(ANG_GAMMA);
	
	// calculate scalers
	KA = 1/(COT_A - COT_ALPHA);
	KB = 1/(COT_B - COT_BETA);
	KC = 1/(COT_C - COT_GAMMA);
	K  = KA + KB + KC;
	
	// calculate middle frame coordinates
	Xf = (KA*XA + KB*XB + KC*XC)/K;
	Yf = (KA*YA + KB*YB + KC*YC)/K;
	
	// calculate bot's position: Not required if using the actual location of the sensors
	//X = cos(theta*pi/180)*Xf - sin(theta*pi/180)*Yf + 48;
	//Y = sin(theta*pi/180)*Xf + cos(theta*pi/180)*Yf + 48;
	
	//CALCULATE direction
	OriginAngle = atan(Y/X)*180/pi;
	if(AngleA == -1){
		Heading = (360 - AngleB - atan(Y/(96-X))*180/pi)+360;
	}
	else{
		Heading = (180 - AngleA + OriginAngle)+360;
	}
	
}

/****************************************************************************
 Functions
   Get_X, Get_Y, and Get_Heading

 Parameters
	none

 Returns
   X or Y coordinates in inches, from center of board, respectively
		Heading, in degrees CCW from East

 Description
   Get Funct.
   
 Author
   John Alsterda
****************************************************************************/

float Get_X( void ) {
	return X;
}
float Get_Y( void ) {
	return Y;
}
float Get_Heading( void ) {
	return Heading;
}
float Get_OriginAngle( void ) {
	return OriginAngle;
}