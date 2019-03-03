/****************************************************************************

  Header file for template service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef HarvesterService_H
#define HarvesterService_H

#include "ES_Types.h"
#include <stdint.h>
#include "ES_Configure.h"
#include "ES_Framework.h"

// Public Function Prototypes
/*------- Leaving these in case we change our minds & we don't want a simple service --------*/
bool InitDCMotorService(uint8_t Priority);
bool PostDCMotorService(ES_Event_t ThisEvent);
ES_Event_t RunDCMotorService(ES_Event_t ThisEvent);
/*-------------------------------------------------------------------------------------------*/
void StartHarvesterMotor(uint32_t DutyCycle); 
void StopHarvesterMotor(void);
void InitHarvesterMotor(void);
#endif /* HarvesterService_H */