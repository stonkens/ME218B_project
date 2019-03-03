
/****************************************************************************
 Module
   ColorService.c

 Revision
   1.0.1

 Description
   Event Checking Module, checks whether a ball is detected and what color it is 
   
   Local ReturnVal = False
   Set the previous clear and RBG values to the current clear & RBG values 
   Set the tolerance value to 20
   Read the new color sensor values 
   If the clear values are above or below the previous clear values by at least the tolerance band
      Determine what color the ball is 
      Set the event type to ES_BALL_DETECTED
      Post an event with the color parameter to BallProcessingSM 
      Set the ReturnVal to true
    Return the ReturnVal
    
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
#include "I2CService.h"

// This module
#include "ColorService.h"
#include "BallProcessingSM.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define FIVE_MS 5
#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC / 2)
#define TWO_SEC (ONE_SEC * 2)
#define FIVE_SEC (ONE_SEC * 5)
#define DETECTION_DELAY 200 //make sure a threshold triggers twice before posting an event 
/*------------------------------ Ball Presence ---------------------------------*/
#define DETECTION_THRES 7000//some analog value 
#define TOLERANCE 1
/*------------------------------ Event params for different colors ---------------------------------*/
#define ANY_BALL 0
#define RED 1 
#define ORANGE 2
#define YELLOW 3 
#define GREEN 4
#define BLUE 5
#define PINK 6 
/*------------------------------ Red ---------------------------------*/
//R = 60 -- 47
//B = 17 -- 21
//G = 19 -- 23
//Clear 
#define RED_R_TARGET 60 //percentage of the clear 
#define RED_B_TARGET 17
#define RED_G_TARGET 19 
#define RED_R_LOWER_THRES  RED_R_TARGET - TOLERANCE //percentage of the clear 
#define RED_R_UPPER_THRES  RED_R_TARGET + TOLERANCE 
#define RED_G_LOWER_THRES  RED_G_TARGET - TOLERANCE //percentage of the clear 
#define RED_G_UPPER_THRES  RED_G_TARGET + TOLERANCE
#define RED_B_LOWER_THRES  RED_B_TARGET - TOLERANCE //percentage of the clear 
#define RED_B_UPPER_THRES  RED_B_TARGET + TOLERANCE 
/*------------------------------ Orange ------------------------------*/
//R = 58 -- 43 -- 45 -- 50
//B = 14 -- 21 -- 20 -- 18
//G = 22 -- 27 -- 26 -- 25
//Clear 
#define ORANGE_R_TARGET 58 //percentage of the clear 
#define ORANGE_B_TARGET 14
#define ORANGE_G_TARGET 22 
#define ORANGE_R_LOWER_THRES ORANGE_R_TARGET - TOLERANCE 
#define ORANGE_R_UPPER_THRES ORANGE_R_TARGET + TOLERANCE
#define ORANGE_G_LOWER_THRES ORANGE_G_TARGET - TOLERANCE
#define ORANGE_G_UPPER_THRES ORANGE_G_TARGET + TOLERANCE 
#define ORANGE_B_LOWER_THRES ORANGE_B_TARGET - TOLERANCE
#define ORANGE_B_UPPER_THRES ORANGE_B_TARGET + TOLERANCE 
/*------------------------------ Yellow ------------------------------*/
//R = 43 
//B = 14
//G = 36
//Clear = 38063
#define YELLOW_R_TARGET 43
#define YELLOW_B_TARGET 14
#define YELLOW_G_TARGET 36 
#define YELLOW_R_LOWER_THRES YELLOW_R_TARGET - TOLERANCE
#define YELLOW_R_UPPER_THRES YELLOW_R_TARGET + TOLERANCE
#define YELLOW_G_LOWER_THRES YELLOW_G_TARGET - TOLERANCE 
#define YELLOW_G_UPPER_THRES YELLOW_G_TARGET + TOLERANCE 
#define YELLOW_B_LOWER_THRES YELLOW_B_TARGET - TOLERANCE
#define YELLOW_B_UPPER_THRES YELLOW_B_TARGET + TOLERANCE 
/*------------------------------ Green -------------------------------*/
//R = 30
//B = 17
//G = 44 
//Clear 
#define GREEN_R_TARGET 30
#define GREEN_B_TARGET 17
#define GREEN_G_TARGET 44 
#define GREEN_R_LOWER_THRES GREEN_R_TARGET - TOLERANCE
#define GREEN_R_UPPER_THRES GREEN_R_TARGET + TOLERANCE 
#define GREEN_G_LOWER_THRES GREEN_G_TARGET - TOLERANCE
#define GREEN_G_UPPER_THRES GREEN_G_TARGET + TOLERANCE 
#define GREEN_B_LOWER_THRES GREEN_B_TARGET - TOLERANCE 
#define GREEN_B_UPPER_THRES GREEN_B_TARGET + TOLERANCE  
/*------------------------------ Blue --------------------------------*/
//R = 13
//B = 48
//G = 32
//Clear 
#define BLUE_R_TARGET 13
#define BLUE_B_TARGET 48
#define BLUE_G_TARGET 32 
#define BLUE_R_LOWER_THRES  BLUE_R_TARGET - TOLERANCE 
#define BLUE_R_UPPER_THRES  BLUE_R_TARGET + TOLERANCE 
#define BLUE_G_LOWER_THRES  BLUE_G_TARGET - TOLERANCE 
#define BLUE_G_UPPER_THRES  BLUE_G_TARGET + TOLERANCE 
#define BLUE_B_LOWER_THRES  BLUE_B_TARGET - TOLERANCE 
#define BLUE_B_UPPER_THRES  BLUE_B_TARGET + TOLERANCE 
/*------------------------------ Pink --------------------------------*/
//R = 48
//B = 26
//G = 22
//Clear 
#define PINK_R_TARGET 48 
#define PINK_B_TARGET 26
#define PINK_G_TARGET 22 
#define PINK_R_LOWER_THRES  PINK_R_TARGET - TOLERANCE 
#define PINK_R_UPPER_THRES  PINK_R_TARGET + TOLERANCE 
#define PINK_G_LOWER_THRES  PINK_G_TARGET - TOLERANCE 
#define PINK_G_UPPER_THRES  PINK_G_TARGET + TOLERANCE 
#define PINK_B_LOWER_THRES  PINK_B_TARGET - TOLERANCE 
#define PINK_B_UPPER_THRES  PINK_B_TARGET + TOLERANCE 

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.
*/

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
static ColorState_t CurrentState; 
static uint8_t DetectCount = 0; 
static uint16_t LastRedValue; 
static uint16_t LastBlueValue; 
static uint16_t LastGreenValue; 
static uint16_t LastClearValue; 
static bool PostFlag = false; 
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
  //ThisEvent.EventType = ES_INIT;
  CurrentState = WaitingForBall;
  ES_Timer_InitTimer(COLOR_SENSE_TIMER,FIVE_MS);
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
    None
    
 Returns
    bool: true if a ball is detected
 Description
    Event checker for sensing the presence and color of a ball 
 Notes
 
 Author
    Kristine Chen, 02/05/2018, 18:11
****************************************************************************/
ES_Event_t RunColorService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  switch (CurrentState)
  { 
//    case ES_INIT: // only respond to ES_Init
//    {
//      ES_Timer_InitTimer(COLOR_SENSE_TIMER, (FIVE_MS));
//      puts("Sensing Colors:");
//      printf("\rES_INIT received in Service %d\r\n", MyPriority);
//      CurrentState = WaitingForBall;
//    }
//    break;
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
  
    case WaitingForBall:
    {
      if((ThisEvent.EventType == ES_TIMEOUT) && 
        (ThisEvent.EventParam == COLOR_SENSE_TIMER))
      {
        //query clear value 
        uint16_t ClearValue;
        ClearValue = I2C_GetClearValue();
        if (ClearValue >= DETECTION_THRES)
        {
            DetectCount++; //after the ball is detected for the first time, DetectCount will be 1 
                      //re-init timer 
            ES_Timer_InitTimer(COLOR_SENSE_TIMER, (FIVE_MS));
            CurrentState = WaitingForBall; 
            if(DetectCount>=2)
            {
              DetectCount = 0; //reset detect count 
                        //re-init timer 
              printf("Ball detected\r\n");
              ES_Timer_InitTimer(COLOR_SENSE_TIMER, (FIVE_MS));
              CurrentState = BallDetected;
            }
        }else{
          //re-init timer 
          ES_Timer_InitTimer(COLOR_SENSE_TIMER, (FIVE_MS));
          //printf("Ball NOT detected\r\n");
          CurrentState = WaitingForBall; 
      }
    }
  }
    break;
  
    case BallDetected:
    {
      //printf("Transitioned to ball detected \r\n");
      ES_Timer_InitTimer(COLOR_SENSE_TIMER, FIVE_MS);
      if((ThisEvent.EventType == ES_TIMEOUT) && 
        (ThisEvent.EventParam == COLOR_SENSE_TIMER))
      {
      //query RBG values 
      uint16_t CurrentClearValue;
      uint16_t CurrentRedValue;
      uint16_t CurrentGreenValue;
      uint16_t CurrentBlueValue;
      CurrentClearValue = I2C_GetClearValue();
      CurrentRedValue   = I2C_GetRedValue();
      CurrentGreenValue = I2C_GetGreenValue();
      CurrentBlueValue  = I2C_GetBlueValue();
       //easier to deal with in percentages 
      uint16_t CurrentRedPercentage; 
      uint16_t CurrentBluePercentage;
      uint16_t CurrentGreenPercentage; 
      CurrentRedPercentage = ((float)CurrentRedValue*100/CurrentClearValue); 
      CurrentGreenPercentage = ((float)CurrentGreenValue*100/CurrentClearValue),
      CurrentBluePercentage = ((float)CurrentBlueValue*100/CurrentClearValue);  
      
      ThisEvent.EventType = EV_BALL_DETECTED;
           if ((RED_R_LOWER_THRES <= CurrentRedPercentage && CurrentRedPercentage <= RED_R_UPPER_THRES) 
        && (RED_G_LOWER_THRES <=  CurrentGreenPercentage &&  CurrentGreenPercentage <= RED_G_UPPER_THRES)
      && (RED_B_LOWER_THRES <=  CurrentBluePercentage && CurrentBluePercentage <= RED_B_UPPER_THRES))
      {
        //if (LowerThres >= CurrentClearValue || CurrentClearValue >= HigherThres){
          //printf("Lower %d Current %d Upper %d \r\n",LowerThres, CurrentClearValue, HigherThres); 
          ThisEvent.EventParam = RED;   
          PostBallProcessingSM(ThisEvent);
          printf("RED\r\n");
          ES_Timer_InitTimer(COLOR_SENSE_TIMER,(FIVE_MS));
          CurrentState = Depositing; 
      }
      //Orange
      if ((ORANGE_R_LOWER_THRES <= CurrentRedPercentage && CurrentRedPercentage <= ORANGE_R_UPPER_THRES) 
        && (ORANGE_G_LOWER_THRES <=  CurrentGreenPercentage && CurrentGreenPercentage <= ORANGE_G_UPPER_THRES)
      && (ORANGE_B_LOWER_THRES <=  CurrentBluePercentage && CurrentBluePercentage <= ORANGE_B_UPPER_THRES))
      {
          //ThisEvent.EventType = ES_BALL_DETECTED;
          ThisEvent.EventParam = ORANGE;  
          PostBallProcessingSM(ThisEvent);
          printf("ORANGE\r\n");
          ES_Timer_InitTimer(COLOR_SENSE_TIMER,(FIVE_MS));
          CurrentState = Depositing;
      } 
      //Yellow
       if (YELLOW_R_LOWER_THRES <= CurrentRedPercentage && CurrentRedPercentage <= YELLOW_R_UPPER_THRES 
        && YELLOW_G_LOWER_THRES <=  CurrentGreenPercentage && CurrentGreenPercentage <= YELLOW_G_UPPER_THRES
      && YELLOW_B_LOWER_THRES <=  CurrentBluePercentage && CurrentBluePercentage <= YELLOW_B_UPPER_THRES)
      {
          //ThisEvent.EventType = ES_BALL_DETECTED;
          ThisEvent.EventParam = YELLOW;   
          PostBallProcessingSM(ThisEvent);
          printf("YELLOW\r\n");
          ES_Timer_InitTimer(COLOR_SENSE_TIMER,(FIVE_MS));
          CurrentState = Depositing; 
      }
      
      //Green 
       if (GREEN_R_LOWER_THRES <= CurrentRedPercentage && CurrentRedPercentage <= GREEN_R_UPPER_THRES 
        && GREEN_G_LOWER_THRES <=  CurrentGreenPercentage && CurrentGreenPercentage <= GREEN_G_UPPER_THRES
      && GREEN_B_LOWER_THRES <=  CurrentBluePercentage && CurrentBluePercentage <= GREEN_B_UPPER_THRES)
      {
          //ThisEvent.EventType = ES_BALL_DETECTED;
          ThisEvent.EventParam = GREEN;  
          PostBallProcessingSM(ThisEvent);
          printf("GREEN\r\n");
          ES_Timer_InitTimer(COLOR_SENSE_TIMER,(FIVE_MS));
          CurrentState = Depositing; 
      }
      
      //Blue 
        if (BLUE_R_LOWER_THRES <= CurrentRedPercentage && CurrentRedPercentage <= BLUE_R_UPPER_THRES 
        && BLUE_G_LOWER_THRES <=  CurrentGreenPercentage && CurrentGreenPercentage <= BLUE_G_UPPER_THRES
      && BLUE_B_LOWER_THRES <=  CurrentBluePercentage && CurrentBluePercentage <= BLUE_B_UPPER_THRES)
      {
          //ThisEvent.EventType = ES_BALL_DETECTED;
          ThisEvent.EventParam = BLUE;  
          PostBallProcessingSM(ThisEvent);
          printf("BLUE\r\n");
          ES_Timer_InitTimer(COLOR_SENSE_TIMER,(FIVE_MS));
          CurrentState = Depositing; 
      }
      //Pink 
        if (PINK_R_LOWER_THRES <= CurrentRedPercentage && CurrentRedPercentage <= PINK_R_UPPER_THRES 
        && PINK_G_LOWER_THRES <=  CurrentGreenPercentage && CurrentGreenPercentage <= PINK_G_UPPER_THRES
      && PINK_B_LOWER_THRES <=  CurrentBluePercentage && CurrentBluePercentage <= PINK_B_UPPER_THRES)
      {
          //ThisEvent.EventType = ES_BALL_DETECTED;
          ThisEvent.EventParam = PINK;  
          PostBallProcessingSM(ThisEvent);
          printf("PINK\r\n");
          ES_Timer_InitTimer(COLOR_SENSE_TIMER,(FIVE_MS));
          CurrentState = Depositing; 
      }
      
    }
  }
      break; 
      
    case Depositing:
    {
       ES_Timer_InitTimer(COLOR_SENSE_TIMER, FIVE_MS);
       if((ThisEvent.EventType == ES_TIMEOUT) && 
        (ThisEvent.EventParam == COLOR_SENSE_TIMER))
      {
      //printf("Depositing now\r\n"); 
      uint16_t CurrentClearValue;
      CurrentClearValue = I2C_GetClearValue();
      if(CurrentClearValue <= DETECTION_THRES){
        printf("Done depositing\r\n");
        ThisEvent.EventType = EV_BALL_GONE; 
        PostBallProcessingSM(ThisEvent);
        ES_Timer_InitTimer(COLOR_SENSE_TIMER,(FIVE_MS));
        CurrentState = WaitingForBall; 
      }
    }
  }
    break;
    }
  return ReturnEvent;
}

//void clearPostFlag(void){
//  PostFlag = false; 
//}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
