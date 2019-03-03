/****************************************************************************
 Module
   EmitterModule.c

 Revision
   1.0.1

 Description
   This is module implements the function necessary for a reload
   (finds the frequency of the emitter and pulses at twice that frequency)

 Author
   Sander Tonkens
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
// Hardware
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_timer.h"
#include "inc/hw_nvic.h"
#include "inc/hw_pwm.h"

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"

// Header for this module
#include "IREmitter.h"

/*----------------------------- Module Defines ----------------------------*/
// Pin Assignments
#define IR_EMITTER_PIN BIT0HI         // PF0

// PWM Definitions
#define GenA_Normal (PWM_2_GENA_ACTCMPAU_ONE | PWM_2_GENA_ACTCMPAD_ZERO)
#define BitsPerNibble 4
#define PWMTicksPerUS 1250  // 40*10^3/32 //40MHz clock divided by 32, converted to us
#define PeriodInUS 600       //What is currently accepted by recycling centre


/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static uint32_t DesiredPeriod = PeriodInUS * PWMTicksPerUS;

/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
     InitEmitterPWM
****************************************************************************/
void InitEmitterPWM(void) // PWM1 GEN Block 1
{
  // start by enabling the clock to the PWM Module (PWM1)
  HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R1;

  // enable the clock to Port F
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R5;

  // Select the PWM clock as System Clock/32
  HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) |
      (SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);

  // make sure that the PWM module clock has gotten going
  while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R1) != SYSCTL_PRPWM_R1)
  {
    ;
  }

  // disable the PWM2 (Gen 1) while initializing
  HWREG(PWM1_BASE + PWM_O_2_CTL) = 0;

  // program generators to go to 2 at rising compare B, 0 on falling compare B
  HWREG(PWM1_BASE + PWM_O_2_GENA) = GenA_Normal;

  // Set the PWM period. Since we are counting both up & down, we initialize
  // the load register to 1/2 the desired total period. We will also program
  // the match compare registers to 1/2 the desired high time
  HWREG(PWM1_BASE + PWM_O_2_LOAD) = (DesiredPeriod/1000) >> 1;

  // Set the initial Duty cycle on B to 50% by programming the compare value
  // to 1/2 the period to count up (or down). Technically, the value to program
  // should be Period/2 - DesiredHighTime/2, but since the desired high time is 1/2
  // the period, we can skip the subtract
  HWREG(PWM1_BASE + PWM_O_2_CMPA) = HWREG(PWM1_BASE + PWM_O_2_LOAD) >> 1;

  // enable the PWM outputs
  HWREG(PWM1_BASE + PWM_O_ENABLE) |= (PWM_ENABLE_PWM4EN);

  // now configure the Port F0 pin to be a PWM output
  // start by selecting the alternate function for PF0
  HWREG(GPIO_PORTF_BASE + GPIO_O_AFSEL) |= (IR_EMITTER_PIN);

  // now choose to map PWM to those pins, this is a mux value of 5 (first 5 in expression below) that we
  // want to use for specifying the function on bits 5
  HWREG(GPIO_PORTF_BASE + GPIO_O_PCTL) =
      (HWREG(GPIO_PORTF_BASE + GPIO_O_PCTL) & 0xfffffff0) + (5 << (0 * BitsPerNibble));

  // Enable for digital I/O
  HWREG(GPIO_PORTF_BASE + GPIO_O_DEN) |= (IR_EMITTER_PIN);

  // make pins on Port E into outputs
  HWREG(GPIO_PORTF_BASE + GPIO_O_DIR) |= (IR_EMITTER_PIN);
  // set the up/down countmode, enable the PWM generator and make
  // both generator updates locally synchronized to zero count
  HWREG(PWM1_BASE + PWM_O_2_CTL) = (PWM_2_CTL_MODE | PWM_2_CTL_ENABLE | 
  PWM_2_CTL_GENAUPD_LS);

  printf("Finished initializing IR emitter PWM\r\n");
  //disable after init
  //DisableEmitterPWM();
}

/****************************************************************************
 Function
     UpdateEmitterPeriod
Description
     Updates the frequency of the recycling IR beacon, updates the PWM period
****************************************************************************/
void UpdateEmitterPeriod(uint32_t RecycleIRPeriod)
{
  DesiredPeriod = RecycleIRPeriod * PWMTicksPerUS;
  // Half the period to double the frequency
  // Divide by 32 for clock pre scaler
  // (Total division by 64 --> 32 for clock prescaler, 2 to load to HWREG)
  HWREG(PWM1_BASE + PWM_O_2_LOAD) = (DesiredPeriod/1000) >> 1;
}


void EnableEmitterPWM(void)
{
  //restore the previous dc
  HWREG(PWM1_BASE + PWM_O_2_GENA) = GenA_Normal;
  //ensure 50% DC
  HWREG(PWM1_BASE + PWM_O_2_CMPA) = HWREG(PWM1_BASE + PWM_O_2_LOAD) >> 1;
}

/****************************************************************************
 Function
     DisableEmitterPWM
****************************************************************************/
void DisableEmitterPWM(void)
{
  //set to no output
  HWREG(PWM1_BASE + PWM_O_2_GENA) = PWM_2_GENA_ACTZERO_ZERO;
}

///*------------------------------- Footnotes -------------------------------*/
///*------------------------------ End of file ------------------------------*/
