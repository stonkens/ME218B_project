
/****************************************************************************
 Module
   ReflectiveTapeChecker.c

 Revision
   1.0.1

 Description
   Event Checking Module, checks whether reflective tape is sensed 
   
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/06/19 17:50 ston    ?? 
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
// Hardware
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "termio.h"

// Specific Hardware

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"
#include "EventCheckers.h"

#include "MasterHSM.h"

// Project modules
#include "ADMulti.h"
// This module
#include "ReflectiveTapeChecker.h"

/*----------------------------- Module Defines ----------------------------*/
#define ENEMY_THRESHOLD 1000 
#define ENEMY_REPETITION_THRESHOLD 20 
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.
*/

/*---------------------------- Module Variables ---------------------------*/
static uint8_t LastInputState; 
static uint32_t AnalogValues[1];
static uint8_t EnemyCounter;
/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
    Check4Enemy
   
 Parameters
    None
    
 Returns
    bool: true if another bot was found 
   
 Description
    Event checker for detecing the presence of reflective tape on another robot  
 Notes
 
 Author
    Kristine Chen, 02/05/2018, 18:11
****************************************************************************/

bool Check4Enemy(void)
{
  ES_Event_t ThisEvent;
  ThisEvent.EventType = ES_NO_EVENT;
  
  // Default return value is false
  bool ReturnVal = false;
  
  // Sample analog port lines
  ADC_MultiRead(AnalogValues);
  
  // Get current state of range finder
  uint32_t CurrentInputState = AnalogValues[0];
  
  if ((CurrentInputState != LastInputState) && (CurrentInputState > 
  ENEMY_THRESHOLD))
  {
    
    EnemyCounter++;
    //printf("%d \r\n", EnemyCounter);
    if(EnemyCounter >= ENEMY_REPETITION_THRESHOLD)
    {
      ThisEvent.EventType = EV_BOT_DETECTED;
      ThisEvent.EventParam = CurrentInputState;

      PostMasterSM(ThisEvent);
      ReturnVal = true;
    }
  }
  else
  {
    EnemyCounter = 0;
  }
  // Update LastInputState
  LastInputState = CurrentInputState;
  
  return ReturnVal;  
}

void InitTapeReflectorHardware(void){    
  // Set PE0 as analog inputs  
  ADC_MultiInit(1);
    
} 



/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
