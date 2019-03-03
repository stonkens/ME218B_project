/****************************************************************************

  Header file for sample I2C implementation 
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef BallDumpingSM_H
#define BallDumpingSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"    /* gets ES_Event_t type */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
  Waiting2Dump, Landfilling, Recycling  
} DumpingState_t;

// Public Function Prototypes

bool InitBallDumpingSM(uint8_t Priority);
bool PostBallDumpingSM(ES_Event_t ThisEvent);
ES_Event_t RunBallDumpingSM(ES_Event_t ThisEvent);
void openRecyclingDoor(void);
void closeRecyclingDoor(void); 
void openLandfillDoor(void); 
void closeLandfillDoor(void);   

#endif /* BallDumpingSM_H */
