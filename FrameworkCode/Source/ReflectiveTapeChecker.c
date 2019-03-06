
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
#define EQUATOR_HIGH_THRESHOLD 200 
#define EQUATOR_REPETITION_THRESHOLD 20 
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.
*/

/*---------------------------- Module Variables ---------------------------*/
static uint8_t LastInputState; 
static uint32_t AnalogValues[1];
static uint8_t EquatorCounter;
static bool enableFlag; 
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

bool Check4Equator(void)
{
  ES_Event_t ThisEvent;
  ThisEvent.EventType = ES_NO_EVENT;
  
  // Default return value is false
  bool ReturnVal = false;
  
  if(enableFlag==true){
		// Sample analog port lines
		ADC_MultiRead(AnalogValues);

		// Get current state of range finder
		uint32_t CurrentInputState = AnalogValues[0];

		//print analog input all the time
		//printf("%d CheckforEquator analog value\r\n", CurrentInputState);  
		
		//if smaller than higher threshold, increase counter
		if ((CurrentInputState != LastInputState) && (CurrentInputState < 
		EQUATOR_HIGH_THRESHOLD))
		{
			
			EquatorCounter++;
			//printf("%d \r\n", EnemyCounter);
			if(EquatorCounter >= EQUATOR_REPETITION_THRESHOLD)
			{
				//Change EV_Bot_Detected to ES_Tape_detected
				//ThisEvent.EventType = EV_BOT_DETECTED;
				//printf("EV_EQUATOR_DETECTED:");
				ThisEvent.EventType=EV_EQUATOR_DETECTED;
				ThisEvent.EventParam = CurrentInputState;
				PostMasterSM(ThisEvent);
				ReturnVal = true;
			}
		}
		else
		{
			EquatorCounter = 0;
		}
		// Update LastInputState
		LastInputState = CurrentInputState;
	
	}
	
  
  return ReturnVal;  
}

void InitTapeReflectorHardware(void){    
  // Set PE0 as analog inputs  
  ADC_MultiInit(1);
    
} 

void enableEquatorDetection(void){
  enableFlag = true; 
}

void disableEquatorDetection(void){
	enableFlag= false;
}
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
