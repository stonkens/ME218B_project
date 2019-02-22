/****************************************************************************

  Header file for template service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef MotorService_H
#define MotorService_H

#include "ES_Types.h"
#include <stdint.h>
#include "ES_Configure.h"
#include "ES_Framework.h"

// Public Function Prototypes
typedef enum
{
  InitState,
  Forward,
  Backward
  //RegularOperation,
  //FindBoundary,
  //AlignBeacon
}MotorState;

bool InitMotorService(uint8_t Priority);
bool PostMotorService(ES_Event_t ThisEvent);
ES_Event_t RunMotorService(ES_Event_t ThisEvent);

uint32_t QueryMeasuredRPM(void);
uint32_t QuerySetRPM(void);

#endif /* MotorService_H */
