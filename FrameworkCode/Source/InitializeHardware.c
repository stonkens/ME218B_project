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
#include "PWM.h"
#include "DCMotorService.h"
#include "SPISM.h"
#include "IREmitter.h"
#include "IRDetector.h"

// This module
#include "InitializeHardware.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static void InitializePorts(void);

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
  InitializePorts();
	InitSPI();
  InitEmitterPWM();
	InitDCPWM();
  InitMotorGPIO();
  InitPWM();
  //InitSPI(); //This uses bits xxx and xxx
  InitInputCapture();

  __enable_irq();
  //InitializeADC();
}

/***************************************************************************
 private functions
 ***************************************************************************/

static void InitializePorts(void)
{
  // Set bit1 and enable port B
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;

  // Wait for peripheral B to be ready
  while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != SYSCTL_PRGPIO_R1)
  {
    ;
  }

  // Set PB2 to usable pins: TapeDetection
  HWREG(GPIO_PORTB_BASE + GPIO_O_DEN) |= (BIT2HI);

  // Set PB2 to input: TapeDetection
  HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) &= (BIT2LO);
  
    
  // Set bit3 and enable port D
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3;

  // Wait for peripheral D to be ready
  while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R3) != SYSCTL_PRGPIO_R3)
  {
    ;
  }
  
  // Set PD2 to input: Bumper Front, Right, Back, Left
  //printf("initD");
    HWREG(GPIO_PORTD_BASE + GPIO_O_DEN) |= (BIT2HI | BIT3HI | BIT6HI | BIT7HI);

  // Set PD2 to input: Bumper Front, Right, Back, Left
  HWREG(GPIO_PORTD_BASE + GPIO_O_DIR) &= (BIT2LO & BIT3LO & BIT6LO & BIT7LO);

// Set bit 4 and enable port E
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R4;

// Wait for peripheral E to be ready
  while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R4) != SYSCTL_PRGPIO_R4)
  {
    ;
  }

  // Set PE0 to usable pins: Old version of IRBeacon
  HWREG(GPIO_PORTE_BASE + GPIO_O_DEN) |= (BIT0HI);

  // Set PE0 to input: old version of IRBeacon
  HWREG(GPIO_PORTE_BASE + GPIO_O_DIR) &= (BIT0LO);
}
