/****************************************************************************

  Header file for template service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef PWM_H
#define PWM_H

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

void InitPWM(void);
void InitMotorGPIO(void);
void PWMSetDuty(uint8_t DutyCycleA, uint8_t DutyCycleB, uint8_t RotationDirection);
void PWMSetDutyCycleA(uint32_t DutyCycleA);
void PWMSetDutyCycleB(uint32_t DutyCycleB);
#endif /* PWM_H */
