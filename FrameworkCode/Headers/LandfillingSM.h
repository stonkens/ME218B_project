/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef LandfillingSM_H
#define LandfillingSM_H

#include <stdbool.h>
#include "ES_Events.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum { Orienting2Landfill, Driving2Landfill, ApproachingLandfill, Preparing4Landfill, DumpingLandfill } LandfillingState_t ;


// Public Function Prototypes

ES_Event_t RunLandfillingSM( ES_Event_t CurrentEvent );
void StartLandfillingSM ( ES_Event_t CurrentEvent );
LandfillingState_t QueryLandfillingSM ( void );

#endif /*LandFilling_H */

