#ifndef TapeFollowingChecker_H
#define TapeFollowingChecker_H

#include <stdint.h>
#include <stdbool.h>

// Public Function Prototypes 
bool Check4TapeFollow(void);
void enableTapeFollow(void); 
void disableTapeFollow(void); 
void InitTapeHardware(void); 

#endif /* TapeFollowingChecker_H */
