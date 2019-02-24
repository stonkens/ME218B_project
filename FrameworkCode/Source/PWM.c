/****************************************************************************
 Module
   PWM.c

 Revision
   1.0.1

 Description
   PWM regulates low-level PWM control

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/06/19 17:50 ston    Customized to Lab 8 functionality
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

// This module
#include "PWM.h"

/*----------------------------- Module Defines ----------------------------*/
#define ONE_SEC 1000
#define PWMTicksPerMs 40000 / 32    // period in ms * pwm ticks per ms
#define PWMTicksPer100us 4000 / 32  // period in us * pwm ticks per us
#define BitsPerNibble 4

#define GenA_Normal1 (PWM_1_GENA_ACTCMPAU_ONE | PWM_1_GENA_ACTCMPAD_ZERO)
#define GenB_Normal1 (PWM_1_GENB_ACTCMPBU_ONE | PWM_1_GENB_ACTCMPBD_ZERO)

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.
*/
static void Set0_DCA(void);
static void Set100_DCA(void);
static void RestoreDCA(void);

static void Set0_DCB(void);
static void Set100_DCB(void);
static void RestoreDCB(void);

static void SetRotationDirection(uint8_t RotationDirection);

/*---------------------------- Module Variables ---------------------------*/
static uint32_t PWMFrequency = 5000;

/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
    InitMotorGPIO

 Parameters
   None

 Returns
   None

 Description
   Initializes Enable lines for DC motor PB0 and PB1
 Notes

 Author
   Sander Tonkens, 2/05/18, 17:50
****************************************************************************/

void InitMotorGPIO(void)
{
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
  while ((HWREG(SYSCTL_RCGCGPIO) & BIT1HI) != BIT1HI)
  {}
  ;

  //HWREG(GPIO_PORTB_BASE + GPIO_O_DEN) |= (BIT0HI | BIT1HI);
  //HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) |= (BIT0HI | BIT1HI);
  HWREG(GPIO_PORTB_BASE + GPIO_O_DEN) |= (BIT2HI | BIT3HI);
  HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) |= (BIT2HI | BIT3HI);

  //Intially set them as low (the motor won't be running)
  HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT2HI;
  HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT3HI;
}

/****************************************************************************
 Function
    InitPWM

 Parameters
   None

 Returns
   None

 Description
   Initializes PWM lines PB4 and PB5
 Notes

 Author
   Sander Tonkens, 2/05/18, 17:50
****************************************************************************/

void InitPWM(void)
{
  // start by enabling the clock to the PWM Module (PWM0)
  HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;

  //The clock for Port B is already enabled in InitMotorGPIO
  //HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
  // Select the PWM clock as System Clock/32
  HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) |
      (SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);
  // make sure that the PWM module clock has gotten going
  while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0)
  {}
  ;
  // disable the PWM while initializing
  //HWREG(PWM0_BASE + PWM_O_0_CTL) = 0;
  HWREG(PWM0_BASE + PWM_O_1_CTL) = 0;
  // program generators to go to 1 at rising compare A/B, 0 on falling compare A/B
  //HWREG(PWM0_BASE + PWM_O_0_GENA) = GenA_Normal;
  //HWREG(PWM0_BASE + PWM_O_0_GENB) = GenB_Normal;
  HWREG(PWM0_BASE + PWM_O_1_GENA) = GenA_Normal1;
  HWREG(PWM0_BASE + PWM_O_1_GENB) = GenB_Normal1;

  uint32_t PeriodIn100us = 10000 / PWMFrequency;
  // If this mode above is selected we need to modify PWM Ticks per ms by factor 10
  //HWREG(PWM0_BASE+PWM_O_0_LOAD) = ((PeriodIn100us * PWMTicksPer100us))>>1;
  HWREG(PWM0_BASE + PWM_O_1_LOAD) = ((PeriodIn100us * PWMTicksPer100us)) >> 1;
  //Set100_DCA();
  //HWREG(PWM0_BASE+PWM_O_0_CMPA) = HWREG(PWM0_BASE+PWM_O_0_LOAD)>>1;
  //HWREG(PWM0_BASE+PWM_O_0_CMPB) = HWREG(PWM0_BASE+PWM_O_0_LOAD)>>1;

  HWREG(PWM0_BASE + PWM_O_1_CMPA) = HWREG(PWM0_BASE + PWM_O_1_LOAD) >> 1;
  HWREG(PWM0_BASE + PWM_O_1_CMPB) = HWREG(PWM0_BASE + PWM_O_1_LOAD) >> 1;
  // enable the PWM outputs
  //HWREG(PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM0EN | PWM_ENABLE_PWM1EN);
  HWREG(PWM0_BASE + PWM_O_ENABLE) |= (PWM_ENABLE_PWM2EN | PWM_ENABLE_PWM3EN);
  // now configure the Port B pins to be PWM outputs
  // alternate function for PB6
  //HWREG(GPIO_PORTB_BASE+GPIO_O_AFSEL) |= (BIT6HI | BIT7HI);
  HWREG(GPIO_PORTB_BASE + GPIO_O_AFSEL) |= (BIT4HI | BIT5HI);
  // now choose to map PWM to those pins, this is a mux value of 4 that we
  // want to use for specifying the function on bits 6 and 7
  //HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) =
  //(HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) & 0xf0ffffff) + (4<<(6*BitsPerNibble)) + (4<<(7*BitsPerNibble));
  HWREG(GPIO_PORTB_BASE + GPIO_O_PCTL) =
      (HWREG(GPIO_PORTB_BASE + GPIO_O_PCTL) & 0xff00ffff) + (4 << (4 * BitsPerNibble)) + (4 << (5 * BitsPerNibble));
      //Kristine + Sander comment: Check if working with this configuration: otherwise 0x00ffffff
  // Enable pins 6 on Port B for digital I/O
  //HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (BIT6HI | BIT7HI);
  HWREG(GPIO_PORTB_BASE + GPIO_O_DEN) |= (BIT4HI | BIT5HI);
  // make pins 6 on Port B into outputs
  //HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (BIT6HI | BIT7HI);
  HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) |= (BIT4HI | BIT5HI);
  // set the up/down count mode, enable the PWM generator and make
  // both generator updates locally synchronized to zero count
  //HWREG(PWM0_BASE+ PWM_O_0_CTL) = (PWM_0_CTL_MODE | PWM_0_CTL_ENABLE |
  //PWM_0_CTL_GENAUPD_LS | PWM_0_CTL_GENBUPD_LS);
  HWREG(PWM0_BASE + PWM_O_1_CTL) = (PWM_1_CTL_MODE | PWM_1_CTL_ENABLE |
      PWM_1_CTL_GENAUPD_LS | PWM_1_CTL_GENBUPD_LS);
}

/****************************************************************************
 Function
    PWMSetDuty

 Parameters
   None

 Returns
   None

 Description
   Middle level function interfacing with MotorService.c, sets desired PWM & direction
 Notes

 Author
   Sander Tonkens, 2/05/18, 17:50
****************************************************************************/

void PWMSetDuty(uint8_t DutyCycleA, uint8_t DutyCycleB, uint8_t RotationDirection)
{
  PWMSetDutyCycleA(DutyCycleA);
  PWMSetDutyCycleB(DutyCycleB);
  SetRotationDirection(RotationDirection);
}

/****************************************************************************
 Function
    PWMSetDutyCycleA

 Parameters
   None

 Returns
   None

 Description
   Low level function, setting PWM cycle of Motor A (PB4, Pink Ziptie)
 Notes

 Author
   Sander Tonkens, 2/05/18, 17:50
****************************************************************************/

void PWMSetDutyCycleA(uint32_t DutyCycleA)
{
  if (DutyCycleA == 0)
  {
    Set0_DCA();
  }
  else if (DutyCycleA == 100)
  {
    Set100_DCA();
  }
  else
  {
    //printf("DutyCycleA: %d", DutyCycleA);
    //Ensure to reset DC
    RestoreDCA();
    //calculate Compare value for Desired Duty Cycle
    HWREG(PWM0_BASE + PWM_O_1_CMPA) = (HWREG(PWM0_BASE + PWM_O_1_LOAD) * (100 - DutyCycleA)) / 100;
    //store compare value into PWM port
  }
}

/****************************************************************************
 Function
    PWMSetDutyCycleB

 Parameters
   None

 Returns
   None

 Description
   Low level function, setting PWM cycle of Motor B (PB5, Blue Ziptie)
 Notes

 Author
   Sander Tonkens, 2/05/18, 17:50
****************************************************************************/

void PWMSetDutyCycleB(uint32_t DutyCycleB)
{
  if (DutyCycleB == 0)
  {
    Set0_DCB();
  }
  else if (DutyCycleB == 100)
  {
    Set100_DCB();
  }
  else
  {
    //Ensure to reset DC
    RestoreDCB();
    //calculate Compare value for Desired Duty Cycle
    HWREG(PWM0_BASE + PWM_O_1_CMPB) = (HWREG(PWM0_BASE + PWM_O_1_LOAD) * (100 - DutyCycleB)) / 100;
    //store compare value into PWM port
  }
}

/****************************************************************************
 Function
    Set0_DCA

 Parameters
   None

 Returns
   None

 Description
   Low level function, setting 0 Duty Cycle on Motor A (PB4, Pink Ziptie)
 Notes

 Author
   Sander Tonkens, 2/05/18, 17:50
****************************************************************************/

static void Set0_DCA(void)
{
  // To program 0% DC, simply set the action on Zero to set the output to zero
  HWREG(PWM0_BASE + PWM_O_1_GENA) = PWM_1_GENA_ACTZERO_ZERO;
}

/****************************************************************************
 Function
    Set100_DCA

 Parameters
   None

 Returns
   None

 Description
   Low level function, setting 100 Duty Cycle on Motor A (PB4, Pink Ziptie)
 Notes

 Author
   Sander Tonkens, 2/05/18, 17:50
****************************************************************************/

static void Set100_DCA(void)
{
  // To program 100% DC, simply set the action on Zero to set the output to one
  HWREG(PWM0_BASE + PWM_O_1_GENA) = PWM_1_GENA_ACTZERO_ONE;
}

/****************************************************************************
 Function
    RestoreDCA

 Parameters
   None

 Returns
   None

 Description
   Low level function, setting 1-99 Duty Cycle on Motor A (PB4, Pink Ziptie)
 Notes

 Author
   Sander Tonkens, 2/05/18, 17:50
****************************************************************************/

static void RestoreDCA(void)
{
  // To restore the previous DC, simply set the action back to the normal actions
  HWREG(PWM0_BASE + PWM_O_1_GENA) = GenA_Normal1;
}

/****************************************************************************
 Function
    Set0_DCB

 Parameters
   None

 Returns
   None

 Description
   Low level function, setting 0 Duty Cycle on Motor B (PB5, Blue Ziptie)
 Notes

 Author
   Sander Tonkens, 2/05/18, 17:50
****************************************************************************/

static void Set0_DCB(void)
{
  // To program 0% DC, simply set the action on Zero to set the output to zero
  HWREG(PWM0_BASE + PWM_O_1_GENB) = PWM_1_GENB_ACTZERO_ZERO;
}

/****************************************************************************
 Function
    Set100_DCB

 Parameters
   None

 Returns
   None

 Description
   Low level function, setting 100 Duty Cycle on Motor B (PB5, Blue Ziptie)
 Notes

 Author
   Sander Tonkens, 2/05/18, 17:50
****************************************************************************/

static void Set100_DCB(void)
{
  // To program 100% DC, simply set the action on Zero to set the output to one
  HWREG(PWM0_BASE + PWM_O_1_GENB) = PWM_1_GENB_ACTZERO_ONE;
}

/****************************************************************************
 Function
    RestoreDCB

 Parameters
   None

 Returns
   None

 Description
   Low level function, setting 1-99 Duty Cycle on Motor B (PB5, Blue Ziptie)
 Notes

 Author
   Sander Tonkens, 2/05/18, 17:50
****************************************************************************/

static void RestoreDCB(void)
{
  // To restore the previous DC, simply set the action back to the normal actions
  HWREG(PWM0_BASE + PWM_O_1_GENB) = GenB_Normal1;
}

/****************************************************************************
 Function
    SetRotationDirection

 Parameters
   None

 Returns
   None

 Description
   Low level function, setting rotation directions on both motor A and B
 Notes

 Author
   Sander Tonkens, 2/05/18, 17:50
****************************************************************************/

static void SetRotationDirection(uint8_t RotationDirection)
{
  if (RotationDirection == ADVANCE)
  {
    //Set both high
   // HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT0LO;
   // HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT1LO;
    HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT2LO;
    HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT3LO;

    HWREG(PWM0_BASE + PWM_O_INVERT) &= ~PWM_INVERT_PWM2INV;
    HWREG(PWM0_BASE + PWM_O_INVERT) &= ~PWM_INVERT_PWM3INV;
  }
  else if (RotationDirection == REVERSE)
  {
    //Set both low
    //HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT0HI;
    //HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT1HI;
    HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT2HI;
    HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT3HI;
    //Change polarity of bits
    HWREG(PWM0_BASE + PWM_O_INVERT) |= PWM_INVERT_PWM2INV;
    HWREG(PWM0_BASE + PWM_O_INVERT) |= PWM_INVERT_PWM3INV;
  }
  else if (RotationDirection == CW) //equals left turn (CW is more general)
  {
   // HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT0LO;
    //HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT1HI;
    
    HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT2LO;
    HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT3HI;

    HWREG(PWM0_BASE + PWM_O_INVERT) &= ~PWM_INVERT_PWM2INV;
    HWREG(PWM0_BASE + PWM_O_INVERT) |= PWM_INVERT_PWM3INV;
  }
  else if (RotationDirection == CCW) //equals right turn (CCW is more general)
  {
    //HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT0HI;
    //HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT1LO;
    
    HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT2HI;
    HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT3LO;
    

    HWREG(PWM0_BASE + PWM_O_INVERT) |= PWM_INVERT_PWM2INV;
    HWREG(PWM0_BASE + PWM_O_INVERT) &= ~PWM_INVERT_PWM3INV;
  }
}
