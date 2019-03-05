#ifndef TapeFollowingService_H
#define TapeFollowingService_H

#include <stdint.h>
#include <stdbool.h>

// Public Function Prototypes
typedef enum
{
  TapeFollowing,
  LostTape
}TapeState_t;

// Public Function Prototypes 
void StartTapeFollowingSM ( ES_Event_t CurrentEvent );
ES_Event_t RunTapeFollowingSM(ES_Event_t CurrentEvent);
#endif /* TapeFollowingService_H */
