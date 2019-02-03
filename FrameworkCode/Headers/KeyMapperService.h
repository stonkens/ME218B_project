/****************************************************************************
 
  Header file for the KeyMapper service 

 ****************************************************************************/

#ifndef KeyMapperService_H
#define KeyMapperService_H

#include "ES_Types.h"

// Public Function Prototypes

bool InitKeyMapperService ( uint8_t Priority );
bool PostKeyMapperService( ES_Event_t ThisEvent );
ES_Event_t RunKeyMapperService( ES_Event_t ThisEvent );


#endif /* KeyMapperService_H */

