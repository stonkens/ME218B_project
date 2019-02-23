/****************************************************************************
 Module
   EmitterModule.c

 Revision
   1.0.1

 Description
   This is module implements the function necessary for a reload
   (finds the frequency of the emitter and pulses at twice that frequency)

 Author
   Obi Onyemepu
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
#define GenB_Normal (PWM_1_GENB_ACTCMPBU_ONE | PWM_1_GENB_ACTCMPBD_ZERO)
#define BitsPerNibble 4
#define PWMTicksPerMS 1250  // 40*10^6/32/1000 //40MHz clock divided by 32, converted to ms
#define PeriodInMS 2        //

// Priority
#define RELOAD_IR_INT_PRIORITY 2

// Motor Constants
#define MIN_DC 0    // min dutycycle
#define MAX_DC 100  // max dutycycle

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static int      DesiredPeriod = PeriodInMS * PWMTicksPerMS;
static uint32_t ReloadIRLastCapture = 0;
static uint32_t ReloadIRPeriod;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitReloadIRCapture

 Description
     Initializies the TIVA hardware for an interrupt timer
****************************************************************************/
void InitReloadIRCapture(void) // using Wide Timer 3B
{
  // start by enabling the clock to the timer (Wide Timer 3)
  HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R3;

  // enable the clock to Port D
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3;

  //WAIT FOR CLOCK TO BE READY TO ACCEPT NEW MODIFICATIONS
  while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R3) != SYSCTL_PRGPIO_R3)
  {
    ;
  }

  // make sure that timer (Timer B) is disabled before configuring
  HWREG(WTIMER3_BASE + TIMER_O_CTL) &= ~TIMER_CTL_TBEN;

  // set it up in 32bit wide (individual, not concatenated) mode
  // the constant name derives from the 16/32 bit timer, but this is a 32/64
  // bit timer so we are setting the 32bit mode
  HWREG(WTIMER3_BASE + TIMER_O_CFG) = TIMER_CFG_16_BIT;

  // we want to use the full 32 bit count, so initialize the Interval Load
  // register to 0xffff.ffff (its default value :-)
  HWREG(WTIMER3_BASE + TIMER_O_TBILR) = 0xffffffff;

  // set up timer B in capture mode (TBMR=3, TBAMS = 0),
  // for edge time (TBCMR = 1) and up-counting (TBCDIR = 1)
  HWREG(WTIMER3_BASE + TIMER_O_TBMR) = (HWREG(WTIMER3_BASE + TIMER_O_TBMR) & ~TIMER_TBMR_TBAMS)
      | (TIMER_TBMR_TBCDIR | TIMER_TBMR_TBCMR | TIMER_TBMR_TBMR_CAP);

  // To set the event to rising edge, we need to modify the TBEVENT bits
  // in GPTMCTL. Rising edge = 00, so we clear the TBEVENT bits
  HWREG(WTIMER3_BASE + TIMER_O_CTL) &= ~TIMER_CTL_TBEVENT_M;

  // Now Set up the port to do the capture (clock was enabled earlier)
  // start by setting the alternate function for Port D bit 3 (WT3CCP1)
  HWREG(GPIO_PORTD_BASE + GPIO_O_AFSEL) |= RELOAD_IR_RECEIVER_PIN;

  // Then, map bit 3's alternate function to WT3CCP1
  // 7 is the mux value to select WT3CCP1, 12 to shift it over to the
  // right nibble for bit 3 (4 bits/nibble * 3 bits)
  HWREG(GPIO_PORTD_BASE + GPIO_O_PCTL) = (HWREG(GPIO_PORTD_BASE + GPIO_O_PCTL) & 0xffff0fff)
      + (7 << (3 * BitsPerNibble));

  // Enable pin on Port D for digital I/O
  HWREG(GPIO_PORTD_BASE + GPIO_O_DEN) |= RELOAD_IR_RECEIVER_PIN;

  // make pin 3 on Port D into an input
  HWREG(GPIO_PORTD_BASE + GPIO_O_DIR) &= ~RELOAD_IR_RECEIVER_PIN;

  // back to the timer to enable a local capture interrupt
  HWREG(WTIMER3_BASE + TIMER_O_IMR) |= TIMER_IMR_CBEIM;

  // enable the Timer B in Wide Timer 3 interrupt in the NVIC
  // it is interrupt number 101 so appears in EN3 at bit 5
  HWREG(NVIC_EN3) |= BIT5HI;

  //set interrupt priority to a higher number (lower priority)
  HWREG(NVIC_PRI25) = (HWREG(NVIC_PRI25) & ~NVIC_PRI25_INTB_M) | (RELOAD_IR_INT_PRIORITY << NVIC_PRI25_INTB_S);

  // make sure interrupts are enabled globally
  __enable_irq();

  // now kick the timer off by enabling it and enabling the timer to
  // stall while stopped by the debugger
  HWREG(WTIMER3_BASE + TIMER_O_CTL) |= (TIMER_CTL_TBEN | TIMER_CTL_TBSTALL);

  DisableEmitterInputCapture();
}

/****************************************************************************
 Function
     ReloadIRInputCaptureResponse
****************************************************************************/
void ReloadIRInputCaptureResponse(void)
{
  uint32_t ThisCapture;

  // clear the source of the interrupt
  HWREG(WTIMER3_BASE + TIMER_O_ICR) = TIMER_ICR_CBECINT;

  // grab the captured time and calculate the period
  ThisCapture = HWREG(WTIMER3_BASE + TIMER_O_TBR);
  if ((ThisCapture - ReloadIRLastCapture) != 0)
  {
    ReloadIRPeriod = ThisCapture - ReloadIRLastCapture;
  }

  ReloadIRLastCapture = ThisCapture;
}

/****************************************************************************
 Function
     InitEmitterPWM
****************************************************************************/
void InitEmitterPWM(void) // PWM1 GEN Block 1
{
  // start by enabling the clock to the PWM Module (PWM1)
  HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R1;

  // enable the clock to Port E
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R4;

  // Select the PWM clock as System Clock/32
  HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) |
      (SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);

  // make sure that the PWM module clock has gotten going
  while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R1) != SYSCTL_PRPWM_R1)
  {
    ;
  }

  // disable the PWM1 (Gen 1) while initializing
  HWREG(PWM1_BASE + PWM_O_1_CTL) = 0;

  // program generators to go to 1 at rising compare B, 0 on falling compare B
  HWREG(PWM1_BASE + PWM_O_1_GENB) = GenB_Normal;

  // Set the PWM period. Since we are counting both up & down, we initialize
  // the load register to 1/2 the desired total period. We will also program
  // the match compare registers to 1/2 the desired high time
  HWREG(PWM1_BASE + PWM_O_1_LOAD) = (DesiredPeriod) >> 1;

  // Set the initial Duty cycle on B to 50% by programming the compare value
  // to 1/2 the period to count up (or down). Technically, the value to program
  // should be Period/2 - DesiredHighTime/2, but since the desired high time is 1/2
  // the period, we can skip the subtract
  HWREG(PWM1_BASE + PWM_O_1_CMPB) = HWREG(PWM1_BASE + PWM_O_1_LOAD) >> 1;

  // enable the PWM outputs
  HWREG(PWM1_BASE + PWM_O_ENABLE) |= (PWM_ENABLE_PWM3EN);

  // now configure the Port B pins to be PWM outputs
  // start by selecting the alternate function for 4
  HWREG(GPIO_PORTE_BASE + GPIO_O_AFSEL) |= (IR_EMITTER_PIN);

  // now choose to map PWM to those pins, this is a mux value of 5 (first 5 in expression below) that we
  // want to use for specifying the function on bits 5
  HWREG(GPIO_PORTE_BASE + GPIO_O_PCTL) =
      (HWREG(GPIO_PORTE_BASE + GPIO_O_PCTL) & 0xff0fffff) + (5 << (5 * BitsPerNibble));

  // Enable for digital I/O
  HWREG(GPIO_PORTE_BASE + GPIO_O_DEN) |= (IR_EMITTER_PIN);

  // make pins on Port E into outputs
  HWREG(GPIO_PORTE_BASE + GPIO_O_DIR) |= (IR_EMITTER_PIN);

  // set the up/down countmode, enable the PWM generator and make
  // both generator updates locally synchronized to zero count
  HWREG(PWM1_BASE + PWM_O_1_CTL) = (PWM_1_CTL_MODE | PWM_1_CTL_ENABLE | PWM_1_CTL_GENBUPD_LS);

  //disable after init
  DisableEmitterPWM();
}

/****************************************************************************
 Function
     UpdateEmitterPeriod
Description
     Doubles the frequency of the reload IR beacon and updates the PWM period
****************************************************************************/
void UpdateEmitterPeriod(void)
{
  // Half the period to double the frequency
  // Divide by 32 for clock pre scaler
  // (Total division by 128 --> 2 for doubling freq, 32 for clock prescaler, 2 to load to HWREG)
  HWREG(PWM1_BASE + PWM_O_1_LOAD) = (ReloadIRPeriod) >> 7;
}

/****************************************************************************
 Function
     EnableEmitterInputCapture
****************************************************************************/
void EnableEmitterInputCapture(void)
{
  HWREG(WTIMER3_BASE + TIMER_O_IMR) |= TIMER_IMR_CBEIM;
}

/****************************************************************************
 Function
     DisableEmitterInputCapture
****************************************************************************/
void DisableEmitterInputCapture(void)
{
  HWREG(WTIMER3_BASE + TIMER_O_IMR) &= ~TIMER_IMR_CBEIM;
}

/****************************************************************************
 Function
     EnableEmitterPWM
****************************************************************************/
void EnableEmitterPWM(void)
{
  //restore the previous dc
  HWREG(PWM1_BASE + PWM_O_1_GENB) = GenB_Normal;
  //ensure 50% DC
  HWREG(PWM1_BASE + PWM_O_1_CMPB) = HWREG(PWM1_BASE + PWM_O_1_LOAD) >> 1;
}

/****************************************************************************
 Function
     DisableEmitterPWM
****************************************************************************/
void DisableEmitterPWM(void)
{
  //set to no output
  HWREG(PWM1_BASE + PWM_O_1_GENB) = PWM_1_GENB_ACTZERO_ZERO;
}

///*------------------------------- Footnotes -------------------------------*/
///*------------------------------ End of file ------------------------------*/
