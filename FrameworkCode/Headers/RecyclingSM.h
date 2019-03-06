/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef RecyclingSM_H
#define RecyclingSM_H

#include <stdbool.h>
#include "ES_Events.h"
#define BALL_COLLECTION_TIME 20000

// typedefs for the states
// State definitions for use with the query function
typedef enum { Orienting2LandfillR, Driving2LandfillR, Orienting2Recycle, Driving2Recycle, ApproachingRecycle, Preparing4Recycle, DumpingRecycle, RecoveringFromRecycle } RecyclingState_t ;


// Public Function Prototypes

ES_Event_t RunRecyclingSM( ES_Event_t CurrentEvent );
void StartRecyclingSM ( ES_Event_t CurrentEvent );
RecyclingState_t QueryRecyclingSM ( void );

#endif /*RecyclingSM_H */

