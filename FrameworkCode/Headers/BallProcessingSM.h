/****************************************************************************

  Header file for sample I2C implementation 
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef BallProcessingSM_H
#define BallProcessingSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"    /* gets ES_Event_t type */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
  Waiting4Ball, Landfill, Recycle 
} ProcessingState_t;

// Public Function Prototypes

bool InitBallProcessingSM(uint8_t Priority);
bool PostBallProcessingSM(ES_Event_t ThisEvent);
ES_Event_t RunBallProcessingSM(ES_Event_t ThisEvent);
uint8_t QueryLandFillBalls(void);
uint8_t QueryRecycleBalls(void); 
//void openRecyclingDoor(void);
//void closeRecyclingDoor(void); 
//void openLandfillDoor(void); 
//void closeLandfillDoor(void);   
void sortToRecycling(void); 
void sortToLandfill(void);
void resetCookie(void); 
void setRecycleColor(uint8_t AssignedColor);
void clearRecycleBalls(void); 
void clearLandfillBalls(void); 

void AddRecycleBalls(void);
void AddLandFillBalls(void);

#endif /* BallProcessingSM_H */

