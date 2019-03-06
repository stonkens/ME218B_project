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

  
void IRInitInputCapture(void);
void IR_ISR(void);

#define EAST_RECYCLING_PERIOD 500 //us
#define WEST_RECYCLING_PERIOD 600 //us
#define SOUTH_LANDFILL_PERIOD 700 //us
#define NORTH_LANDFILL_PERIOD 800 //us
#define WEST_RECYCLING_PERIOD_EMITTER 1000 //us
#define EAST_RECYCLING_PERIOD_EMITTER 200 //us

#define DETECTION_TOLERANCE 25 //us

//IR Disabler/Enabler
//IR sensor to be disabled after every localization
void IRDisableInterrupt(void);

//IR sensor to be enabled when entering localization state
void IREnableInterrupt(void);

//Currently commented: Acts as EventChecker
bool IR_found(void);

//Getter/setter functions to interface with OrientingSM
uint32_t IRGetRunCount(void);
void IRResetRunCount(void);
uint32_t IRGetPeriod(void);

//Activator function to find specific beacon
//(Interface with RecyclingSM and LandfillingSM)
void ActivateBeaconFinder(uint32_t Period);
void DisableBeaconFinder(void);
#endif //IRDetector_H
