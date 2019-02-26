/****************************************************************************

  Header file for template service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef DriveMotorPWM_H
#define DriveMotorPWM_H

#include "ES_Types.h"
#include <stdint.h>
#include "ES_Configure.h"
#include "ES_Framework.h"

// Public module defines
#define ADVANCE 0
#define REVERSE 1
#define CW 2
#define CCW 3

// Public Function Prototypes

void InitDriveMotorPWM(void);
void InitDriveMotorGPIO(void);
void PWMSetDutyCycle_1(int DutyCycle_1);
void PWMSetDutyCycle_2(int DutyCycle_2);
#endif /* DriveMotorPWM_H */
