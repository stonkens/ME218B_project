/****************************************************************************
 Module
   InitializeHardware.c

 Revision
   1.0.1

 Description
   Initialize all the hardware for the lab besides PWM and capture pins

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/06/19 17:50 ston    HW Init customized to Lab 8 pins
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
// Hardware
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "termio.h"

// Specific Hardware
#include "hw_pwm.h"

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"

// Project modules
#include "ADMulti.h"
//#include "CommunicationSSI.h"
#include "DriveMotorPWM.h"
#include "HarvesterService.h"
#include "SPISM.h"
#include "IREmitter.h"
#include "EncoderCapture.h"
#include "DriveCommandModule.h"
#include "IRDetector.h"
#include "TapeFollowingChecker.h"
#include "ReflectiveTapeChecker.h"
// This module
#include "InitializeHardware.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static void InitializeAllPorts(void);
static void InitializeRegularGPIOPorts(void);

/*---------------------------- Module Variables ---------------------------*/

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitializeHardware

 Parameters
     Takes no input parameters

 Returns
     void

 Description
    Call InitializePorts()
    Call InitializeADC()

 Notes

 Author
****************************************************************************/

void InitializeHardware(void)
{
  
  InitializeAllPorts();
  InitializeRegularGPIOPorts();
	InitSPI();
  InitEmitterPWM();
	InitHarvesterMotor();
  InitDriveMotor();
  Enc_Init();
  Drive_Control_Init();
  IRInitInputCapture();
  InitTapeReflectorHardware();
  __enable_irq();
  //InitializeADC();
}

void StopAllMovingParts(void)
{
  StopDrive();
  StopHarvesterMotor();
}


/***************************************************************************
 private functions
 ***************************************************************************/

static void InitializeAllPorts(void)
{  
  // Enable Port A
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R0;  
  // Wait for peripheral A to be ready
  while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R0) != SYSCTL_PRGPIO_R0)
  {;
  }

  //Enable Port B
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
  // Wait for peripheral B to be ready
  while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != SYSCTL_PRGPIO_R1)
  {;
  }
  
  //Enable Port C
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R2;
  // Wait for peripheral C to be ready
  while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R2) != SYSCTL_PRGPIO_R2)
  {;
  }
  
  //Enable Port D
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3;
  // Wait for peripheral D to be ready
  while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R3) != SYSCTL_PRGPIO_R3)
  {;
  }  

  //Enable Port E
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R4;
  // Wait for peripheral E to be ready
  while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R4) != SYSCTL_PRGPIO_R4)
  {;
  } 

  //Enable Port F
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R5;
  // Wait for peripheral F to be ready
  while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R5) != SYSCTL_PRGPIO_R5)
  {;
  }  
}

static void InitializeRegularGPIOPorts(void)
{
  //Set PB0 & PB1 as a digital output
  HWREG(GPIO_PORTB_BASE + GPIO_O_DEN) |= (BIT0HI | BIT1HI);
  HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) |= (BIT0HI | BIT1HI);

  //Intially set them as low (this means they're on)
  HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT0LO;
  HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT1LO;  
  
  //Set PB7 as digital input (for Tapefollowing)
  HWREG(GPIO_PORTB_BASE + GPIO_O_DEN) |= (BIT7HI);
  HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) &= BIT7HI;
  
  //Set PD2, PD3, PD6 & PD7 as digital inputs
  HWREG(GPIO_PORTD_BASE + GPIO_O_DEN) |= (BIT2HI | BIT3HI | BIT6HI | BIT7HI);
  HWREG(GPIO_PORTD_BASE + GPIO_O_DIR) &= BIT2LO; 
  HWREG(GPIO_PORTD_BASE + GPIO_O_DIR) &= BIT3LO;
  HWREG(GPIO_PORTD_BASE + GPIO_O_DIR) &= BIT6LO;  
  HWREG(GPIO_PORTD_BASE + GPIO_O_DIR) &= BIT7LO;  
  
  //Set PE1, PE2 and PE3 as digital inputs (for Tapefollowing & Team Switch)
  HWREG(GPIO_PORTE_BASE + GPIO_O_DEN) |= (BIT1HI | BIT2HI | BIT3HI);
  HWREG(GPIO_PORTE_BASE + GPIO_O_DIR) &= BIT1LO; 
  HWREG(GPIO_PORTE_BASE + GPIO_O_DIR) &= BIT2LO;
  HWREG(GPIO_PORTE_BASE + GPIO_O_DIR) &= BIT3LO;
}
