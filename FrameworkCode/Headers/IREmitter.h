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

void InitReloadIRCapture(void);
void ReloadIRInputCaptureResponse(void);
void InitEmitterPWM(void);
void UpdateEmitterPeriod(void);
void EnableEmitterInputCapture(void);
void DisableEmitterInputCapture(void);
void EnableEmitterPWM(void);
void DisableEmitterPWM(void);

#endif //IREmitter_H
