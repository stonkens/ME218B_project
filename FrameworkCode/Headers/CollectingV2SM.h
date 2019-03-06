/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef CollectingV2SM_H
#define CollectingV2SM_H

#include <stdbool.h>
#include "ES_Events.h"

#define XTARGET 30 //Target in inches
#define YTARGET 30 //Target in inches

// typedefs for the states
// State definitions for use with the query function
typedef enum {Align2Landfill, Prepare4Harvesting, FindingTape, StraightDrive, TurnDrive} CollectingState_t ;


// Public Function Prototypes

ES_Event_t RunCollectingV2SM( ES_Event_t CurrentEvent );
void StartCollectingV2SM ( ES_Event_t CurrentEvent );
CollectingState_t QueryCollectingV2SM ( void );


#endif /*CollectingV2SM_H */

