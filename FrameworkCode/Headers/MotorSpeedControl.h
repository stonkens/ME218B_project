/****************************************************************************
 Header
   Drive_SpeedControl.h

 Module Revision
   1.0.1
	 
****************************************************************************/

#ifndef Drive_SpeedControl_H
#define Drive_SpeedControl_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/****************************************************************************
	FUNCTION PROTOTYPES
****************************************************************************/

void Drive_SpeedControl_Init(void);
float Drive_GetRPM(uint8_t wheel);
void Drive_SetDistance(float newLimit);
void Drive_SetHeading(float newLimit);
void Drive_Stop(void);
void Drive_SetClampRPM(float newRPM);

void Drive_SpeedUpdateTimer_Init(uint16_t updateTime);



float GetMuV(void);
float GetDeltaV(void);
float GetMuError(void);
float GetDeltaError(void);

uint32_t GetRunCount(void);
uint16_t GetDuty1(void);
uint16_t GetDuty2(void);
uint32_t checkSpeed(void);


//***************************************************************************

#endif /* Drive_SpeedControl_H */
