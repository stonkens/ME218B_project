/****************************************************************************
 Module
   IRDetector.h
****************************************************************************/

#ifndef IRDetector_H
#define IRDetector_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Types.h"
#include "ES_Configure.h"
#include "ES_Events.h"

void InitInputCapture(void);
void IR_ISR(void);
uint32_t IR_getPeriod(void);
void IR_disable(void);
void IR_enable(void);
bool IR_found(void);
#endif //IRDetector_H
