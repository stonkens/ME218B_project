
/****************************************************************************
 Module
   BumperChecker.c

 Revision
   1.0.1

 Description
   Event Checking Module, checks whether a limit switch on the bumper is triggered
   
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

// Project modules
#include "MasterHSM.h"

// This module
#include "BumperChecker.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.
*/

/*---------------------------- Module Variables ---------------------------*/
static uint8_t LastFrontSwitchState; //PD2 - assigning this as 1 for the event parameter
static uint8_t LastRightSwitchState; //PD3 - assigning this as 2 for the event parameter
static uint8_t LastBackSwitchState; //PD6 - assigning this as 3 for the event parameter
static uint8_t LastLeftSwitchState; //PD7 - assigning this as 4 for the event parameter
/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
    Check4Bumper
   
 Parameters
    None
    
 Returns
    bool: true if a tape event was detected
   
 Description
    Event checker for hitting any of the limit switches on the bumper 
		Pin state goes high when the button is pressed 
 Notes
 
 Author
    Kristine Chen, 02/05/2018, 18:11
****************************************************************************/

bool Check4Bump(void)
{
  //printf("checking bumper");
  ES_Event_t  ThisEvent;
  uint8_t     CurrentFrontSwitchState;
	uint8_t     CurrentRightSwitchState;
	uint8_t     CurrentBackSwitchState;
	uint8_t     CurrentLeftSwitchState;
  bool        ReturnVal = false;
  CurrentFrontSwitchState = HWREG(GPIO_PORTD_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT2HI; //PD2
	CurrentRightSwitchState = HWREG(GPIO_PORTD_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT3HI; //PD3
	CurrentBackSwitchState = HWREG(GPIO_PORTD_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT6HI; //PD6
	CurrentLeftSwitchState = HWREG(GPIO_PORTD_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT7HI; //PD7
	//printf("%d\r\n", CurrentLeftSwitchState);
  
  
  //printf("%d",CurrentFrontSwitchState);
  if (CurrentFrontSwitchState != LastFrontSwitchState) //front bumper PD2
  {
    //printf("%d",CurrentFrontSwitchState);
    ReturnVal = true;
    if (CurrentFrontSwitchState) // was 1, changing to 4...
    {
      printf("\r\nHit the front left bumper");
      ThisEvent.EventType = EV_BUMPER_HIT;
			ThisEvent.EventParam = 1; //was 1, change to 4
      PostMasterSM(ThisEvent); //don't know what service to post to yet 
    }
  }
	
	if (CurrentRightSwitchState != LastRightSwitchState) //right bumper PD3
  {
    ReturnVal = true;
    if (CurrentRightSwitchState)
    {
      printf("Hit the right front bumper\r\n");
      ThisEvent.EventType = EV_BUMPER_HIT;
			ThisEvent.EventParam = 2; 
      PostMasterSM(ThisEvent); //don't know what service to post to yet 
    }
  }
	
	if (CurrentBackSwitchState != LastBackSwitchState) //back bumper PD6
  {
    ReturnVal = true;
    if (CurrentBackSwitchState)
    {
      printf("Hit the back left bumper\r\n");
      ThisEvent.EventType = EV_BUMPER_HIT;
			ThisEvent.EventParam = 3; 
      PostMasterSM(ThisEvent); //don't know what service to post to yet 
    }
  }
	
	if (CurrentLeftSwitchState != LastLeftSwitchState) //left bumper // PD7
  {
    ReturnVal = true;
    if (CurrentLeftSwitchState)
    {
      printf("Hit the back right bumper\r\n");
      ThisEvent.EventType = EV_BUMPER_HIT;
			ThisEvent.EventParam = 4; 
      PostMasterSM(ThisEvent); //don't know what service to post to yet 
    }
  }
  LastFrontSwitchState = CurrentFrontSwitchState;
	LastRightSwitchState = CurrentRightSwitchState;
	LastBackSwitchState = CurrentBackSwitchState;
	LastLeftSwitchState = CurrentLeftSwitchState;
  return ReturnVal;
}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
