/****************************************************************************
 
  Header file for the KeyMapper service 

 ****************************************************************************/

#ifndef KeyMapperService_H
#define KeyMapperService_H

#include "ES_Types.h"
#include "ES_Framework.h"
// Public Function Prototypes

bool InitKeyMapperService ( uint8_t Priority );
bool PostKeyMapperService( ES_Event_t ThisEvent );
ES_Event_t RunKeyMapperService( ES_Event_t ThisEvent );
//TapeState_t QueryTapeSM(void);
//GameControlState_t QueryGameControl(void);

uint8_t HowManyTrashBalls(void);
uint8_t HowManyRecyclingBalls(void);
uint8_t QueryRecycleBalls(void);
uint8_t QueryLandFillBalls(void);
#endif /* KeyMapperService_H */

