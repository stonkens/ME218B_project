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
#include "SPISM.h"
//#include "DriveSM.h"

/* include header files for hardware access
 */
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_pwm.h"
#include "inc/hw_ssi.h"
#include "inc/hw_nvic.h"

#include "MotorService.h"
#include "MasterHSM.h"
/*----------------------------- Module Defines ----------------------------*/
#define SSI_PRESCALE 0x00000014
#define SCR_VALUE    0x0000C800

#define CPSDVSR 0x00000014 
#define SCR 0x0000C800

#define REG_NORTH 0x10
#define REG_SOUTH 0x01

#define ACK_NORTH 0xA1
#define ACK_SOUTH 0xA3
#define ACK_MASK  0xF3

#define SSI0Clk BIT2HI
#define SSI0Fss BIT3HI
#define SSI0Rx BIT4HI
#define SSI0Tx BIT5HI

#define BitsPerNibble 4

#define SPI_QUERYTIME 2
#define SPI_REFRESHTIME 100

#define SPI_INITIALIZING 0xFF

#define TEAM_NORTH 1
#define TEAM_SOUTH 0

#define EAST_RECYCLE 2
#define WEST_RECYCLE 3



#define ZERO_BYTE 0x00

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine*/
//void SPIReceiveISR(void);
//static void SPISend(uint8_t message);
static void WriteToSPI(uint8_t TransmitMessage);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static SPISM_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

//My team
static uint8_t TeamStatusByte;
static uint8_t GameStatusByte;
static uint8_t ValueByte;

static uint8_t RegistrationByte;

static uint8_t TeamInfoTxByte = 0xD2;
static uint8_t GameStatusTxByte = 0x78;
static uint8_t ValueTxByte = 0x69;

//static uint8_t TeamInfoTxByte = 0xD2;
//static uint8_t StatusCmd = 0x78;
//static uint8_t ValCmd = 0x69;

static uint16_t RecycleActFreq[16] = {1000,947,893,840,787,733,680,627,
                                       573,520,467,413,360,307,253,200};

static uint8_t ExpectedAckByte;

static uint8_t LastGameState;
static uint8_t CurrentGameState;
static uint8_t CurrentRecyclingCenter;
static uint8_t LastRecyclingCenter;                                       
static uint16_t LeftRecycleFrequency;
static uint16_t RightRecycleFrequency;
static uint8_t AssignedColor;
static uint8_t EastRecycleColor;
static uint8_t WestRecycleColor;                                       
																			 
static uint16_t AssignedFrequency;																			 

static uint8_t TeamSwitchValue;


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
     Sander TOnkens, 02/15/19, 19:09
****************************************************************************/
bool InitSPISM(uint8_t Priority)
{
	ES_Event_t ThisEvent;
  MyPriority = Priority;  // save our priority
  ThisEvent.EventType = ES_INIT;

  //initialize SPI hardware
  //InitSPI();

  printf("SSI Init Complete\r\n");

  // Initialize value of register variable to 0

  TeamStatusByte = 0xFF;
  GameStatusByte = 0xFF;
  ValueByte = 0xFF;
  AssignedColor = 0xFF;
  LastGameState = 0xFF;
  CurrentGameState = 0xFF;
  LastRecyclingCenter = 0xFF;
  CurrentRecyclingCenter = 0xFF;
	
  AssignedFrequency = 0xFFFF;

  TeamSwitchValue = TEAM_SOUTH;
  printf("Go Team:%d \r\n", TeamSwitchValue);
	
  if (TeamSwitchValue == TEAM_NORTH)
  {
    //Set REG byte and expected ACK byte accordingly
    RegistrationByte = REG_NORTH;
		//RegCmd = REG_NORTH;
    ExpectedAckByte = ACK_NORTH;

    LeftRecycleFrequency = EAST_RECYCLE_FREQUENCY;
    RightRecycleFrequency = WEST_RECYCLE_FREQUENCY;
  }

  else if (TeamSwitchValue == TEAM_SOUTH)
  {
    //Set REG byte and expected ACK byte accordingly
    RegistrationByte = REG_SOUTH;
		//RegCmd = REG_SOUTH;
    ExpectedAckByte = ACK_SOUTH; 

    LeftRecycleFrequency = WEST_RECYCLE_FREQUENCY;
    RightRecycleFrequency = EAST_RECYCLE_FREQUENCY;
  }

  //Start Timer to start sending messages to COMPASS
	
	ES_Timer_InitTimer(SPI_TIMER, SPI_QUERYTIME);
  //disable interrupt
  HWREG(SSI0_BASE + SSI_O_IM) &= ~SSI_IM_TXIM;
  
  //Transition to initial state
  CurrentState = Registering;

  // Initialization event
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
     Sander TOnkens, 02/15/19, 19:08
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
   Sander TOnkens, 02/15/19, 19:08
****************************************************************************/
ES_Event_t RunSPISM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ES_Event_t CommunicationEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  SPISM_t NextState = CurrentState;
	uint8_t ReceivedAckByte;

  switch (CurrentState)
  {
    case Registering:
    {
      if((ThisEvent.EventType == ES_TIMEOUT) && 
        (ThisEvent.EventParam == SPI_TIMER))
      {
				printf("Writing to SPI \r\n");
        WriteToSPI(RegistrationByte);

      }
      else if (ThisEvent.EventType == RESPONSE_RECEIVED)
      {
				printf("Response received \r\n");
        //Bits 2&3 have to be masked, as ACK byte is unknown
        ReceivedAckByte = ThisEvent.EventParam;

        //Move to next state only if behaviour is as expected
        if(ReceivedAckByte == ExpectedAckByte)
        {
					printf("Successfully registered team \n \r");
          ES_Timer_InitTimer(SPI_TIMER, SPI_QUERYTIME);

          //Disable interrupt
          HWREG(SSI0_BASE + SSI_O_IM) &= ~SSI_IM_TXIM;
          //NextState = QueryTeamInfo;
					NextState = QueryTeamInfo;
        }

        else
        {
          printf("Failed to Register Team \n\r");
        }

      }
    }
    break;
    
	case QueryTeamInfo:
    {
      if ((ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == SPI_TIMER))
      {
        WriteToSPI(TeamInfoTxByte);
	    }
	    else if (ThisEvent.EventType == RESPONSE_RECEIVED)
      {
				TeamStatusByte = ThisEvent.EventParam;
				
				AssignedColor = (TeamStatusByte & (BIT1HI|BIT2HI|BIT3HI)) >> 1;
        printf("Assigned Color: %d\n\r", AssignedColor);
        AssignedFrequency = RecycleActFreq[(TeamStatusByte & 
          (BIT4HI|BIT5HI|BIT6HI|BIT7HI)) >> 4];
        printf("Assigned Frequency: %d\n\r", AssignedFrequency);
				
				//Initialize timers and disable SSI interrupt 
				//in preparation for move to next state
				ES_Timer_InitTimer(SPI_TIMER, SPI_QUERYTIME);
		    ES_Timer_InitTimer(SPI_REFRESH_TIMER, SPI_REFRESHTIME);
				HWREG(SSI0_BASE + SSI_O_IM) &= ~SSI_IM_TXIM;
				NextState = QueryingStatus;
      }
    }
    break;
	
	case QueryingStatus:
    {
      if ((ThisEvent.EventType == ES_TIMEOUT) && 
        (ThisEvent.EventParam == SPI_TIMER))
      {
        WriteToSPI(GameStatusTxByte);
	    }
	    else if (ThisEvent.EventType == RESPONSE_RECEIVED)
      {
				GameStatusByte = ThisEvent.EventParam;
				CurrentGameState = (GameStatusByte & (BIT0HI|BIT1HI));
        EastRecycleColor = (GameStatusByte & (BIT2HI | BIT3HI | BIT4HI)) >> 2;
        WestRecycleColor = (GameStatusByte & (BIT5HI | BIT6HI | BIT7HI)) >> 5;
				//printf("Current Game State: %d\n\r", CurrentGameState);
				if (EastRecycleColor == AssignedColor)
        {
          CurrentRecyclingCenter = EAST_RECYCLE;
        }
        else
        {
          CurrentRecyclingCenter = WEST_RECYCLE;
        }
				if ((CurrentGameState == RECYCLING) && 
          (LastGameState == WAITING_FOR_START))
        {
          printf("Game Started; event not posted\n\r");
          CommunicationEvent.EventType = ES_CLEANING_UP;
          //Change To Master SM
          PostMasterSM(CommunicationEvent);
          
          //Set recycling center we orient to in the RecyclingSM
          
        }
        else if ((CurrentGameState == GAME_OVER) && 
          (LastGameState == RECYCLING))
        {
          printf("Game Over; event not posted\n\r");
          CommunicationEvent.EventType = ES_GAME_OVER;
          PostMasterSM(CommunicationEvent);
        }
        
        if (CurrentRecyclingCenter != LastRecyclingCenter)
        {
          printf("New recycling center: %d \n\r", CurrentRecyclingCenter);
          //Set recycling center we orient to in the RecyclingSM
          CommunicationEvent.EventType = EV_COMPASS_RECYCLE_CHANGE;
          PostMasterSM(CommunicationEvent);
        }
        
        LastGameState = CurrentGameState;
				LastRecyclingCenter = CurrentRecyclingCenter;
				//Initialize timer and disable SSI interrupt
				//in preparation for move to next state
		    ES_Timer_InitTimer(SPI_TIMER, SPI_QUERYTIME);
				HWREG(SSI0_BASE + SSI_O_IM) &= ~SSI_IM_TXIM;
		    NextState = QueryingValue;
      }
    }
    break;
	
	case QueryingValue:
    {
      if ((ThisEvent.EventType == ES_TIMEOUT) && 
        (ThisEvent.EventParam == SPI_TIMER))
      {
        WriteToSPI(ValueTxByte);
	    }  
	    else if (ThisEvent.EventType == RESPONSE_RECEIVED)
      {
        ValueByte = ThisEvent.EventParam;
      }
	    else if ((ThisEvent.EventType == ES_TIMEOUT) && 
        (ThisEvent.EventParam == SPI_REFRESH_TIMER))
      {
				//prep for move to QueryingSTATUS state
        ES_Timer_InitTimer(SPI_TIMER, SPI_QUERYTIME);
		    ES_Timer_InitTimer(SPI_REFRESH_TIMER, SPI_REFRESHTIME);
		    HWREG(SSI0_BASE + SSI_O_IM) &= ~SSI_IM_TXIM;
				NextState = QueryingStatus;
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
     SPIISRResponse

 Parameters
     None

 Returns
     None

 Description
     
 Notes

 Author
     Sander TOnkens, 02/15/2019, 13:14
****************************************************************************/
void SPIISRResponse(void)
{
  uint8_t ResponseMessage;
	//printf("ISR triggered");
  //Clear source of interrupt
  HWREG(SSI0_BASE + SSI_O_ICR) = SSI_ICR_RTIC;

  //Read the data register
	HWREG(SSI0_BASE+SSI_O_DR);
	HWREG(SSI0_BASE+SSI_O_DR);
  ResponseMessage = HWREG(SSI0_BASE + SSI_O_DR);

  //Post message to SPI SM
  ES_Event_t ThisEvent;
  ThisEvent.EventType = RESPONSE_RECEIVED;
  ThisEvent.EventParam = ResponseMessage;
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
     Sander TOnkens, 02/15/2019, 13:14
****************************************************************************/
uint8_t GetTeamInfoByte(void)
{
  return TeamStatusByte;
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
     Sander TOnkens, 02/15/2019, 13:15
****************************************************************************/
uint8_t GetGameStatusByte(void)
{
  return GameStatusByte;
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
     Sander TOnkens, 02/15/2019, 13:16
****************************************************************************/
uint8_t GetValueByte(void)
{
	return ValueByte;
}

/****************************************************************************
 Function
     GetAssignedFreq

 Parameters
     None

 Returns
     Assigned Frequency

 Description
     
 Notes

 Author
     Sander TOnkens, 02/15/2019, 13:16
****************************************************************************/
uint16_t GetAssignedFreq(void)
{
	return AssignedFrequency;
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
     Sander TOnkens, 02/15/2019, 13:16
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
     Sander TOnkens, 02/18/2019, 19:41
****************************************************************************/
uint8_t QueryWhichRecycle(void)
{
  return CurrentRecyclingCenter;
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
     Sander TOnkens, 02/18/2019, 19:41
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
     Sander TOnkens, 02/18/2019, 19:41
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
     Sander TOnkens, 02/18/2019, 19:41
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
     Sander TOnkens, 02/18/2019, 19:41
****************************************************************************/
void InitSPI(void)
{
 __disable_irq();
	printf("Initializing the SSI Module \n \r");

    // Enable the clock to the GPIO Port
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R0;
  
	// Enable the clock to the SSI module
	HWREG(SYSCTL_RCGCSSI) |= SYSCTL_RCGCSSI_R0;

    // Wait for GPIO port to be ready
	while((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R0) != SYSCTL_PRGPIO_R0)
	{
	}	
  while((HWREG(SYSCTL_PRSSI) & SYSCTL_PRSSI_R0) != SYSCTL_PRSSI_R0)
  {}
	// Program the GPIO to use the alternate functions
	// on the SSI pins. The alternate pins are PA2, 3, 4 and 5
	// CLK line = PA2
	// SS line = PA3
	// MISO line = PA4
	// MOSI line = PA5
	HWREG(GPIO_PORTA_BASE + GPIO_O_AFSEL) |= (SSI0Clk | SSI0Fss | SSI0Rx | SSI0Tx); //enable 2-5 for alternative

	//Set mux position in GPIOPCTL to select the SSI use of the pins
	//Mask 2,3,4,5 and send 2 to the position
	HWREG(GPIO_PORTA_BASE + GPIO_O_PCTL) =
		(HWREG(GPIO_PORTA_BASE + GPIO_O_PCTL) & 0xff0000ff) + (2 << (5 * BitsPerNibble)) +
		(2 << (4 * BitsPerNibble)) + (2 << (3 * BitsPerNibble)) + (2 << (2 * BitsPerNibble));

  	// Program the port lines for digital I/O (PA2, PA3, PA4, PA5)
	HWREG(GPIO_PORTA_BASE + GPIO_O_DEN) |= (SSI0Clk | SSI0Fss | SSI0Rx | SSI0Tx);

	// Program the required data directions on the port lines
	// Tx(3), Fs(1) & Clk(0) are all outputs
	// Rx(2) is an input
	HWREG(GPIO_PORTD_BASE + GPIO_O_DIR) |= (SSI0Clk | SSI0Fss | SSI0Tx);
	HWREG(GPIO_PORTD_BASE + GPIO_O_DIR) &= ~(SSI0Rx);
  	
	// If using SPI mode 3, program the pull-up on the clock line (PA2)
	HWREG(GPIO_PORTD_BASE + GPIO_O_PUR) |= SSI0Clk;

  	// Wait for SSI0 to be ready
	while((HWREG(SYSCTL_PRSSI) & SYSCTL_PRSSI_R0) != SYSCTL_PRSSI_R0)
	{
	}
	// Make sure that the SSI is disabled before programming mode bits
	HWREG(SSI0_BASE + SSI_O_CR1) &= ~SSI_CR1_SSE;

  // Select master mode (MS) & TXRIS indicating End of Transmit (EOT)
	HWREG(SSI0_BASE + SSI_O_CR1) &= ~SSI_CR1_MS;
	HWREG(SSI0_BASE + SSI_O_CR1) |= SSI_CR1_EOT;

  //set the system clock as the clock source
  HWREG(SSI0_BASE + SSI_O_CC) &= ~SSI_CC_CS_M;

  	// Configure the clock prescaler
	HWREG(SSI0_BASE + SSI_O_CPSR) &= 0xffffff00; //~SSI_CPSR_CPSDVSR_M;
	HWREG(SSI0_BASE + SSI_O_CPSR) |= CPSDVSR;

	// Configure clock rate, clock phase & clock polarity

  HWREG(SSI0_BASE + SSI_O_CR0) &= 0xffff00ff; //~(SSI_CR0_SCR_M);
  HWREG(SSI0_BASE + SSI_O_CR0) |= SCR;

  //Set SPH and SP0 to 1 for mode 3
	HWREG(SSI0_BASE + SSI_O_CR0) |= (SSI_CR0_SPH | SSI_CR0_SPO);

  //set the communications type to freescale SPI
  HWREG(SSI0_BASE + SSI_O_CR0) &= ~SSI_CR0_FRF_M;

  //set the data size to 8
  HWREG(SSI0_BASE + SSI_O_CR0) &= ~SSI_CR0_DSS_M;
  HWREG(SSI0_BASE + SSI_O_CR0) |= SSI_CR0_DSS_8;

	// Locally enable interrupts
	HWREG(SSI0_BASE + SSI_O_IM) |= (SSI_IM_TXIM | SSI_IM_RXIM);

  // Make sure that the SSI is enabled for operation
  HWREG(SSI0_BASE + SSI_O_CR1) |= SSI_CR1_SSE;

	// Enable NVIC interrupt for SSI when starting transmission
	// NVIC_EN0 handles IRQs 0-31
	// SSI0 = IRQ 7 --> EN_0 |= BIT7HI
	HWREG(NVIC_EN0) |= BIT7HI;

	// Enable interrupts globally
	//__enable_irq();

	printf("SSI Initialization Complete \n\r");
}

 
 /***************************************************************************
 private functions
 ***************************************************************************/

/****************************************************************************
 Function
     WriteToSPI

 Parameters
     message to send

 Returns
     nothing

 Description
     
 Notes

 Author
     Sander TOnkens, 02/15/2019, 16:02
****************************************************************************/
static void WriteToSPI(uint8_t TransmitMessage)
{
	//printf("%d", TransmitMessage);
  //Write data to data register
  HWREG(SSI0_BASE + SSI_O_DR) = TransmitMessage;
	HWREG(SSI0_BASE + SSI_O_DR) = ZERO_BYTE;
	HWREG(SSI0_BASE + SSI_O_DR) = ZERO_BYTE;
  //Enable interrupt
  HWREG(SSI0_BASE + SSI_O_IM) |= SSI_IM_TXIM;
}
