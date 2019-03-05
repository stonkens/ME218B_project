/* include header files for hardware access */
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"

/* include header files for the framework */
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"

/* include header files for relevant services */
#include "LEDService.h"
#include "MasterHSM.h"

/* include header files for servo control */

/*----------------------------- Module Defines ----------------------------*/
#define LED_TIME 500 //time in ms 
#define PB0  HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT0HI //00000001 //Current PB0 pin state
#define PB1  HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT1HI //00000010//Current PB1 pin state
#define NORTH 0 
#define SOUTH 1 
/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static LEDState_t CurrentState;
static uint8_t      MyPriority;
static bool LEDOn = 0;
static bool nextLEDOn = 0;
/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
     InitLEDService 

 Parameters
     uint8_t : the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets the initial state of the LEDService 
 Notes

 Author
    Kristine Chen 

****************************************************************************/
bool InitLEDService(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  
  //Set PB0 & PB1 as a digital output
  HWREG(GPIO_PORTB_BASE + GPIO_O_DEN) |= (BIT0HI | BIT1HI);
  HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) |= (BIT0HI | BIT1HI);

  //Intially set them as low (this means they're on)
  HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT0LO;
  HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT1LO;  

  CurrentState = Waiting2Play; 
  ES_Timer_InitTimer(LED_TIMER,LED_TIME);

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
     PostLEDService

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
    Kristine Chen 

****************************************************************************/
bool PostLEDService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunLEDService

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
 
 Author
    Kristine Chen 

****************************************************************************/
ES_Event_t RunLEDService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  switch (CurrentState)
  {
    case Waiting2Play:
    {
      if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == LED_TIMER)
      {
        if(QueryTeam() == TEAM_NORTH)
        {
          if(LEDOn == false)
          {
            HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= (BIT0LO); //turn LED on 
            HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT1HI;
            uint8_t status = (HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT0HI);
            //HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT1LO; //turn LED on
            nextLEDOn = true;
          }
          else
          {
            HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT0HI;
            HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT1HI;
            //HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT0LO;
            //HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT1LO;
            uint8_t status = (HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT0HI);
            nextLEDOn = false;
          }
          LEDOn =nextLEDOn;
        }
        
        else if (QueryTeam() == TEAM_SOUTH)
        {
          if(LEDOn == false)
          {
            HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= (BIT1HI); //turn LED on 
            HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT1LO;
            //HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT1LO; //turn LED on
            nextLEDOn = true;
          }
          else
          {
            HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT0HI;
            HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT1HI;
            //HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT0LO;
            //HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT1LO;
            nextLEDOn = false;
          }
          LEDOn =nextLEDOn;
        }
        ES_Timer_InitTimer(LED_TIMER, LED_TIME);
      }
      else if(ThisEvent.EventType == EV_COMPASS_CLEANING_UP)
      {
        CurrentState = Playing; 
        if(QueryTeam() == TEAM_NORTH)
        {
          HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT0LO; //turn LED on 
          HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT1HI;          

        }
        else
        {
          HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT1LO; //turn LED on 
          HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT0HI;
        }

      }
    }
    break;
    
    case Playing:
    {
    
      if(ThisEvent.EventType == EV_COMPASS_GAME_OVER)
      {
        HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT0HI; //PB0 high
        HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT1HI; //PB1 high   
        CurrentState = GameOver; 
      }
    }
    break;

    case GameOver: //turn both LEDs off 
    {     
    }
    break;
  }
  return ReturnEvent;
}
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
