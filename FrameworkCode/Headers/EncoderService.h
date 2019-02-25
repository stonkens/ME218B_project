/****************************************************************************

  Header file for MotorService
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef EncoderService_H
#define EncoderService_H

#include "ES_Types.h"
#include "ES_Events.h"

// Public Function Prototypes
bool InitEncoderService(uint8_t Priority);
bool PostEncoderService(ES_Event_t ThisEvent);
ES_Event_t RunEncoderService(ES_Event_t ThisEvent);
uint32_t getEncoderPeriod(void); 

#endif /* EncoderService_H */
