/****************************************************************************
 Module
   EncoderService.c

 Revision
   1.0.1

 Description
   Handles calculating the period of the encoder and the RPM of the motor

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 1/10/19 09:58 ml      began conversion from TemplateFSM.c
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"

#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "inc/hw_pwm.h"
#include "inc/hw_timer.h"
#include "inc/hw_nvic.h"

// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "termio.h"

#include "ADMulti.h"
#include "BITDEFS.h"
#include "InitializeHardware.h"
#include "MotorService.h"

/*----------------------------- Module Defines ----------------------------*/
#define BitsPerNibble 4
#define GenB_Normal (PWM_1_GENB_ACTCMPBU_ONE | PWM_1_GENB_ACTCMPBD_ZERO)
#define GenA_Normal (PWM_1_GENB_ACTCMPBU_ONE | PWM_1_GENB_ACTCMPBD_ZERO)
#define PeriodInMS 0.414                      //empirically determined for this motor
#define TicksPerMS 40000
#define ENCODER_TIME 1000
#define PULSES_PER_REV 512
#define PULSES_PER_REV_SMALL 3

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static void InitInputCaptures(void);
static void InitPeriodicInt(void);
static uint32_t calculateRPM(void);
/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t  MyPriority;
static uint32_t periodInTicks_A1; //period for channel A for MOTOR 1/A in ticks
static uint32_t periodInTicks_B1; //period for channel B for MOTOR 1/A in ticks
static uint32_t periodInTicks_A2; //period for channel A for MOTOR 2/B in ticks
static uint32_t periodInTicks_B2; //period for channel B for MOTOR 2/B in ticks
static uint32_t LastCapture_A1; //encoder channel A for MOTOR 1/A
static uint32_t LastCapture_B1; //encoder channel B for MOTOR 1/A
static uint32_t LastCapture_A2; //encoder channel A for MOTOR 2/B
static uint32_t LastCapture_B2; //encoder channel B for MOTOR 2/B
// control variables 
static float    Kp;
static float    Ki;
static uint32_t RPM;
static int32_t  SumError;
static int32_t  RPMError;
static uint32_t TargetRPM;
static float    RequestedDuty;
static uint32_t CommandDuty;
/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitMotorService

 Parameters
     uint8_t : the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description

 Notes

****************************************************************************/
bool InitEncoderService(uint8_t Priority)
{
  ES_Event_t ThisEvent;
  MyPriority = Priority;
  ES_Timer_InitTimer(ENCODER_TIMER, ENCODER_TIME);
  InitInputCaptures();
  //InitPeriodicInt();
  //Kp = 2;
  //Ki = 0.1;
  

  //ThisEvent.EventType = ES_INIT;
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
     PostEncoderService

 Parameters
     ES_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

****************************************************************************/
bool PostEncoderService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunEncoderService

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description

 Notes

****************************************************************************/
ES_Event_t RunEncoderService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  //printf("\r\n Running encoder service"); 
  
  if ((ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam ==
      ENCODER_TIMER))
  {
    //printf("\r\n A1_period: %d B1_period: %d",periodInTicks_A1,periodInTicks_B1);
    //printf("\r\n A2_period: %d B2_period: %d",periodInTicks_A2,periodInTicks_B2);
    ES_Timer_InitTimer(ENCODER_TIMER, ENCODER_TIME);
  }
  //printf("Right before the return\r\n");
  return ReturnEvent;
}

//uint32_t getEncoderPeriod(void)
//{
//  return periodInTicks;
//}

/* Functions private to the module */

static uint32_t calculateRPM(void)
{
//  //RPM = 2400000000 / periodInTicks / (PULSES_PER_REV) * 6);  // calculation for large motors in lab
//  RPM = 2400000000 / periodInTicks / (PULSES_PER_REV_SMALL * 298);  // 2400000000 is the number of ticks/min ; 6 is the 5.9 gear ratio rounded up
//  return RPM;
}

static void InitPeriodicInt(void)    //PC5
{// start by enabling the clock to the timer (Wide Timer 0)
//  HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R0; // kill a few cycles to let the clock get going
//  while ((HWREG(SYSCTL_PRWTIMER) & SYSCTL_PRWTIMER_R0) != SYSCTL_PRWTIMER_R0)
//  {}
//  ;
//// make sure that timer (Timer B) is disabled before configuring
//  HWREG(WTIMER0_BASE + TIMER_O_CTL) &= ~TIMER_CTL_TBEN;
//// set it up in 32bit wide (individual, not concatenated) mode
//  HWREG(WTIMER0_BASE + TIMER_O_CFG) = TIMER_CFG_16_BIT;
//// set up timer B in periodic mode so that it repeats the time-outs
//  HWREG(WTIMER0_BASE + TIMER_O_TBMR) =
//      (HWREG(WTIMER0_BASE + TIMER_O_TBMR) & ~TIMER_TBMR_TBMR_M) |
//      TIMER_TBMR_TBMR_PERIOD;
//// set timeout to 2mS
//  HWREG(WTIMER0_BASE + TIMER_O_TBILR) = TicksPerMS * 2;
//// enable a local timeout interrupt
//  HWREG(WTIMER0_BASE + TIMER_O_IMR) |= TIMER_IMR_TBTOIM;
//// set the interrupt priority to the second highest priority: 1;
//  HWREG(NVIC_PRI23_INTD_M) = 1;
//// enable the Timer B in Wide Timer 0 interrupt in the NVIC
//// it is interrupt number 95 so appears in EN2 at bit 31
//  HWREG(NVIC_EN2) |= BIT31HI;
//// make sure interrupts are enabled globally
//  __enable_irq();
//// now kick the timer off by enabling it and enabling the timer to
//// stall while stopped by the debugger
//  HWREG(WTIMER0_BASE + TIMER_O_CTL) |= (TIMER_CTL_TBEN |
//      TIMER_CTL_TBSTALL);
}

void controlLoopISR(void)
{
//  raiseIO(); //for 2.6
//  //start by clearing the source of the interrupt, the input capture event
//  HWREG(WTIMER0_BASE + TIMER_O_ICR) = TIMER_ICR_TBTOCINT;
//  //implement control law
//  TargetRPM = 30; //getScaledValue();
//  RPM = calculateRPM();
//  RPMError = TargetRPM - RPM;
//  SumError = SumError + RPMError;
//  RequestedDuty = Kp * (RPMError + (Ki * SumError));
//  //anti-windup for the integrator
//  if (RequestedDuty > 100)
//  {
//    RequestedDuty = 100;
//    SumError -= RPMError; /* anti-windup */
//  }
//  else if (RequestedDuty < 0)
//  {
//    RequestedDuty = 0;
//    SumError -= RPMError; /* anti-windup */
//  }
//  CommandDuty = RequestedDuty;
//  setDuty(CommandDuty);
//  lowerIO(); //for 2.6
}

void A1ISR(void)
{
  //printf(".");
  uint32_t    ThisCapture_A1;
  //start by clearing the source of the interrupt, the input capture event
  HWREG(WTIMER0_BASE + TIMER_O_ICR) = TIMER_ICR_CAECINT;
  //read the input capture pin
  ThisCapture_A1 = HWREG(WTIMER0_BASE + TIMER_O_TAR);
  //take the difference between the last input capture and this input capture
  periodInTicks_A1 = ThisCapture_A1 - LastCapture_A1;
  //set the last capture value
  LastCapture_A1 = ThisCapture_A1;
}


void B1ISR(void)
{
  //printf(",");
  uint32_t    ThisCapture_B1;
  //start by clearing the source of the interrupt, the input capture event
  HWREG(WTIMER0_BASE + TIMER_O_ICR) = TIMER_ICR_CBECINT;
  //read the input capture pin
  ThisCapture_B1 = HWREG(WTIMER0_BASE + TIMER_O_TBR);
  //take the difference between the last input capture and this input capture
  periodInTicks_B1 = ThisCapture_B1 - LastCapture_B1;
  //set the last capture value
  LastCapture_B1 = ThisCapture_B1;
}

void A2ISR(void)
{
  //printf(".");
  uint32_t    ThisCapture_A2;
  //start by clearing the source of the interrupt, the input capture event
  HWREG(WTIMER1_BASE + TIMER_O_ICR) = TIMER_ICR_CAECINT;
  //read the input capture pin
  ThisCapture_A2 = HWREG(WTIMER1_BASE + TIMER_O_TAR);
  //take the difference between the last input capture and this input capture
  periodInTicks_A2 = ThisCapture_A2 - LastCapture_A2;
  //set the last capture value
  LastCapture_A2 = ThisCapture_A2;
}

void B2ISR(void)
{
  //printf(",");
  uint32_t    ThisCapture_B2;
  //start by clearing the source of the interrupt, the input capture event
  HWREG(WTIMER1_BASE + TIMER_O_ICR) = TIMER_ICR_CBECINT;
  //read the input capture pin
  ThisCapture_B2 = HWREG(WTIMER1_BASE + TIMER_O_TBR);
  //take the difference between the last input capture and this input capture
  periodInTicks_B2 = ThisCapture_B2 - LastCapture_B2;
  //set the last capture value
  LastCapture_B2 = ThisCapture_B2;
}

static void InitInputCaptures(void)
{
  /*----------------------------- PC4 -----------------------------*/
// start by enabling the clock to the timer (Wide Timer 0)
  HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R0;
// enable the clock to Port C
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R2;
// since we added this Port C clock init, we can immediately start
  while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_RCGCGPIO_R2) != SYSCTL_RCGCGPIO_R2)
  {}
// into configuring the timer, no need for further delay
// make sure that timer (Timer A) is disabled before configuring
  HWREG(WTIMER0_BASE + TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
// set it up in 32bit wide (individual, not concatenated) mode
// the constant name derives from the 16/32 bit timer, but this is a 32/64
// bit timer so we are setting the 32bit mode
  HWREG(WTIMER0_BASE + TIMER_O_CFG) = TIMER_CFG_16_BIT;
// we want to use the full 32 bit count, so initialize the Interval Load
// register to 0xffff.ffff (its default value :-)
  HWREG(WTIMER0_BASE + TIMER_O_TAILR) = 0xffffffff;
// set up timer A in capture mode (TAMR=3, TAAMS = 0),
// for edge time (TACMR = 1) and up-counting (TACDIR = 1)
  HWREG(WTIMER0_BASE + TIMER_O_TAMR) =
      (HWREG(WTIMER0_BASE + TIMER_O_TAMR) & (~TIMER_TAMR_TAAMS)) |
      (TIMER_TAMR_TACDIR | TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP);
// To set the event to rising edge, we need to modify the TAEVENT bits
// in GPTMCTL. Rising edge = 00, so we clear the TAEVENT bits
  HWREG(WTIMER0_BASE + TIMER_O_CTL) &= ~TIMER_CTL_TAEVENT_M;
// Now set up the port to do the capture (clock was enabled earlier)
// start by setting the alternate function for Port C bit 4 (WT0CCP0)
  HWREG(GPIO_PORTC_BASE + GPIO_O_AFSEL) |= BIT4HI;
// Then, map bit 4's alternate function to WT0CCP0
// 7 is the mux value to select WT0CCP0, 16 to shift it over to the
// right nibble for bit 4 (4 bits/nibble * 4 bits)
  HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) =
      (HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) & 0xfff0ffff) + (7 << 16);
// Enable pin on Port C for digital I/O
  HWREG(GPIO_PORTC_BASE + GPIO_O_DEN) |= BIT4HI;
// make pin 4 on Port C into an input
  HWREG(GPIO_PORTC_BASE + GPIO_O_DIR) &= BIT4LO;
// back to the timer to enable a local capture interrupt
  HWREG(WTIMER0_BASE + TIMER_O_IMR) |= TIMER_IMR_CAEIM;
// enable the Timer A in Wide Timer 0 interrupt in the NVIC
// it is interrupt number 94 so appears in EN2 at bit 30
  HWREG(NVIC_EN2) |= BIT30HI;
// make sure interrupts are enabled globally
  //__enable_irq();
// now kick the timer off by enabling it and enabling the timer to
// stall while stopped by the debugger
  HWREG(WTIMER0_BASE + TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);
  
  /*----------------------------- PC5 -----------------------------*/
// start by enabling the clock to the timer (Wide Timer 0)
  HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R0;
// enable the clock to Port C
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R2;
// since we added this Port C clock init, we can immediately start
// into configuring the timer, no need for further delay
// make sure that timer (Timer B) is disabled before configuring
  HWREG(WTIMER0_BASE + TIMER_O_CTL) &= ~TIMER_CTL_TBEN;
// set it up in 32bit wide (individual, not concatenated) mode
// the constant name derives from the 16/32 bit timer, but this is a 32/64
// bit timer so we are setting the 32bit mode
  HWREG(WTIMER0_BASE + TIMER_O_CFG) = TIMER_CFG_16_BIT;
// we want to use the full 32 bit count, so initialize the Interval Load
// register to 0xffff.ffff (its default value :-)
  HWREG(WTIMER0_BASE + TIMER_O_TBILR) = 0xffffffff;
// set up timer B in capture mode (TBMR=3, TBAMS = 0),
// for edge time (TBCMR = 1) and up-counting (TBCDIR = 1)
  HWREG(WTIMER0_BASE + TIMER_O_TBMR) =
      (HWREG(WTIMER0_BASE + TIMER_O_TBMR) & ~TIMER_TBMR_TBAMS) |
      (TIMER_TBMR_TBCDIR | TIMER_TBMR_TBCMR | TIMER_TBMR_TBMR_CAP);
// To set the event to rising edge, we need to modify the TBEVENT bits
// in GPTMCTL. Rising edge = 00, so we clear the TBEVENT bits
  HWREG(WTIMER0_BASE + TIMER_O_CTL) &= ~TIMER_CTL_TBEVENT_M;
// Now set up the port to do the capture (clock was enabled earlier)
// start by setting the alternate function for Port C bit 5 (WT0CCP1)
  HWREG(GPIO_PORTC_BASE + GPIO_O_AFSEL) |= BIT5HI;
// Then, map bit 5's alternate function to WT0CCP1
// 7 is the mux value to select WT0CCP1, 20 to shift it over to the
// right nibble for bit 5 (4 bits/nibble * 5 bits)
  HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) =
      (HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) & 0xff0fffff) + (7 << 20); // CHECK THIS VALUE
// Enable pin on Port C for digital I/O
  HWREG(GPIO_PORTC_BASE + GPIO_O_DEN) |= BIT5HI;
// make pin 5 on Port C into an input
  HWREG(GPIO_PORTC_BASE + GPIO_O_DIR) &= BIT5LO;
// back to the timer to enable a local capture interrupt
  HWREG(WTIMER0_BASE + TIMER_O_IMR) |= TIMER_IMR_CBEIM;
// enable the Timer B in Wide Timer 0 interrupt in the NVIC
// it is interrupt number 95 so appears in EN2 at bit 31
  HWREG(NVIC_EN2) |= BIT31HI;
// make sure interrupts are enabled globally
  __enable_irq();
// now kick the timer off by enabling it and enabling the timer to
// stall while stopped by the debugger
  HWREG(WTIMER0_BASE + TIMER_O_CTL) |= (TIMER_CTL_TBEN | TIMER_CTL_TBSTALL);
   //printf("\r\nFinished init input captures");
  
  /*----------------------------- PC6 -----------------------------*/
// start by enabling the clock to the timer (Wide Timer 1)
  HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R1;
// enable the clock to Port C
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R2;
// since we added this Port C clock init, we can immediately start
// into configuring the timer, no need for further delay
// make sure that timer (Timer A) is disabled before configuring
  HWREG(WTIMER1_BASE + TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
// set it up in 32bit wide (individual, not concatenated) mode
// the constant name derives from the 16/32 bit timer, but this is a 32/64
// bit timer so we are setting the 32bit mode
  HWREG(WTIMER1_BASE + TIMER_O_CFG) = TIMER_CFG_16_BIT;
// we want to use the full 32 bit count, so initialize the Interval Load
// register to 0xffff.ffff (its default value :-)
  HWREG(WTIMER1_BASE + TIMER_O_TAILR) = 0xffffffff;
// set up timer A in capture mode (TAMR=3, TAAMS = 0),
// for edge time (TACMR = 1) and up-counting (TACDIR = 1)
  HWREG(WTIMER1_BASE + TIMER_O_TAMR) =
      (HWREG(WTIMER1_BASE + TIMER_O_TAMR) & ~TIMER_TAMR_TAAMS) |
      (TIMER_TAMR_TACDIR | TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP);
// To set the event to rising edge, we need to modify the TAEVENT bits
// in GPTMCTL. Rising edge = 00, so we clear the TAEVENT bits
  HWREG(WTIMER1_BASE + TIMER_O_CTL) &= ~TIMER_CTL_TAEVENT_M;
// Now set up the port to do the capture (clock was enabled earlier)
// start by setting the alternate function for Port C bit 6 (WT1CCP0)
  HWREG(GPIO_PORTC_BASE + GPIO_O_AFSEL) |= BIT6HI;
// Then, map bit 6's alternate function to WT1CCP0
// 7 is the mux value to select WT1CCP0, 24 to shift it over to the
// right nibble for bit 4 (4 bits/nibble * 6 bits)
  HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) =
      (HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) & 0xf0ffffff) + (7 << 24);
// Enable pin on Port C for digital I/O
  HWREG(GPIO_PORTC_BASE + GPIO_O_DEN) |= BIT6HI;
// make pin 6 on Port C into an input
  HWREG(GPIO_PORTC_BASE + GPIO_O_DIR) &= BIT6LO;
// back to the timer to enable a local capture interrupt
  HWREG(WTIMER1_BASE + TIMER_O_IMR) |= TIMER_IMR_CAEIM;
// enable the Timer A in Wide Timer 1 interrupt in the NVIC
// it is interrupt number 96 so appears in EN3 at bit 0
  HWREG(NVIC_EN3) |= BIT0HI;
// make sure interrupts are enabled globally
  //__enable_irq();
// now kick the timer off by enabling it and enabling the timer to
// stall while stopped by the debugger
  HWREG(WTIMER1_BASE + TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);
  
    /*----------------------------- PC7 -----------------------------*/
// start by enabling the clock to the timer (Wide Timer 1)
  HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R1;
// enable the clock to Port C
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R2;
// since we added this Port C clock init, we can immediately start
// into configuring the timer, no need for further delay
// make sure that timer (Timer B) is disabled before configuring
  HWREG(WTIMER1_BASE + TIMER_O_CTL) &= ~TIMER_CTL_TBEN;
// set it up in 32bit wide (individual, not concatenated) mode
// the constant name derives from the 16/32 bit timer, but this is a 32/64
// bit timer so we are setting the 32bit mode
  HWREG(WTIMER1_BASE + TIMER_O_CFG) = TIMER_CFG_16_BIT;
// we want to use the full 32 bit count, so initialize the Interval Load
// register to 0xffff.ffff (its default value :-)
  HWREG(WTIMER1_BASE + TIMER_O_TBILR) = 0xffffffff;
// set up timer B in capture mode (TBMR=3, TBAMS = 0),
// for edge time (TBCMR = 1) and up-counting (TBCDIR = 1)
  HWREG(WTIMER1_BASE + TIMER_O_TBMR) =
      (HWREG(WTIMER1_BASE + TIMER_O_TBMR) & ~TIMER_TBMR_TBAMS) |
      (TIMER_TBMR_TBCDIR | TIMER_TBMR_TBCMR | TIMER_TBMR_TBMR_CAP);
// To set the event to rising edge, we need to modify the TBEVENT bits
// in GPTMCTL. Rising edge = 00, so we clear the TBEVENT bits
  HWREG(WTIMER1_BASE + TIMER_O_CTL) &= ~TIMER_CTL_TBEVENT_M;
// Now set up the port to do the capture (clock was enabled earlier)
// start by setting the alternate function for Port C bit 7 (WT1CCP1)
  HWREG(GPIO_PORTC_BASE + GPIO_O_AFSEL) |= BIT7HI;
// Then, map bit 7's alternate function to WT1CCP1
// 7 is the mux value to select WT1CCP1, 28 to shift it over to the
// right nibble for bit 7 (4 bits/nibble * 7 bits)
  HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) =
      (HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) & 0x0fffffff) + (7 << 28); // CHECK THIS VALUE
// Enable pin on Port C for digital I/O
  HWREG(GPIO_PORTC_BASE + GPIO_O_DEN) |= BIT7HI;
// make pin 7 on Port C into an input
  HWREG(GPIO_PORTC_BASE + GPIO_O_DIR) &= BIT7LO;
// back to the timer to enable a local capture interrupt
  HWREG(WTIMER1_BASE + TIMER_O_IMR) |= TIMER_IMR_CBEIM;
// enable the Timer B in Wide Timer 1 interrupt in the NVIC
// it is interrupt number 97 so appears in EN3 at bit 1
  HWREG(NVIC_EN3) |= BIT1HI;
// make sure interrupts are enabled globally
  __enable_irq();
// now kick the timer off by enabling it and enabling the timer to
// stall while stopped by the debugger
  HWREG(WTIMER1_BASE + TIMER_O_CTL) |= (TIMER_CTL_TBEN | TIMER_CTL_TBSTALL); 
}

/*------------------------------- Footnotes -------------------------------*/
