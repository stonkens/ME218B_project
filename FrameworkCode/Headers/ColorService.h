/****************************************************************************

  Header file for sample I2C implementation 
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef ColorService_H
#define ColorService_H

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

bool InitColorService(uint8_t Priority);
bool PostColorService(ES_Event_t ThisEvent);
ES_Event_t RunColorService(ES_Event_t ThisEvent);

#endif /* ColorService_H */
