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

#include <math.h> //for acos and atan

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gPIo.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_timer.h"
#include "inc/hw_nvic.h"
#include "driverlib/sysctl.h"
#include "driverlib/PIn_map.h"
#include "driverlib/gPIo.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"


#include "Triangulation.h"
/*----------------------------- Module Defines ----------------------------*/
// Readability defines:
#define PI 3.14159265358979

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/


/*---------------------------- Module Variables ---------------------------*/
// Data private to the module

static float Xf;
static float Yf;
static float theta;
static float XRelative;
static float YRelative;
static float RelativeTheta;

static float X = 0; //Data to return, coordinates of bot
static float Y = 0;
static float Heading = 0;

static float X1; //Data of beacons for calculation
static float X2;
static float X3;
static float Y1;
static float Y2;
static float Y3;

// Data for angles
static float Angle1; // interior angles of traingle ABC
static float Angle2;
static float Angle3;
static float AngA;
static float AngB;
static float AngC;
static float Alpha;
static float Beta;
static float Gamma;
static float AngAlpha;  // angles from bot perspective, between beacons B,C
static float AngBeta; // between beacons A,C
static float AngGamma; // between beacons A,B


//cotangents of angles
static float COT_A;
static float COT_B;
static float COT_C;
static float COT_Alpha;
static float COT_Beta;
static float COT_Gamma;

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
    
    //Not using Angle to West Recycling Center
    Angle1 = AngleB;
    Angle2 = AngleC;
    Angle3 = AngleD;
    X1 = 48;
    Y1 = -12;
    X2 = -30;
    Y2 = -58.8;
    X3 = 30;
    Y3 = 58.8;
    
    AngA = 1.862253121;
    AngB = 0.558599315;
    AngC = 0.720740217;
    Alpha = Angle3 - Angle2;
    if (Alpha <0)
    {
      Alpha += 360; 
    }
    Beta = Angle1 - Angle3;
    if (Beta < 0)
    {
      Beta += 360;
    }
    Gamma = Angle2 - Angle1;
    if (Gamma < 0)
    {
      Gamma += 360;
    }
    AngAlpha = Alpha*PI/180;
    AngBeta = Beta*PI/180;
    AngGamma = Gamma*PI/180;
    
    //Application of Tienstra's algorithm

    //COTANGENTS
    COT_A = 1/tan(AngA);
    COT_B = 1/tan(AngB);
    COT_C = 1/tan(AngC);
    COT_Alpha = 1/tan(AngAlpha);
    COT_Beta = 1/tan(AngBeta);
    COT_Gamma = 1/tan(AngGamma);
    
    // calculate scalers
    KA = 1/(COT_A - COT_Alpha);
    KB = 1/(COT_B - COT_Beta);
    KC = 1/(COT_C - COT_Gamma);
    K  = KA + KB + KC;
    
    // calculate middle frame coordinates
    Xf = (KA*X1 + KB*X2 + KC*X3)/K;
    Yf = (KA*Y1 + KB*Y2 + KC*Y3)/K;
    
    //Calculate heading
    XRelative = X1 - Xf;
    YRelative = Y1 - Yf;
    
    
    RelativeTheta = atan2((YRelative), (XRelative));
    
    if(((AngleB - RelativeTheta) >= 0))
    {
      Heading = (AngleB - RelativeTheta)*180/PI;
      
      if (Heading < 0) 
      {
        Heading = - Heading;
      }
    }
    else
    {
      Heading = (AngleB - RelativeTheta + 2*PI) * 180/PI;
    }
	} 

	else if (AngleB == -1) //
	{
    //Not using Angle to East Recycling Center
    Angle1 = AngleC;
    Angle2 = AngleD;
    Angle3 = AngleA;
    X1 = -30;
    Y1 = -58.8;
    X2 = 30;
    Y2 = 58.8;
    X3 = -48;
    Y3 = 12;
    
    AngA = 0.720740217;
    AngB = 0.558599315;
    AngC = 1.862253121;
    Alpha = Angle3 - Angle2;
    if (Alpha <0)
    {
      Alpha += 360; 
    }
    Beta = Angle1 - Angle3;
    if (Beta < 0)
    {
      Beta += 360;
    }
    Gamma = Angle2 - Angle1;
    if (Gamma < 0)
    {
      Gamma += 360;
    }
    AngAlpha = Alpha*PI/180;
    AngBeta = Beta*PI/180;
    AngGamma = Gamma*PI/180;
    
    //Application of Tienstra's algorithm

    //COTANGENTS
    COT_A = 1/tan(AngA);
    COT_B = 1/tan(AngB);
    COT_C = 1/tan(AngC);
    COT_Alpha = 1/tan(AngAlpha);
    COT_Beta = 1/tan(AngBeta);
    COT_Gamma = 1/tan(AngGamma);
    
    // calculate scalers
    KA = 1/(COT_A - COT_Alpha);
    KB = 1/(COT_B - COT_Beta);
    KC = 1/(COT_C - COT_Gamma);
    K  = KA + KB + KC;
    
    // calculate middle frame coordinates
    Xf = (KA*X1 + KB*X2 + KC*X3)/K;
    Yf = (KA*Y1 + KB*Y2 + KC*Y3)/K;
    
    //Calculate heading
    XRelative = X3 - Xf;
    YRelative = Y3 - Yf;
    
    //HEADING POSSIBLY TO BE UPDATED
    RelativeTheta = atan2((YRelative), (XRelative));
    
    if(((AngleA - RelativeTheta) >= 0))
    {
      Heading = (AngleA - RelativeTheta)*180/PI;
      
      if (Heading < 0) 
      {
        Heading = - Heading;
      }
    }
    else
    {
      Heading = (AngleA - RelativeTheta + 2*PI) * 180/PI;
    }
	} 

	else if (AngleC == -1) 
	{	
    
    //Not using Angle to South Landfill
    Angle1 = AngleD;
    Angle2 = AngleA;
    Angle3 = AngleB;
    X1 = 30;
    Y1 = 58.8;
    X2 = -48;
    Y2 = 12;
    X3 = 48;
    Y3 = -12;
    
    AngA = 1.279339532;
    AngB = 0.785398163;
    AngC = 1.076854958;
    Alpha = Angle3 - Angle2;
    if (Alpha <0)
    {
      Alpha += 360; 
    }
    Beta = Angle1 - Angle3;
    if (Beta < 0)
    {
      Beta += 360;
    }
    Gamma = Angle2 - Angle1;
    if (Gamma < 0)
    {
      Gamma += 360;
    }
    AngAlpha = Alpha*PI/180;
    AngBeta = Beta*PI/180;
    AngGamma = Gamma*PI/180;
    
    //Application of Tienstra's algorithm

    //COTANGENTS
    COT_A = 1/tan(AngA);
    COT_B = 1/tan(AngB);
    COT_C = 1/tan(AngC);
    COT_Alpha = 1/tan(AngAlpha);
    COT_Beta = 1/tan(AngBeta);
    COT_Gamma = 1/tan(AngGamma);
    
    // calculate scalers
    KA = 1/(COT_A - COT_Alpha);
    KB = 1/(COT_B - COT_Beta);
    KC = 1/(COT_C - COT_Gamma);
    K  = KA + KB + KC;
    
    // calculate middle frame coordinates
    Xf = (KA*X1 + KB*X2 + KC*X3)/K;
    Yf = (KA*Y1 + KB*Y2 + KC*Y3)/K;
    
    //Calculate heading
    XRelative = X1 - Xf;
    YRelative = Y1 - Yf;
    
    
    RelativeTheta = atan2((YRelative), (XRelative));
    
    if(((AngleB - RelativeTheta) >= 0))
    {
      Heading = (AngleB - RelativeTheta)*180/PI;
      
      if (Heading < 0) 
      {
        Heading = - Heading;
      }
    }
    else
    {
      Heading = (AngleB - RelativeTheta + 2*PI) * 180/PI;
    }
	}

	else //AngleD is not considered
	{	
    //Not using Angle to North Landfill
    Angle1 = AngleA;
    Angle2 = AngleB;
    Angle3 = AngleC;
    X1 = -48;
    Y1 = 12;
    X2 = 48;
    Y2 = -12;
    X3 = -30;
    Y3 = -58.8;
    
    AngA = 1.076854958;
    AngB = 0.785398163;
    AngC = 1.279339532;
    Alpha = Angle3 - Angle2;
    if (Alpha <0)
    {
      Alpha += 360; 
    }
    Beta = Angle1 - Angle3;
    if (Beta < 0)
    {
      Beta += 360;
    }
    Gamma = Angle2 - Angle1;
    if (Gamma < 0)
    {
      Gamma += 360;
    }
    AngAlpha = Alpha*PI/180;
    AngBeta = Beta*PI/180;
    AngGamma = Gamma*PI/180;
    
    //Application of Tienstra's algorithm

    //COTANGENTS
    COT_A = 1/tan(AngA);
    COT_B = 1/tan(AngB);
    COT_C = 1/tan(AngC);
    COT_Alpha = 1/tan(AngAlpha);
    COT_Beta = 1/tan(AngBeta);
    COT_Gamma = 1/tan(AngGamma);
    
    // calculate scalers
    KA = 1/(COT_A - COT_Alpha);
    KB = 1/(COT_B - COT_Beta);
    KC = 1/(COT_C - COT_Gamma);
    K  = KA + KB + KC;
    
    // calculate middle frame coordinates
    Xf = (KA*X1 + KB*X2 + KC*X3)/K;
    Yf = (KA*Y1 + KB*Y2 + KC*Y3)/K;
    
    //Calculate heading
    XRelative = X1 - Xf;
    YRelative = Y1 - Yf;
    
    
    RelativeTheta = atan2((YRelative), (XRelative));
    
    if(((AngleB - RelativeTheta) >= 0))
    {
      Heading = (AngleB - RelativeTheta)*180/PI;
      
      if (Heading < 0) 
      {
        Heading = - Heading;
      }
    }
    else
    {
      Heading = (AngleB - RelativeTheta + 2*PI) * 180/PI;
    }
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
   Sander Tonkens
****************************************************************************/

float QueryXCoordinate( void ) {
	return X;
}
float QueryYCoordinate( void ) {
	return Y;
}
float QueryHeading( void ) {
	return Heading;
}
