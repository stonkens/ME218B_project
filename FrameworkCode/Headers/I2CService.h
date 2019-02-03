/****************************************************************************

  Header file for sample I2C implementation 
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef I2CService_H
#define I2CService_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"    /* gets ES_Event_t type */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
  InitPState, Idle, Interpreting, Waiting4Busy, Waiting4Time
}I2CState_t;

// Public Function Prototypes

bool InitI2CService(uint8_t Priority);
bool PostI2CService(ES_Event_t ThisEvent);
ES_Event_t RunI2CService(ES_Event_t ThisEvent);
I2CState_t QueryI2CService(void);
bool IsI2C0Finished(void);

#endif /* I2CService_H */

