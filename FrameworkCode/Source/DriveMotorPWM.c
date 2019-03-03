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
#include "DriveMotorPWM.h"

/*----------------------------- Module Defines ----------------------------*/
#define PWMTicksPerMs 40000 / 32    // period in ms * pwm ticks per ms
#define PWMTicksPer100us 4000 / 32  // period in us * pwm ticks per us
#define BitsPerNibble 4

#define PWM1_GenA_Normal (PWM_1_GENA_ACTCMPAU_ONE | PWM_1_GENA_ACTCMPAD_ZERO)
#define PWM1_GenB_Normal (PWM_1_GENB_ACTCMPBU_ONE | PWM_1_GENB_ACTCMPBD_ZERO)

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.
*/
static void Set0_DC_1(void);
static void Set100_DC_1(void);
static void RestoreDC_1(void);

static void Set0_DC_2(void);
static void Set100_DC_2(void);
static void RestoreDC_2(void);


/*---------------------------- Module Variables ---------------------------*/
static uint32_t PWMFrequency = 3000;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    InitDriveMotor

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
void InitDriveMotor(void)
{
	InitDriveMotorGPIO();
	InitDriveMotorPWM();
}

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

void InitDriveMotorGPIO(void)
{
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
  while ((HWREG(SYSCTL_RCGCGPIO) & BIT1HI) != BIT1HI)
  {}
  ;

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

void InitDriveMotorPWM(void)
{
  // start by enabling the clock to the PWM Module (There is PWM0 and PWM1 as options)
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
  HWREG(PWM0_BASE + PWM_O_1_CTL) = 0;
  // program generators to go to 1 at rising compare A/B, 0 on falling compare A/B
  HWREG(PWM0_BASE + PWM_O_1_GENA) = PWM1_GenA_Normal;
  HWREG(PWM0_BASE + PWM_O_1_GENB) = PWM1_GenB_Normal;

  uint32_t PeriodIn100us = 10000 / PWMFrequency;
  // If this mode above is selected we need to modify PWM Ticks per ms by factor 10
  HWREG(PWM0_BASE + PWM_O_1_LOAD) = ((PeriodIn100us * PWMTicksPer100us)) >> 1;
	
  //This sets the initial duty cycle to 50 % (COMMENTED OUT --> CHECK IF THIS IS AN ISSUE)
  //HWREG(PWM0_BASE + PWM_O_1_CMPA) = HWREG(PWM0_BASE + PWM_O_1_LOAD) >> 1;
  //HWREG(PWM0_BASE + PWM_O_1_CMPB) = HWREG(PWM0_BASE + PWM_O_1_LOAD) >> 1;
  
  //Set initial duty cycle to 0% (Not moving when starting up)
  HWREG(PWM0_BASE + PWM_O_1_GENA) = PWM_1_GENA_ACTZERO_ZERO;
  HWREG(PWM0_BASE + PWM_O_1_GENB) = PWM_1_GENB_ACTZERO_ZERO;
    
	// enable the PWM outputs
  HWREG(PWM0_BASE + PWM_O_ENABLE) |= (PWM_ENABLE_PWM2EN | PWM_ENABLE_PWM3EN);
  
	// now configure the Port B pins to be PWM outputs
  HWREG(GPIO_PORTB_BASE + GPIO_O_AFSEL) |= (BIT4HI | BIT5HI);
		
  // now choose to map PWM to those pins, this is a mux value of 4 that we
  // want to use for specifying the function on bits 6 and 7
  HWREG(GPIO_PORTB_BASE + GPIO_O_PCTL) =
      (HWREG(GPIO_PORTB_BASE + GPIO_O_PCTL) & 0xff00ffff) + (4 << (4 * BitsPerNibble)) + (4 << (5 * BitsPerNibble));
      //Kristine + Sander comment: Check if working with this configuration: otherwise 0x00ffffff
			
  // Enable pins 4 and 5 on Port B for digital I/O
  HWREG(GPIO_PORTB_BASE + GPIO_O_DEN) |= (BIT4HI | BIT5HI);
	
  // make pins 4 and 5 on Port B into outputs
  HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) |= (BIT4HI | BIT5HI);
	
  // set the up/down count mode, enable the PWM generator and make
  // both generator updates locally synchronized to zero count
  HWREG(PWM0_BASE + PWM_O_1_CTL) = (PWM_1_CTL_MODE | PWM_1_CTL_ENABLE |
      PWM_1_CTL_GENAUPD_LS | PWM_1_CTL_GENBUPD_LS);
}


/****************************************************************************
 Function
    PWMSetDutyCycle_1

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

void PWMSetDutyCycle_1(int DutyCycle_1)
{
  if (DutyCycle_1 < 0)
  {
		//Set PB2 high (Motor direction pin)
		HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT2HI;
		//Set Duty Cycle to positive
		DutyCycle_1 = - DutyCycle_1;
    //Change polarity of bits
    HWREG(PWM0_BASE + PWM_O_INVERT) |= PWM_INVERT_PWM2INV;
	}
	
	else //if DutyCycle_1>=0
	{
		HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT2LO;
		HWREG(PWM0_BASE + PWM_O_INVERT) &= ~PWM_INVERT_PWM2INV;
	}
	
	//After setting direction, bound DutyCycle_1 to interval
	if(DutyCycle_1 >=100)
	{
		Set100_DC_1();
	}
	else if (DutyCycle_1 ==0)
	{
		//If necessary calculate inverted Duty
		//DutyCycle_1 = 100 - DutyCycle_1;
    Set0_DC_1();
  }
  else //Regular operation 0<DutyCycle_1<100
  {
    //printf("DutyCycle_1: %d", DutyCycle_1);
    //Ensure to reset DC
    RestoreDC_1();
    //calculate Compare value for Desired Duty Cycle
    HWREG(PWM0_BASE + PWM_O_1_CMPA) = (HWREG(PWM0_BASE + PWM_O_1_LOAD) * (100 - DutyCycle_1)) / 100;
    //store compare value into PWM port
  }
}

/****************************************************************************
 Function
    PWMSetDutyCycle_2

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

void PWMSetDutyCycle_2(int DutyCycle_2)
{
  if (DutyCycle_2 < 0)
  {
		//Set PB2 high (Motor direction pin)
		HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT3HI;
		//Set Duty Cycle to positive
		DutyCycle_2 = - DutyCycle_2;
    //Change polarity of bits
    HWREG(PWM0_BASE + PWM_O_INVERT) |= PWM_INVERT_PWM3INV;
	}
	
	else //if DutyCycle_1>=0
	{
		HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT3LO;
		HWREG(PWM0_BASE + PWM_O_INVERT) &= ~PWM_INVERT_PWM3INV;
	}
	
	//After setting direction, bound DutyCycle_1 to interval
	if(DutyCycle_2 >=100)
	{
		Set100_DC_2();
	}
	else if (DutyCycle_2 ==0)
	{
		//If necessary calculate inverted Duty
		//DutyCycle_1 = 100 - DutyCycle_1;
    Set0_DC_2();
  }
  else //Regular operation 0<DutyCycle_1<100
  {
    //printf("DutyCycle_1: %d", DutyCycle_1);
    //Ensure to reset DC
    RestoreDC_2();
    //calculate Compare value for Desired Duty Cycle
    HWREG(PWM0_BASE + PWM_O_1_CMPB) = (HWREG(PWM0_BASE + PWM_O_1_LOAD) * (100 - DutyCycle_2)) / 100;
    //store compare value into PWM port
  }
}


/****************************************************************************
 Function
    Set0_DC_1

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

static void Set0_DC_1(void)
{
  // To program 0% DC, simply set the action on Zero to set the output to zero
  HWREG(PWM0_BASE + PWM_O_1_GENA) = PWM_1_GENA_ACTZERO_ZERO;
}

/****************************************************************************
 Function
    Set100_DC_1

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

static void Set100_DC_1(void)
{
  // To program 100% DC, simply set the action on Zero to set the output to one
  HWREG(PWM0_BASE + PWM_O_1_GENA) = PWM_1_GENA_ACTZERO_ONE;
}

/****************************************************************************
 Function
    RestoreDC_1

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

static void RestoreDC_1(void)
{
  // To restore the previous DC, simply set the action back to the normal actions
  HWREG(PWM0_BASE + PWM_O_1_GENA) = PWM1_GenA_Normal;
}

/****************************************************************************
 Function
    Set0_DC_2

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

static void Set0_DC_2(void)
{
  // To program 0% DC, simply set the action on Zero to set the output to zero
  HWREG(PWM0_BASE + PWM_O_1_GENB) = PWM_1_GENB_ACTZERO_ZERO;
}

/****************************************************************************
 Function
    Set100_DC_2

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

static void Set100_DC_2(void)
{
  // To program 100% DC, simply set the action on Zero to set the output to one
  HWREG(PWM0_BASE + PWM_O_1_GENB) = PWM_1_GENB_ACTZERO_ONE;
}

/****************************************************************************
 Function
    RestoreDC_2

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

static void RestoreDC_2(void)
{
  // To restore the previous DC, simply set the action back to the normal actions
  HWREG(PWM0_BASE + PWM_O_1_GENB) = PWM1_GenB_Normal;
}

