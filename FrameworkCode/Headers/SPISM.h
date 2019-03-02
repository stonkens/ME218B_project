/****************************************************************************

  Header file for template Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef SPISM_H
#define SPISM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

//Symbolic Constants
#define WAITING_FOR_START 0x00
#define RECYCLING		      0x01
#define GAME_OVER         0x02

#define RED               0x00
#define ORANGE            0x01
#define YELLOW            0x02
#define GREEN             0x03
#define BLUE              0x04
#define PINK              0x05


#define WEST_RECYCLE_FREQUENCY    1667
#define EAST_RECYCLE_FREQUENCY    2000
#define NORTH_LANDFILL_FREQUENCY  2000
#define SOUTH_LANDFILL_FREQUENCY  2000

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
  Registering, QueryTeamInfo, QueryingStatus, QueryingValue
}SPISM_t;

// Public Function Prototypes

bool InitSPISM(uint8_t Priority);
bool PostSPISM(ES_Event_t ThisEvent);
ES_Event_t RunSPISM(ES_Event_t ThisEvent);
uint8_t GetTeamInfoByte(void);
uint8_t GetGameStatusByte(void);
uint8_t GetValueByte(void);
uint16_t GetLeftRecycleFreq(void);
uint16_t GetRightRecycleFreq(void);
uint8_t GetAssignedColor(void);
uint8_t QueryWhichRecycle(void);
uint16_t GetAssignedFreq(void);
void InitSPI(void);


#endif /* SPISM_H */

