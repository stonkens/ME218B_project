/****************************************************************************
 Header file for OrientingSM
 ****************************************************************************/

#ifndef OrientingSM_H
#define OrientingSM_H

#include <stdbool.h>
#include "ES_Events.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum {Measuring} OrientingState_t ;

// Public Function Prototypes
ES_Event_t RunOrientingSM ( ES_Event_t CurrentEvent );
void StartOrientingSM ( ES_Event_t CurrentEvent );
OrientingState_t QueryOrientingSM(void);

float GetCurrentXPosition(void);
float GetCurrentYPosition(void);
float GetCurrentHeading(void);
void ClearMeasurements(void);
void ResetOrientingRunCount(void);


#endif /*OrientingSM_H */
