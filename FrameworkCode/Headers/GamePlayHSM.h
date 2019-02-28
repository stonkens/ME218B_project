/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef GamePlayHSM_H
#define GamePlayHSM_H

#include <stdbool.h>
#include "ES_Events.h"
#define BALL_COLLECTION_TIME 20000

// typedefs for the states
// State definitions for use with the query function
typedef enum { COLLECTING_GARBAGE, RECYCLING, LANDFILLING } GamePlayState_t ;


// Public Function Prototypes

ES_Event_t RunGamePlayHSM( ES_Event_t CurrentEvent );
void StartGamePlayHSM ( ES_Event_t CurrentEvent );
GamePlayState_t QueryGamePlayHSM ( void );

#endif /*GamePlayHSM_H */

