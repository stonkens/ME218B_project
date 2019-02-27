/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts

 ****************************************************************************/

#ifndef MasterHSM_H
#define MasterHSM_H

#include <stdbool.h>
#include "ES_Events.h"

// State definitions for use with the query function
typedef enum { WAITING_FOR_START, GAME_PLAY, GAME_ENDED, COLLISION_AVOIDANCE } MasterState_t ;

// Public Function Prototypes

ES_Event_t RunMasterSM( ES_Event_t CurrentEvent );
void StartMasterSM ( ES_Event_t CurrentEvent );
bool PostMasterSM( ES_Event_t ThisEvent );
bool InitMasterSM ( uint8_t Priority );

#endif /*MasterHSM_H */

