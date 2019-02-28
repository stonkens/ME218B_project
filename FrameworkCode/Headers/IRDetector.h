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

void InitInputCapture(void);
void IR_ISR(void);
uint32_t IR_getPeriod(void);
void IR_disable(void);
void IR_enable(void);
#endif //IRDetector_H
