/****************************************************************************
 Module
   TemplateFSM.c

 Revision
   1.0.1

 Description
   This is a template file for implementing flat state machines under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 11:12 jec      revisions for Gen2 framework
 11/07/11 11:26 jec      made the queue static
 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTemplateSM()
 10/23/11 18:20 jec      began conversion from SMTemplate.c (02/20/07 rev)
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "SPISMV2.h"
#include "GamePlayHSM.h"
#include "MasterHSM.h"
#include "BallProcessingSM.h"

/* include header files for hardware access
 */
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_pwm.h"
#include "inc/hw_ssi.h"
#include "inc/hw_nvic.h"

/*----------------------------- Module Defines ----------------------------*/
#define TEST_EMITTER_FREQ 0

#define SSI_PRESCALE 0x00000014
#define SCR_VALUE    0x0000C800

#define REG_NORTH 0x10
#define REG_SOUTH 0x01

#define ACK_NORTH 0xA1
#define ACK_SOUTH 0xA3
#define ACK_MASK  0xF3

#define SPI_BYTE_VALUE 2
#define SPI_REFRESH_TIMER_VALUE 100

#define SPI_INITIALIZING 0xFF

#define WEST_RECYCLE_FREQUENCY 1667
#define EAST_RECYCLE_FREQUENCY 2000



#define ZERO_BYTE 0x00

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine*/
void SPIReceiveISR(void);
static void SPISend(uint8_t message);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static SPISM_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

static uint8_t TeamByte;
static uint8_t StatusByte;
static uint8_t ValByte;

static uint8_t RegCmd;
static uint8_t TeamCmd = 0xD2;
static uint8_t StatusCmd = 0x78;
static uint8_t ValCmd = 0x69;

static uint16_t RecycleActFreq[16] = {1000,947,893,840,787,733,680,627,
                                       573,520,467,413,360,307,253,200};

static uint8_t ExpectedAckByte;

static uint8_t LastGameState;
static uint8_t CurrentGameState;
static uint16_t LeftRecycleFrequency;
static uint16_t RightRecycleFrequency;
static uint8_t AssignedColor;
static uint16_t AssignedPeriod;

static uint8_t NSSwitchVal;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitSPISM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
     Sander Tonkens, 02/15/19, 19:09
****************************************************************************/
bool InitSPISM(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  
  //Initialize value of register variable to 0
  TeamByte = 0xFF;
  StatusByte = 0xFF;
  ValByte = 0xFF;
  AssignedColor = 0xFF;
  LastGameState = 0xFF;
  CurrentGameState = 0xFF;
  AssignedPeriod = 0xFFFF;
  
  //Read value of North/South Switch
  NSSwitchVal = QueryTeam(); 
	if (NSSwitchVal == TEAM_NORTH)
    {
      printf("Team North\n\r");
      //Turn on appropriate LED
      HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT0LO;
      //Set Recycling Center Definitions
      LeftRecycleFrequency = EAST_RECYCLE_FREQUENCY;
      RightRecycleFrequency = WEST_RECYCLE_FREQUENCY;
      //Set appropriate REG byte and expected acknowledgement byte
      RegCmd = REG_NORTH;
		  ExpectedAckByte = ACK_NORTH;
    }
    else if (NSSwitchVal == TEAM_SOUTH)
    {
      printf("Team South\n\r");
      //Turn on appropriate LED
      HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT1LO;
      //Set Recycling Center Definitions
      LeftRecycleFrequency = WEST_RECYCLE_FREQUENCY;
      RightRecycleFrequency = EAST_RECYCLE_FREQUENCY;
      //Set appropriate REG byte and expected acknowledgement byte
      RegCmd = REG_SOUTH;
      ExpectedAckByte = ACK_SOUTH;
    }
	
  //Start byte timer in order to start sending messages to COMPASS
	ES_Timer_InitTimer(SPI_BYTE_TIMER, SPI_BYTE_VALUE);
  //disable interrupt
  HWREG(SSI0_BASE + SSI_O_IM) &= ~SSI_IM_TXIM;
  
  // put us into the Initial State
  CurrentState = Registering;
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
     PostSPISM

 Parameters
     EF_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     Sander Tonkens, 02/15/19, 19:08
****************************************************************************/
bool PostSPISM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunSPISM

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   Sander Tonkens, 02/15/19, 19:08
****************************************************************************/
ES_Event_t RunSPISM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  SPISM_t NextState = CurrentState;
	uint8_t ReceivedAckByte;

  switch (CurrentState)
  {
    case Registering:
    {
      if ((ThisEvent.EventType == ES_TIMEOUT) && 
        (ThisEvent.EventParam == SPI_BYTE_TIMER))
      {
				SPISend(RegCmd);
	    }
	    else if (ThisEvent.EventType == EV_COMPASS_RESPONSE_RECEIVED) 
      {       
        ReceivedAckByte = ThisEvent.EventParam;
				//printf("Received Acknowledgment Byte: 0x%02hhx\n\r", ReceivedAckByte);
				if (ReceivedAckByte == ExpectedAckByte)
        {
          printf("COMPASS Registration Success!\n\r");
          //Initialize timer and disable SSI interrupt
					//in preparation for move to next state
					ES_Timer_InitTimer(SPI_BYTE_TIMER, SPI_BYTE_VALUE);
					HWREG(SSI0_BASE + SSI_O_IM) &= ~SSI_IM_TXIM;
					NextState = QueryingTEAM;
		    }
		    else
        {
          printf("Reg: %d\r\n", ReceivedAckByte);
          printf("COMPASS Registration Failed\n\r");
		    }
      }
    }
    break;
    
	case QueryingTEAM:
    {
      if ((ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == SPI_BYTE_TIMER))
      {
        SPISend(TeamCmd);
	    }
	    else if (ThisEvent.EventType == EV_COMPASS_RESPONSE_RECEIVED)
      {
				TeamByte = ThisEvent.EventParam;
				
				AssignedColor = (TeamByte & (BIT1HI|BIT2HI|BIT3HI)) >> 1;
        printf("Assigned Color: %d\n\r", AssignedColor);
        AssignedPeriod = RecycleActFreq[(TeamByte & 
          (BIT4HI|BIT5HI|BIT6HI|BIT7HI)) >> 4];
        printf("Assigned Frequency: %d\n\r", AssignedPeriod);
				
				//Initialize timers and disable SSI interrupt 
				//in preparation for move to next state
				ES_Timer_InitTimer(SPI_BYTE_TIMER, SPI_BYTE_VALUE);
		    ES_Timer_InitTimer(SPI_REFRESH_TIMER, SPI_REFRESH_TIMER_VALUE);
				HWREG(SSI0_BASE + SSI_O_IM) &= ~SSI_IM_TXIM;
				NextState = QueryingSTATUS;
      }
    }
    break;
	
	case QueryingSTATUS:
    {
      if ((ThisEvent.EventType == ES_TIMEOUT) && 
        (ThisEvent.EventParam == SPI_BYTE_TIMER))
      {
        SPISend(StatusCmd);
	    }
	    else if (ThisEvent.EventType == EV_COMPASS_RESPONSE_RECEIVED)
      {
				StatusByte = ThisEvent.EventParam;
				CurrentGameState = (StatusByte & (BIT0HI|BIT1HI));
				//printf("Current Game State: %d\n\r", CurrentGameState);
				
				if ((CurrentGameState == CLEANING_UP) && 
          (LastGameState == WAITING_FOR_START))
        {
          printf("Game Started; event posted\n\r");
          ES_Event_t ThisEvent;
          ThisEvent.EventType = EV_COMPASS_CLEANING_UP;
          PostMasterSM(ThisEvent);
          //PostLEDService(ThisEvent)
        }
        else if ((CurrentGameState == GAME_OVER) && 
          (LastGameState == CLEANING_UP))
        {
          printf("Game Over; event posted\n\r");
          ES_Event_t ThisEvent;
          ThisEvent.EventType = EV_COMPASS_GAME_OVER;
          PostMasterSM(ThisEvent);
          //PostLEDService(ThisEvent)
        }
        LastGameState = CurrentGameState;
				
				//Initialize timer and disable SSI interrupt
				//in preparation for move to next state
		    ES_Timer_InitTimer(SPI_BYTE_TIMER, SPI_BYTE_VALUE);
				HWREG(SSI0_BASE + SSI_O_IM) &= ~SSI_IM_TXIM;
		    NextState = QueryingVAL;
      }
    }
    break;
	
	case QueryingVAL:
    {
      if ((ThisEvent.EventType == ES_TIMEOUT) && 
        (ThisEvent.EventParam == SPI_BYTE_TIMER))
      {
        SPISend(ValCmd);
	    }  
	    else if (ThisEvent.EventType == EV_COMPASS_RESPONSE_RECEIVED)
      {
        ValByte = ThisEvent.EventParam;
      }
	    else if ((ThisEvent.EventType == ES_TIMEOUT) && 
        (ThisEvent.EventParam == SPI_REFRESH_TIMER))
      {
				//prep for move to QueryingSTATUS state
        ES_Timer_InitTimer(SPI_BYTE_TIMER, SPI_BYTE_VALUE);
		    ES_Timer_InitTimer(SPI_REFRESH_TIMER, SPI_REFRESH_TIMER_VALUE);
		    HWREG(SSI0_BASE + SSI_O_IM) &= ~SSI_IM_TXIM;
				NextState = QueryingSTATUS;
      }
    }
    break;
	
    default:
      ;
  }                                   // end switch on Current State
  
  CurrentState = NextState;
  return ReturnEvent;
}

/****************************************************************************
 Function
     SPIReceiveISR

 Parameters
     None

 Returns
     None

 Description
     
 Notes

 Author
     Sander Tonkens, 02/15/2019, 13:14
****************************************************************************/
void SPIReceiveISR(void)
{
  uint8_t ReceivedMessage;
	
	//clear the interupt
  HWREG(SSI0_BASE + SSI_O_ICR) = SSI_ICR_RTIC;
	
	//Ignore first 2 bytes, and store last byte
	uint8_t Receive1 = HWREG(SSI0_BASE + SSI_O_DR);
  uint8_t Receive2 = HWREG(SSI0_BASE + SSI_O_DR);
	ReceivedMessage = HWREG(SSI0_BASE + SSI_O_DR);
  
  //post message to SPI state machine
  printf("Receive 1: %d\r\n", Receive1);
  printf("Receive 2: %d\r\n", Receive2);
  printf("Receive 3: %d\r\n", ReceivedMessage);
  ES_Event_t ThisEvent;
  ThisEvent.EventType = EV_COMPASS_RESPONSE_RECEIVED;
  ThisEvent.EventParam = ReceivedMessage;
  PostSPISM(ThisEvent);
}

/****************************************************************************
 Function
     GetTeamByte

 Parameters
     None

 Returns
     byte value of TEAM

 Description
     
 Notes

 Author
     Sander Tonkens, 02/15/2019, 13:14
****************************************************************************/
uint8_t GetTeamByte(void)
{
  return TeamByte;
}

/****************************************************************************
 Function
     GetStatusByte

 Parameters
     None

 Returns
     byte value of STATUS

 Description
     
 Notes

 Author
     Sander Tonkens, 02/15/2019, 13:15
****************************************************************************/
uint8_t GetStatusByte(void)
{
  return StatusByte;
}

/****************************************************************************
 Function
     GetValByte

 Parameters
     None

 Returns
     byte value of VAL

 Description
     
 Notes

 Author
     Sander Tonkens, 02/15/2019, 13:16
****************************************************************************/
uint8_t GetValByte(void)
{
	return ValByte;
}

/****************************************************************************
 Function
     GetAssignedPeriod

 Parameters
     None

 Returns
     Assigned Frequency

 Description
     
 Notes

 Author
     Sander Tonkens, 02/15/2019, 13:16
****************************************************************************/
uint16_t GetAssignedPeriod(void)
{
	#if TEST_EMITTER_FREQ
  if (RegCmd == TEAM_NORTH)
  {
    AssignedPeriod = 600;
  }
  else if (RegCmd == TEAM_SOUTH)
  {
    AssignedPeriod = 500;
  }
  #endif
  
  return AssignedPeriod;
}

/****************************************************************************
 Function
     GetGameState

 Parameters
     None

 Returns
     Game State

 Description
     
 Notes

 Author
     Sander Tonkens, 02/15/2019, 13:16
****************************************************************************/
uint8_t GetGameState(void)
{
	return CurrentGameState;
}

/****************************************************************************
 Function
     QueryWhichRecycle

 Parameters
     None

 Returns
     which Recycle station is currently accepting our team's color

 Description
     
 Notes

 Author
     Sander Tonkens, 02/18/2019, 19:41
****************************************************************************/
uint8_t QueryWhichRecycle(void)
{
  uint8_t LeftAcceptedColor;
  uint8_t WhichRecycle;
  
  if (NSSwitchVal == TEAM_NORTH)
  {
    LeftAcceptedColor = (GetStatusByte() & (BIT2HI | BIT3HI | BIT4HI)) >> 2;
  }
  else
  {
    LeftAcceptedColor = (GetStatusByte() & (BIT7HI | BIT6HI | BIT5HI)) >> 2;
  }
  
  if (LeftAcceptedColor == GetAssignedColor())
  {
    WhichRecycle = EAST_RECYCLE;
  }
  else
  {
    WhichRecycle = WEST_RECYCLE;
  }
  
  return WhichRecycle;
}

/****************************************************************************
 Function
     GetAssignedColor

 Parameters
     None

 Returns
     Assigned Color

 Description
     
 Notes

 Author
     Sander Tonkens, 02/18/2019, 19:41
****************************************************************************/
uint8_t GetAssignedColor(void)
{
  return AssignedColor;
}

/****************************************************************************
 Function
     GetLeftRecycleFreq

 Parameters
     None

 Returns
     Frequency of Left Recycle Station

 Description
     
 Notes

 Author
     Sander Tonkens, 02/18/2019, 19:41
****************************************************************************/
uint16_t GetLeftRecycleFreq(void)
{
  return LeftRecycleFrequency;
}

/****************************************************************************
 Function
     GetRightRecycleFreq

 Parameters
     None

 Returns
     Frequency of Left Recycle Station

 Description
     
 Notes

 Author
     Sander Tonkens, 02/18/2019, 19:41
****************************************************************************/
uint16_t GetRightRecycleFreq(void)
{
  return RightRecycleFrequency;
}

/****************************************************************************
 Function
     InitSPI

 Parameters
     None

 Returns
     None

 Description
     
 Notes

 Author
     Sander Tonkens, 02/18/2019, 19:41
****************************************************************************/
void InitSPI(void)
{
  //disable interrupts
	__disable_irq();
	//enable the clock to GPIO A and wait
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R0;
    while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R0) != SYSCTL_PRGPIO_R0)
    {}
	//turn on the clock to the ssi module and wait
	HWREG(SYSCTL_RCGCSSI) |= SYSCTL_RCGCSSI_R0;
			while((HWREG(SYSCTL_PRSSI) & SYSCTL_PRSSI_R0) != SYSCTL_PRSSI_R0)
			{}
	
	//set up port a to its alternate function
  HWREG(GPIO_PORTA_BASE + GPIO_O_AFSEL) |= (BIT2HI|BIT3HI|BIT4HI|BIT5HI);				
	
	//set alternate fuctions for pins A2-5 to ssi
	HWREG(GPIO_PORTA_BASE + GPIO_O_PCTL) =
      (HWREG(GPIO_PORTA_BASE + GPIO_O_PCTL) & 0xff0000ff) + (2 << 8) + (2 << 12)
				+ (2 << 16) + (2 << 20);
				
	// Enable pins 2-5 on Port A for digital I/O
  HWREG(GPIO_PORTA_BASE + GPIO_O_DEN) |= (BIT2HI | BIT3HI | BIT4HI | BIT5HI);
  // make pins 2,3 5  on Port A into output
  HWREG(GPIO_PORTA_BASE + GPIO_O_DIR) |= (BIT2HI | BIT3HI | BIT5HI);
	// make pin 4 on Port A into an input
	HWREG(GPIO_PORTA_BASE + GPIO_O_DIR) &= BIT4LO;
			
	//set the pullup resistor for the clock line
	HWREG(GPIO_PORTA_BASE + GPIO_O_PUR) |= BIT2HI;
				
	//disable the ssi before programming it
	HWREG(SSI0_BASE + SSI_O_CR1) &= ~SSI_CR1_SSE;
  
	//select master mode
	HWREG(SSI0_BASE + SSI_O_CR1) &= ~SSI_CR1_MS;
	//select end of transmission interupt enable
	HWREG(SSI0_BASE + SSI_O_CR1) |= SSI_CR1_EOT;

  //set the system clock as the clock source
  HWREG(SSI0_BASE + SSI_O_CC) &= ~SSI_CC_CS_M;
	
	//set the clock prescaler to 4
	HWREG(SSI0_BASE + SSI_O_CPSR) &= 0xffffff00;
	HWREG(SSI0_BASE + SSI_O_CPSR) |= SSI_PRESCALE;
	
	//clear the configure the Serial clock rate bits and write the value we want
	HWREG(SSI0_BASE + SSI_O_CR0) &= 0xffff00ff;
	HWREG(SSI0_BASE + SSI_O_CR0) |= SCR_VALUE;
	
	//set SPH and SPO to 1 for mode 3
	HWREG(SSI0_BASE + SSI_O_CR0) |= (SSI_CR0_SPH | SSI_CR0_SPO);
	
	//set the communications type to freescale SPI
	HWREG(SSI0_BASE + SSI_O_CR0) &= ~SSI_CR0_FRF_M;
	
	//set the data size to 8
	HWREG(SSI0_BASE + SSI_O_CR0) &= ~SSI_CR0_DSS_M;
	HWREG(SSI0_BASE + SSI_O_CR0) |= SSI_CR0_DSS_8;
	
	//enable interupts in ssim
	HWREG(SSI0_BASE + SSI_O_IM) |= (SSI_IM_TXIM | SSI_IM_RXIM);
	
	//enable the ssi before programming it
	HWREG(SSI0_BASE + SSI_O_CR1) |= SSI_CR1_SSE;
	
	//enable wide timer 1 in NVIC (interupt 96)
  HWREG(NVIC_EN0) |= BIT7HI;
	
	//reenable all interupts
	//__enable_irq();
}
 
 /***************************************************************************
 private functions
 ***************************************************************************/

/****************************************************************************
 Function
     SPISend

 Parameters
     message to send

 Returns
     nothing

 Description
     
 Notes

 Author
     Sander Tonkens, 02/15/2019, 16:02
****************************************************************************/
static void SPISend(uint8_t message)
{
  printf("Send: %d\r\n", message);
  //write the data to the data register
  HWREG(SSI0_BASE + SSI_O_DR) = message;
	HWREG(SSI0_BASE + SSI_O_DR) = ZERO_BYTE;
	HWREG(SSI0_BASE + SSI_O_DR) = ZERO_BYTE;
	
	//enable interrupt
  HWREG(SSI0_BASE + SSI_O_IM) |= SSI_IM_TXIM;
}
