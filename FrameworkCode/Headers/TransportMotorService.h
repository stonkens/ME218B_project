/****************************************************************************

  Header file for template service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef TransportMotorService_H
#define TransportMotorService_H

#include "ES_Types.h"
#include <stdint.h>
#include "ES_Configure.h"
#include "ES_Framework.h"

// Public Function Prototypes
/*------- Leaving these in case we change our minds & we don't want a simple service --------*/
bool InitTransportMotorService(uint8_t Priority);
bool PostTransportMotorService(ES_Event_t ThisEvent);
ES_Event_t RunTransportMotorService(ES_Event_t ThisEvent);
/*-------------------------------------------------------------------------------------------*/
void startTransportMotor(uint32_t DutyCycle); 
void stopTransportMotor(void);

#endif /* TransportMotorService_H */
