/****************************************************************************
 Module
   IREmitter.h
****************************************************************************/

#ifndef DriveCommandModule_H
#define DriveCommandModule_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Types.h"
#include "ES_Configure.h"

#define STRAIGHT_SPEED	        200 //Max drive speed
#define TURNING_SPEED					  200 //Max rotation speed
#define LOCALIZATION_SPEED		  100 //Rotation speed when detecting IR
#define COLLISIONAVOID_SPEED    50
#define APPROACH_SPEED          20  //Approaching dumping station speed

#define COLLISIONAVOID_DISTANCE 1200


#define QUARTER_TURN            900
#define PLACEHOLDER_ANGLE       900

void Drive_Control_Init(void);
void DriveStraight(float MaxRPM, float distancex100);
void DriveRotate(float MaxRPM, float degreesx10);
void StopDrive(void);

#endif //DriveCommandModule_H
