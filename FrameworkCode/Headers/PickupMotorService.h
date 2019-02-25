/****************************************************************************

  Header file for template service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef PickupMotorService_H
#define PickupMotorService_H

#include "ES_Types.h"
#include <stdint.h>
#include "ES_Configure.h"
#include "ES_Framework.h"

// Public Function Prototypes
/*------- Leaving these in case we change our minds & we don't want a simple service --------*/
bool InitPickupMotorService(uint8_t Priority);
bool PostPickupMotorService(ES_Event_t ThisEvent);
ES_Event_t RunPickupMotorService(ES_Event_t ThisEvent);
/*-------------------------------------------------------------------------------------------*/
void startPickupMotor(uint32_t DutyCycle); 
void stopPickupMotor(void);

#endif /* PickupMotorService_H */
