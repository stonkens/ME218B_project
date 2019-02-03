/****************************************************************************

  Header file for Test Harness I2C Service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef TestHarnessI2C_H
#define TestHarnessI2C_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"

// Public Function Prototypes

bool InitTestHarnessI2C(uint8_t Priority);
bool PostTestHarnessI2C(ES_Event_t ThisEvent);
ES_Event_t RunTestHarnessI2C(ES_Event_t ThisEvent);

#endif /* TestHarnessI2C_H */

