/****************************************************************************
 Module
   ColorService.c

 Revision
   1.0.1

 Description
   This is the first service for the Test Harness under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 10/26/17 18:26 jec     moves definition of ALL_BITS to ES_Port.h
 10/19/17 21:28 jec     meaningless change to test updating
 10/19/17 18:42 jec     removed referennces to driverlib and programmed the
                        ports directly
 08/21/17 21:44 jec     modified LED blink routine to only modify bit 3 so that
                        I can test the new new framework debugging lines on PF1-2
 08/16/17 14:13 jec      corrected ONE_SEC constant to match Tiva tick rate
 11/02/13 17:21 jec      added exercise of the event deferral/recall module
 08/05/13 20:33 jec      converted to test harness service
 01/16/12 09:58 jec      began conversion from TemplateFSM.c
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/

// Hardware
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"
#include "ES_Port.h"

// Our application
#include "I2CService.h"

// This module
//#include "TestHarnessService0.h"
#include "ColorService.h"
/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define FIVE_MS 5 
#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC / 2)
#define TWO_SEC (ONE_SEC * 2)
#define FIVE_SEC (ONE_SEC * 5)
/*------------------------------ Ball Presence ---------------------------------*/
#define DETECTION_THRES //some analog value 
/*------------------------------ Red ---------------------------------*/
#define RED_R_LOWER_THRES //percentage of the clear 
#define RED_R_UPPER_THRES
#define RED_G_LOWER_THRES //percentage of the clear 
#define RED_G_UPPER_THRES
#define RED_B_LOWER_THRES //percentage of the clear 
#define RED_B_UPPER_THRES
/*------------------------------ Orange ------------------------------*/
#define ORANGE_R_LOWER_THRES
#define ORANGE_R_UPPER_THRES
#define ORANGE_G_LOWER_THRES
#define ORANGE_G_UPPER_THRES
#define ORANGE_B_LOWER_THRES
#define ORANGE_B_UPPER_THRES
/*------------------------------ Yellow ------------------------------*/
#define YELLOW_R_LOWER_THRES
#define YELLOW_R_UPPER_THRES
#define YELLOW_G_LOWER_THRES
#define YELLOW_G_UPPER_THRES
#define YELLOW_B_LOWER_THRES
#define YELLOW_B_UPPER_THRES
/*------------------------------ Green -------------------------------*/
#define GREEN_R_LOWER_THRES
#define GREEN_R_UPPER_THRES
#define GREEN_G_LOWER_THRES
#define GREEN_G_UPPER_THRES
#define GREEN_B_LOWER_THRES
#define GREEN_B_UPPER_THRES
/*------------------------------ Blue --------------------------------*/
#define BLUE_R_LOWER_THRES
#define BLUE_R_UPPER_THRES
#define BLUE_G_LOWER_THRES
#define BLUE_G_UPPER_THRES
#define BLUE_B_LOWER_THRES
#define BLUE_B_UPPER_THRES
/*------------------------------ Pink --------------------------------*/
#define PINK_R_LOWER_THRES
#define PINK_R_UPPER_THRES
#define PINK_G_LOWER_THRES
#define PINK_G_UPPER_THRES
#define PINK_B_LOWER_THRES
#define PINK_B_UPPER_THRES
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static ColorState_t CurrentState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitColorService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any
     other required initialization for this service
 Notes

 Author
     J. Edward Carryer, 02/03/19, 10:46
****************************************************************************/
bool InitColorService(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService(MyPriority, ThisEvent) == true)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/****************************************************************************
 Function
     PostColorService

 Parameters
     ES_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
J. Edward Carryer, 02/03/19, 10:48
****************************************************************************/
bool PostColorService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunColorService 

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes

 Author
J. Edward Carryer, 02/03/19, 10:49
****************************************************************************/
ES_Event_t RunColorService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  switch (ThisEvent.EventType)
  { 
    case ES_INIT: // only respond to ES_Init
    {
      ES_Timer_InitTimer(COLOR_SENSE_TIMER, (FIVE_MS));
      puts("Sensing Colors:");
      printf("\rES_INIT received in Service %d\r\n", MyPriority);
      CurrentState = WaitingForBall;
    }
    break;
//    case ES_TIMEOUT:   // re-start timer & announce
//    {
//      uint16_t ClearValue;
//      uint16_t RedValue;
//      uint16_t GreenValue;
//      uint16_t BlueValue;
//      
//      ES_Timer_InitTimer(I2C_TEST_TIMER, ONE_SEC);
//      ClearValue = I2C_GetClearValue();
//      RedValue   = I2C_GetRedValue();
//      GreenValue = I2C_GetGreenValue();
//      BlueValue  = I2C_GetBlueValue();
//      
//      printf("Clr: %d, Red: %d, Grn: %d, Blu: %d, R%%: %.2f, G%% %.2f, B%% %.2f \r\n",
//          ClearValue, RedValue, GreenValue, BlueValue, 
//          ((float)RedValue*100/ClearValue),
//          ((float)GreenValue*100/ClearValue),
//          ((float)BlueValue*100/ClearValue));
//    }
//    break;
//    case ES_NEW_KEY:   // announce
//    {
//      printf("ES_NEW_KEY received with -> %c <- in Service 0\r\n",
//          (char)ThisEvent.EventParam);
//    }
//    break;
//    default:
//    {}
//     break;
  
    case WaitingForBall:
    {
      //query clear value 
      uint16_t ClearValue;
      ClearValue = I2C_GetClearValue();
      //If clear value not above the threshold
      if (ClearValue > DETECTION_THRES)
      {
        //CurrentState = BallDetected 
      }else{
      //re-init timer 
      //else
      }
    }
    break;
    
    case BallDetected:
    {
      //query RBG values 
      uint16_t ClearValue;
      uint16_t RedValue;
      uint16_t GreenValue;
      uint16_t BlueValue;
      ClearValue = I2C_GetClearValue();
      RedValue   = I2C_GetRedValue();
      GreenValue = I2C_GetGreenValue();
      BlueValue  = I2C_GetBlueValue();
      //if RED_R_LOWER_THRES< Rvalue <RED_R_UPPER_THRES
      //post ES_RED_DETECTED
      
      //post ES_ORANGE_DETECTED
      //post ES_YELLOW_DETECTED
      //post ES_GREEN_DETECTED
      //post ES_BLUE_DETECTED
      //post ES_PINK_DETECTED 
    }
    break;
    
  }
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

