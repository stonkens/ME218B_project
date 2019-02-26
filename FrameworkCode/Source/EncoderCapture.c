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


#include "EncoderCapture.h"

/*----------------------------- Module Defines ----------------------------*/
#define BitsPerNibble 4
#define GenB_Normal (PWM_1_GENB_ACTCMPBU_ONE | PWM_1_GENB_ACTCMPBD_ZERO)
#define GenA_Normal (PWM_1_GENB_ACTCMPBU_ONE | PWM_1_GENB_ACTCMPBD_ZERO)
#define PeriodInMS 0.414                      //empirically determined for this motor
#define TicksPerMS 40000
#define PULSES_PER_REV 512
#define PULSES_PER_REV_SMALL 3

#define LEFT_WHEEL 1
#define RIGHT_WHEEL 2
#define BOTH_WHEELS 0

#define LEFT_A 1
#define RIGHT_A 2
#define LEFT_B 3
#define RIGHT_B 4

/*---------------------------- Module Variables ---------------------------*/
// Data private to the module
static float Last_Period_1A;
static float Last_Period_1B;
static float Last_Period_2A;
static float Last_Period_2B;

static uint32_t Last_Capture_1A;
static uint32_t Last_Capture_1B;
static uint32_t Last_Capture_2A;
static uint32_t Last_Capture_2B;

static float TickCount_1;
static float TickCount_2;

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static void InitInputCaptureEnc_1(void);
static void InitInputCaptureEnc_2(void);

/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
   Enc_Init

 Parameters
   void

 Returns
   void

 Description
   initializes hardware and interrupts for left and right encoders
 Notes
   
 Author
   Sander Tonkens
****************************************************************************/
void Enc_Init(void){
	 //initialize encoders A & B for motor 1 (left)
	InitInputCaptureEnc_1();
	 //initialize encoders A & B for motor 2 (right)
	InitInputCaptureEnc_2();
}

/****************************************************************************
 Function
   Enc_GetTickCount

 Parameters
uint8_t: select which wheel's current tick count to return

 Returns
   Current tick count for the selected wheel

 Description
   
 Notes
   
 Author
   Sander Tonkens
****************************************************************************/

float Enc_GetTickCount(uint8_t wheel)
{
	if (wheel == LEFT_WHEEL)
	{
		return TickCount_1;
	}
	else if (wheel == RIGHT_WHEEL)
	{
		return TickCount_2;
	}
	
	else
	{
		return 0;
	}
}

/****************************************************************************
 Function
   Enc_ResetTickCount

 Parameters
uint8_t : select which wheels's current tick count you want to reset

 Returns
  void

 Description
	Sets a wheel's tick count to 0
 Notes
   
 Author
   Sander Tonkens
****************************************************************************/
void Enc_ResetTickCount(uint8_t wheel){
	
	if(wheel == LEFT_WHEEL){
		TickCount_1 = 0;
	}
	else if(wheel == RIGHT_WHEEL){
		TickCount_2 = 0;
	}
	else if(wheel == BOTH_WHEELS){
		TickCount_1 = 0;
		TickCount_2 = 0;
	}
}

/****************************************************************************
 Function
   Enc_GetPeriod

 Parameters
uint8_t : select which sensor's reading you want to return

 Returns
  void

 Description
	Getter for last recorded pulse period from selected sensor
 Notes
   
 Author
   Sander Tonkens
****************************************************************************/
float Enc_GetPeriod(uint8_t sensor){
	
	if(sensor == LEFT_A){
		return Last_Period_1A;
	}
	else if(sensor == LEFT_B){
		return Last_Period_1B;
	}
	else if(sensor == RIGHT_A){
		return Last_Period_2A;
	}
	else if(sensor == RIGHT_B){
		return Last_Period_2B;
	}
	else{
		return 0;
	}
}

/****************************************************************************
 Function
   Enc_GetLastEdge

 Parameters
uint8_t : select which sensor's reading you want to return

 Returns
  void

 Description
	Getter for last recorded edge time from selected sensor
 Notes
   
 Author
   Sander Tonkens
****************************************************************************/
uint32_t Enc_GetLastEdge(uint8_t sensor){
	
	if(sensor == LEFT_A){
		return Last_Capture_1A;
	}
	else if(sensor == LEFT_B){
		return Last_Capture_1B;
	}
	else if(sensor == RIGHT_A){
		return Last_Capture_2A;
	}
	else if(sensor == RIGHT_B){
		return Last_Capture_2B;
	}
	else{
		return 0;
	}
}


static void InitInputCaptureEnc_1(void)
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
 // __enable_irq();
// now kick the timer off by enabling it and enabling the timer to
// stall while stopped by the debugger
  HWREG(WTIMER0_BASE + TIMER_O_CTL) |= (TIMER_CTL_TBEN | TIMER_CTL_TBSTALL);
   //printf("\r\nFinished init input captures");

}

static void InitInputCaptureEnc_2(void)
{	
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
 // __enable_irq();
// now kick the timer off by enabling it and enabling the timer to
// stall while stopped by the debugger
  HWREG(WTIMER1_BASE + TIMER_O_CTL) |= (TIMER_CTL_TBEN | TIMER_CTL_TBSTALL); 
}


void Enc_1AISR(void)
{
  //printf(".");
  uint32_t    ThisCapture_1A;
  //start by clearing the source of the interrupt, the input capture event
  HWREG(WTIMER0_BASE + TIMER_O_ICR) = TIMER_ICR_CAECINT;
  //read the input capture pin
  ThisCapture_1A = HWREG(WTIMER0_BASE + TIMER_O_TAR);
  //take the difference between the last input capture and this input capture
  Last_Period_1A = ThisCapture_1A - Last_Capture_1A;
  //set the last capture value
  Last_Capture_1A = ThisCapture_1A;
	
	//change Tick Count (based on quadrature) MAYBE REVERSED
	
	//if Encoder B is high
	if(HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT5HI)
	{
		//Increment Tick Count
		TickCount_1++;
	}
	else
	{
		//Decrement Tick count and take negative of Last Period
		TickCount_1--;
		Last_Period_1A = -Last_Period_1A;
	}
	
}


void Enc_1BISR(void)
{
  //printf(",");
  uint32_t    ThisCapture_1B;
  //start by clearing the source of the interrupt, the input capture event
  HWREG(WTIMER0_BASE + TIMER_O_ICR) = TIMER_ICR_CBECINT;
  //read the input capture pin
  ThisCapture_1B = HWREG(WTIMER0_BASE + TIMER_O_TBR);
  //take the difference between the last input capture and this input capture
  Last_Period_1B = ThisCapture_1B - Last_Capture_1B;
  //set the last capture value
  Last_Capture_1B = ThisCapture_1B;
}

void Enc_2AISR(void)
{
  //printf(".");
  uint32_t    ThisCapture_2A;
  //start by clearing the source of the interrupt, the input capture event
  HWREG(WTIMER1_BASE + TIMER_O_ICR) = TIMER_ICR_CAECINT;
  //read the input capture pin
  ThisCapture_2A = HWREG(WTIMER1_BASE + TIMER_O_TAR);
  //take the difference between the last input capture and this input capture
  Last_Period_2A = ThisCapture_2A - Last_Capture_2A;
  //set the last capture value
  Last_Capture_2A = ThisCapture_2A;
	
	//Change tick count (based on quadrature)
	if(HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT7HI)
	{
		TickCount_2++;
	}
	else
	{
		TickCount_2--;
		Last_Period_2A = - Last_Period_2A;
	}
}

void Enc_2BISR(void)
{
  //printf(",");
  uint32_t    ThisCapture_2B;
  //start by clearing the source of the interrupt, the input capture event
  HWREG(WTIMER1_BASE + TIMER_O_ICR) = TIMER_ICR_CBECINT;
  //read the input capture pin
  ThisCapture_2B = HWREG(WTIMER1_BASE + TIMER_O_TBR);
  //take the difference between the last input capture and this input capture
  Last_Capture_2B = ThisCapture_2B - Last_Capture_2B;
  //set the last capture value
  Last_Capture_2B = ThisCapture_2B;
}

/*------------------------------- Footnotes -------------------------------*/
