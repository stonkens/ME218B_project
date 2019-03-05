/****************************************************************************

  Header file for template Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef SPISMV2_H
#define SPISMV2_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"
#include <stdint.h>
#include <stdbool.h>

//Symbolic Constants
#define WAITING_FOR_START 0x00
#define CLEANING_UP       0x01
#define GAME_OVER         0x02

#define RED               0x00
#define ORANGE            0x01
#define YELLOW            0x02
#define GREEN             0x03
#define BLUE              0x04
#define PINK              0x05

#define EAST_RECYCLE 1
#define WEST_RECYCLE 0

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
  Registering, QueryingTEAM, QueryingSTATUS, QueryingVAL
}SPISM_t;

// Public Function Prototypes

bool InitSPISM(uint8_t Priority);
bool PostSPISM(ES_Event_t ThisEvent);
ES_Event_t RunSPISM(ES_Event_t ThisEvent);
uint8_t GetTeamByte(void);
uint8_t GetStatusByte(void);
uint8_t GetValByte(void);
uint16_t GetLeftRecycleFreq(void);
uint16_t GetRightRecycleFreq(void);
uint8_t GetAssignedColor(void);
uint8_t QueryWhichRecycle(void);
uint16_t GetAssignedPeriod(void);
void InitSPI(void);

#endif /* SPISMV2_H */

