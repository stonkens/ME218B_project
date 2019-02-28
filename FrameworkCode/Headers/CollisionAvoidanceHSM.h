/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef CollisionAvoidanceHSM_H
#define CollisionAvoidanceHSM_H

#include <stdbool.h>
#include "ES_Events.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum { MOVING_BACKWARDS} CollisionAvoidanceState_t ;


// Public Function Prototypes

ES_Event_t RunCollisionAvoidanceHSM( ES_Event_t CurrentEvent );
void StartCollisionAvoidanceHSM ( ES_Event_t CurrentEvent );
CollisionAvoidanceState_t QueryCollisionAvoidanceHSM ( void );

#endif /*CollisionAvoidanceHSM_H */

