
/****************************************************************************
 Module
   ColorChecker.c

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
#include "ColorChecker.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define FIVE_MS 5 
#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC / 2)
#define TWO_SEC (ONE_SEC * 2)
#define FIVE_SEC (ONE_SEC * 5)
#define DETECTION_DELAY 200 //make sure a threshold triggers twice before posting an event 
/*------------------------------ Ball Presence ---------------------------------*/
#define DETECTION_THRES 1040//some analog value 
#define TOLERANCE 20
/*------------------------------ Event params for different colors ---------------------------------*/
#define ANY_BALL 0
#define RED 1 
#define ORANGE 2
#define YELLOW 3 
#define GREEN 4
#define BLUE 5
#define PINK 6 
/*------------------------------ Red ---------------------------------*/
#define RED_R_LOWER_THRES 0 //percentage of the clear 
#define RED_R_UPPER_THRES 0
#define RED_G_LOWER_THRES 0 //percentage of the clear 
#define RED_G_UPPER_THRES 0
#define RED_B_LOWER_THRES 0 //percentage of the clear 
#define RED_B_UPPER_THRES 0
/*------------------------------ Orange ------------------------------*/
#define ORANGE_R_LOWER_THRES 0 
#define ORANGE_R_UPPER_THRES 0
#define ORANGE_G_LOWER_THRES 0
#define ORANGE_G_UPPER_THRES 0
#define ORANGE_B_LOWER_THRES 0
#define ORANGE_B_UPPER_THRES 0
/*------------------------------ Yellow ------------------------------*/
#define YELLOW_R_LOWER_THRES 0
#define YELLOW_R_UPPER_THRES 0
#define YELLOW_G_LOWER_THRES 0
#define YELLOW_G_UPPER_THRES 0
#define YELLOW_B_LOWER_THRES 0
#define YELLOW_B_UPPER_THRES 0
/*------------------------------ Green -------------------------------*/
#define GREEN_R_LOWER_THRES 0
#define GREEN_R_UPPER_THRES 0
#define GREEN_G_LOWER_THRES 0
#define GREEN_G_UPPER_THRES 0
#define GREEN_B_LOWER_THRES 0
#define GREEN_B_UPPER_THRES 0
/*------------------------------ Blue --------------------------------*/
#define BLUE_R_LOWER_THRES  0
#define BLUE_R_UPPER_THRES  0
#define BLUE_G_LOWER_THRES  0
#define BLUE_G_UPPER_THRES  0
#define BLUE_B_LOWER_THRES  0
#define BLUE_B_UPPER_THRES  0
/*------------------------------ Pink --------------------------------*/
#define PINK_R_LOWER_THRES  0
#define PINK_R_UPPER_THRES  0
#define PINK_G_LOWER_THRES  0
#define PINK_G_UPPER_THRES  0
#define PINK_B_LOWER_THRES  0
#define PINK_B_UPPER_THRES  0 

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.
*/

/*---------------------------- Module Variables ---------------------------*/
static uint8_t DetectCount = 0; 
static uint16_t LastRedValue; 
static uint16_t LastBlueValue; 
static uint16_t LastGreenValue; 
static uint16_t LastClearValue; 
/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
    Check4Color
   
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

bool Check4Color(void)
{
  printf("Checking color\r\n");
  ES_Event_t  ThisEvent;
  ThisEvent.EventType = ES_NO_EVENT; 
  bool ReturnVal = false; 
  //query RBG values 
  uint16_t CurrentClearValue;
  uint16_t CurrentRedValue;
  uint16_t CurrentGreenValue;
  uint16_t CurrentBlueValue;
  //easier to deal with in percentages 
  uint16_t CurrentRedPercentage; 
  uint16_t CurrentBluePercentage;
  uint16_t CurrentGreenPercentage; 
  
  CurrentClearValue = I2C_GetClearValue();
  CurrentRedValue   = I2C_GetRedValue();
  CurrentGreenValue = I2C_GetGreenValue();
  CurrentBlueValue  = I2C_GetBlueValue();
  
  CurrentRedPercentage = ((float)CurrentRedValue*100/CurrentClearValue); 
  CurrentGreenPercentage = ((float)CurrentGreenValue*100/CurrentClearValue),
  CurrentBluePercentage = ((float)CurrentBlueValue*100/CurrentClearValue);
  
  if((CurrentClearValue + TOLERANCE >= DETECTION_THRES)||(CurrentClearValue - TOLERANCE >= DETECTION_THRES))
  {
    DetectCount++; 
    if (DetectCount >= 2){
      //ball has officially been detected 
      ThisEvent.EventType = ES_BALL_DETECTED;
      ThisEvent.EventParam = ANY_BALL; // ONLY FOR NOW 
      ReturnVal = true; 
      //determine the color
      //set the param according to the color of the ball 
      
      printf("Red: %d, Blue: %d, Green: %d \r\n",CurrentRedPercentage,CurrentBluePercentage,CurrentGreenPercentage);
      
      //Red 
      if (RED_R_LOWER_THRES <= CurrentRedPercentage <= RED_R_UPPER_THRES 
        && RED_G_LOWER_THRES <=  CurrentGreenPercentage <= RED_G_UPPER_THRES
      && RED_B_LOWER_THRES <=  CurrentBluePercentage <= RED_B_UPPER_THRES)
      {
          ThisEvent.EventParam = RED; 
          printf("RED\r\n");
      }
      //Orange
      if (ORANGE_R_LOWER_THRES <= CurrentRedPercentage <= ORANGE_R_UPPER_THRES 
        && ORANGE_G_LOWER_THRES <=  CurrentGreenPercentage <= ORANGE_G_UPPER_THRES
      && ORANGE_B_LOWER_THRES <=  CurrentBluePercentage <= ORANGE_B_UPPER_THRES)
      {
          ThisEvent.EventParam = ORANGE; 
          printf("ORANGE\r\n");
      } 
      
      //Yellow
       if (YELLOW_R_LOWER_THRES <= CurrentRedPercentage <= YELLOW_R_UPPER_THRES 
        && YELLOW_G_LOWER_THRES <=  CurrentGreenPercentage <= YELLOW_G_UPPER_THRES
      && YELLOW_B_LOWER_THRES <=  CurrentBluePercentage <= YELLOW_B_UPPER_THRES)
      {
          ThisEvent.EventParam = YELLOW; 
          printf("YELLOW\r\n");
      }
      
      //Green 
       if (GREEN_R_LOWER_THRES <= CurrentRedPercentage <= GREEN_R_UPPER_THRES 
        && GREEN_G_LOWER_THRES <=  CurrentGreenPercentage <= GREEN_G_UPPER_THRES
      && GREEN_B_LOWER_THRES <=  CurrentBluePercentage <= GREEN_B_UPPER_THRES)
      {
        ThisEvent.EventParam = GREEN; 
        printf("GREEN\r\n");
      }
      
      //Blue 
        if (BLUE_R_LOWER_THRES <= CurrentRedPercentage <= BLUE_R_UPPER_THRES 
        && BLUE_G_LOWER_THRES <=  CurrentGreenPercentage <= BLUE_G_UPPER_THRES
      && BLUE_B_LOWER_THRES <=  CurrentBluePercentage <= BLUE_B_UPPER_THRES)
      {
        ThisEvent.EventParam = BLUE; 
        printf("BLUE\r\n");
      }
      //Pink 
                   if (PINK_R_LOWER_THRES <= CurrentRedPercentage <= PINK_R_UPPER_THRES 
        && PINK_G_LOWER_THRES <=  CurrentGreenPercentage <= PINK_G_UPPER_THRES
      && PINK_B_LOWER_THRES <=  CurrentBluePercentage <= PINK_B_UPPER_THRES)
      {
        ThisEvent.EventParam = PINK; 
        printf("PINK\r\n");
      }
      
//       printf("Clr: %d, Red: %d, Grn: %d, Blu: %d, R%%: %.2f, G%% %.2f, B%% %.2f \r\n",
//          ClearValue, RedValue, GreenValue, BlueValue, 
//          ((float)RedValue*100/ClearValue),
//          ((float)GreenValue*100/ClearValue),
//          ((float)BlueValue*100/ClearValue));

      //post ball detected event to ball processing state machine 
      //PostBallProcessingSM(ThisEvent.EventType, ThisEvent.EventParam);
      //reset the DetectCount 
      DetectCount =0; 
    }
  }
  LastClearValue = CurrentClearValue;
	LastRedValue = CurrentRedValue;
	LastBlueValue = CurrentBlueValue;
	LastGreenValue = CurrentGreenValue;
  return ReturnVal;
}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
