#ifndef TapeFollowingService_H
#define TapeFollowingService_H

#include <stdint.h>
#include <stdbool.h>

// Public Function Prototypes
typedef enum
{
  InitTapeState,
  LostTape,
  ForwardTape,
  RotateRight,
  RotateLeft,
  RotateRightFine,
  RotateLeftFine,
  IMPOSSIBRU

}TapeState;

// Public Function Prototypes 
bool InitTapeFollowingService(uint8_t Priority); 
bool PostTapeFollowingService(ES_Event_t ThisEvent);
ES_Event_t RunTapeFollowingService(ES_Event_t ThisEvent);
#endif /* TapeFollowingService_H */