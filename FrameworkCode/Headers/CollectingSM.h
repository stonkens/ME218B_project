/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef CollectingSM_H
#define CollectingSM_H

#include <stdbool.h>
#include "ES_Events.h"

#define XTARGET 30 //Target in inches
#define YTARGET 30 //Target in inches

// typedefs for the states
// State definitions for use with the query function
typedef enum { Orienting, Driving2Target, Roaming} CollectingState_t ;


// Public Function Prototypes

ES_Event_t RunCollectingSM( ES_Event_t CurrentEvent );
void StartCollectingSM ( ES_Event_t CurrentEvent );
CollectingState_t QueryCollectingSM ( void );

//Setter function for the OrientingSM
void SetPositionAwareness(bool PositionStatus);
bool QueryPositionAwareness(void);
#endif /*CollectingSM_H */
