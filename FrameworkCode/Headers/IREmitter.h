/****************************************************************************
 Module
   IREmitter.h
****************************************************************************/

#ifndef IREmitter_H
#define IREmitter_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Types.h"
#include "ES_Configure.h"

void InitEmitterPWM(void);
void UpdateEmitterPeriod(uint32_t RecycleIRPeriod);
void EnableEmitterPWM(void);
void DisableEmitterPWM(void);

#endif //IREmitter_H
