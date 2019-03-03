/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts

 ****************************************************************************/

#ifndef MasterHSM_H
#define MasterHSM_H

#include <stdbool.h>
#include "ES_Events.h"
#include "ES_Framework.h"

//Team indication defines
#define TEAM_NORTH 0
#define TEAM_SOUTH 1

// State definitions for use with the query function
typedef enum
{
  WaitingForStart, GamePlay, GameEnded, CollisionAvoidance
}MasterState_t;
// Public Function Prototypes

ES_Event_t RunMasterSM( ES_Event_t CurrentEvent );
void StartMasterSM ( ES_Event_t CurrentEvent );
bool PostMasterSM( ES_Event_t ThisEvent );
bool InitMasterSM ( uint8_t Priority );
uint8_t QueryTeam(void);
MasterState_t QueryMasterSM(void);
#endif /*MasterHSM_H */

