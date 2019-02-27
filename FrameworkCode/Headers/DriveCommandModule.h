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

void Drive_Control_Init(void);
void Drive_Straight(float distancex100);
void Drive_Turn(float degreesx10);


#endif //DriveCommandModule_H
