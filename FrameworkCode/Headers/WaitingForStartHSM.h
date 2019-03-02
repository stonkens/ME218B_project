/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef WaitingForStartHSM_H
#define WaitingForStartHSM_H

#include <stdbool.h>
#include "ES_Events.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum { WaitingForCompassAck, WaitingForCompassStart } WaitingForStartState_t ;


// Public Function Prototypes

ES_Event_t RunWaitingForStartSM( ES_Event_t CurrentEvent );
void StartWaitingForStartSM ( ES_Event_t CurrentEvent );
WaitingForStartState_t QueryWaitingForStartSM ( void );

#endif /*WaitingForStartHSM_H */

