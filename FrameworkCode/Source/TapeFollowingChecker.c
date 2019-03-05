
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
#include "MasterHSM.h"
// Specific Hardware

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"
#include "EventCheckers.h"
//#include "MasterHSM.h"

// Project modules

// This module
#include "TapeFollowingChecker.h"

#define MidTapeHI  BIT7HI
#define MidTapeLO  BIT7HI
#define MidTapePort HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) //also change intialization 


/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.
*/

/*---------------------------- Module Variables ---------------------------*/
static uint8_t LastTapeFollowLeft;
static uint8_t LastTapeFollowRight; 
static uint8_t LastTapeFollowMid;

//For test
static bool enableFlag = true;
//Test end 
/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
    Check4TapeFollow 
   
 Parameters
    None
    
 Returns
    bool: true if a tape event was recently detected 
 Description
    Event checker for sensing the presence of tape 
 Notes
 
 Author
    Kristine Chen, 02/05/2018, 18:11
****************************************************************************/

bool Check4TapeFollow(void)
{
  //printf("Tape Following Checker Now \r\n"); 
  ES_Event_t ThisEvent;
  uint8_t CurrentTapeFollowLeft;
  uint8_t CurrentTapeFollowRight;  
   uint8_t CurrentTapeFollowMid; 
  bool ReturnVal = false;   
  if (enableFlag == true)
  {
  //printf("Enable flag is true\r\n");
    //read the state of the tape 
  CurrentTapeFollowLeft = (HWREG(GPIO_PORTE_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT1HI); //PE1
  CurrentTapeFollowRight = (HWREG(GPIO_PORTE_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT2HI); //PE2
  CurrentTapeFollowMid = (MidTapePort & MidTapeHI);
  if (CurrentTapeFollowLeft != LastTapeFollowLeft)
  {
    ReturnVal = true;
    if ((CurrentTapeFollowLeft | BIT1LO) == BIT1LO) //signal is low when on the tape 
    {
<<<<<<< HEAD
      ThisEvent.EventType = EV_LEFT_TAPE_ON;
      ThisEvent.EventParam = ES_Timer_GetTime(); //time in ticks 
      //printf("Left tape on\r\n");
      PostMasterSM(ThisEvent);
    }
    if ((CurrentTapeFollowLeft & BIT1HI) == BIT1HI) //signal is high when off the tape 
    {
      ThisEvent.EventType = EV_LEFT_TAPE_OFF;
      ThisEvent.EventParam = ES_Timer_GetTime(); //time in ticks 
      //printf("Left tape off\r\n");
      PostMasterSM(ThisEvent);
=======
      ThisEvent.EventType = EV_NEW_TAPE;//EV_LEFT_TAPE_ON;
      ThisEvent.EventParam = 1;
      //ThisEvent.EventParam = ES_Timer_GetTime(); //time in ticks 
      printf("Left tape on\r\n");
      //PostMasterSM(ThisEvent);
    }
    if ((CurrentTapeFollowLeft & BIT1HI) == BIT1HI) //signal is high when off the tape 
    {
      ThisEvent.EventType = EV_NEW_TAPE;//EV_LEFT_TAPE_OFF;
      ThisEvent.EventParam = 2; //ES_Timer_GetTime(); //time in ticks 
      printf("Left tape off\r\n");
      //PostMasterSM(ThisEvent);
>>>>>>> TapeFollow
    }    
  }
    if (CurrentTapeFollowRight != LastTapeFollowRight)
  {
    ReturnVal = true;
    if ((CurrentTapeFollowRight | BIT2LO) == BIT2LO) //signal is low when on the tape 
    {
      ThisEvent.EventType = EV_NEW_TAPE;//EV_RIGHT_TAPE_ON;
      ThisEvent.EventParam = 3;//ES_Timer_GetTime(); //time in ticks 
      printf("Right tape on\r\n");
      //PostMasterSM(ThisEvent);
    }
    if ((CurrentTapeFollowRight & BIT2HI) == BIT2HI) //signal is high when off the tape 
    {
      ThisEvent.EventType = EV_NEW_TAPE;//EV_RIGHT_TAPE_OFF;
      ThisEvent.EventParam = 4;  //ES_Timer_GetTime(); 
      printf("Right tape off\r\n");
      //PostMasterSM(ThisEvent);
    }    
  }
  
  /*  if (CurrentTapeFollowMid != LastTapeFollowMid)
  {
    ReturnVal = true;
    if ((CurrentTapeFollowRight | MidTapeLO) == MidTapeLO) //signal is low when on the tape 
    {
      ThisEvent.EventType = EV_RIGHT_TAPE_ON;
      ThisEvent.EventParam = ES_Timer_GetTime(); //time in ticks 
      //printf("Right tape on\r\n");
      PostMasterSM(ThisEvent);
    }
    if ((CurrentTapeFollowRight & BIT2HI) == BIT2HI) //signal is high when off the tape 
    {
      ThisEvent.EventType = EV_RIGHT_TAPE_OFF;
      ThisEvent.EventParam = ES_Timer_GetTime(); 
      //printf("Right tape off\r\n");
      PostMasterSM(ThisEvent);
    }    
  }*/
  
  LastTapeFollowLeft = CurrentTapeFollowLeft; 
  LastTapeFollowRight = CurrentTapeFollowRight; 
  }
  return ReturnVal; 
}

void enableTapeFollow(void){
  enableFlag = true; 
}

void disableTapeFollow(void){
  enableFlag = false; 
}

void InitTapeHardware(void){
  //printf("Initializing tape hardware \r\n");
  //initialize PE1 & PE2 as digital inputs
     // Set bit 4 and enable port E
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R4;
  // Wait for peripheral E to be ready
  while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R4) != SYSCTL_PRGPIO_R4)
  {
    ;
  }
  // Set PE1, PE2 to usable pins 
  HWREG(GPIO_PORTE_BASE + GPIO_O_DEN) |= (BIT1HI | BIT2HI);
  // Set PE0, PE1, PE2 to input
  HWREG(GPIO_PORTE_BASE + GPIO_O_DIR) &= (BIT1LO & BIT2LO);
  // Set PB7 to usable pins
  HWREG(GPIO_PORTB_BASE + GPIO_O_DEN) |= (BIT7HI);
  // Set PE0, PE1, PE2 to input
  HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) &= (BIT7LO);
  disableTapeFollow();
}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
