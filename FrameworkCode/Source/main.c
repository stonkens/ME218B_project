/****************************************************************************
 Module
   main.c
   
 Revision
   1.0.1

 Description
   This is a template file for a main() implementing Hierarchical 
   State Machines within the Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/26/17 14:09 jec     updated with changes from main.c for headers & port 
                        initialization
 02/27/17 12:01 jec     set framework timer rate to 1mS
 02/06/15 13:21 jec     minor tweaks to include header files & clock init for 
                        Tiva
 02/08/12 10:32 jec     major re-work for the Events and Services Framework
                        Gen2
 03/03/10 00:36 jec     now that StartTemplateSM takes an event as input
                        you should pass it something.
 03/17/09 10:20 jec     added cast to return from RunTemplateSM() to quiet
                        warnings because now that function returns Event_t
 02/11/05 16:56 jec     Began coding
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"
#include "termio.h"
#include "EnablePA25_PB23_PD7_PF0.h"

#include "InitializeHardware.h"
#include "TapeFollowingChecker.h"

#define clrScrn() printf("\x1b[2J")
#define goHome() printf("\x1b[1,1H")
#define clrLine() printf("\x1b[K")

int main (void)
{
  ES_Return_t ErrorType;
    
// Set the clock to run at 40MhZ using the PLL and 16MHz external crystal
  SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN
			| SYSCTL_XTAL_16MHZ);

  // Initialize the terminal for puts/printf debugging
  TERMIO_Init();
  clrScrn();
   
// When doing testing, it is useful to announce just which program
// is running.
	puts("\rStarting Team 9 ME218B Project with \r");
	printf("the 2nd Generation Events & Services Framework V2.4\r\n");
	printf("%s %s\n",__TIME__, __DATE__);
	printf("\n\r\n");
  puts( "Keys simulate events:\r");
  puts( "'S'= EV_COMPASS_CLEANING_UP\r");
  puts( "'C'= EV_COMPASS_RECYCLE_CHANGE\r");
  puts( "'G'= EV_COMPASS_GAME_OVER\r");
  puts( "'V, B, N, M'= EV_BUMPER_HIT 1, 2, 3, 4\r");
  puts( "'D'= EV_RECYCLING_DONE\r");
  puts( "'L'= EV_LANDFILLING_DONE\r\n");
  puts( "'Y'= EV_MOVED_BACK\r\n");
  puts( "'P'= EV_MOVE_COMPLETED\r\n");
  puts( "'T'= EV_TAPE_DETECTED\r\n");
  puts( "'Z'= EV_ALIGNED2BEACON\r\n");
  puts( "'X'= Change Position Awareness to true\r\n");
  puts( "      MasterSM     | GamePlayHSM|  CollectingSM | OrientingSM |     RecyclingSM   |     LandfillingSM  | Col. Avoidance |D | R |L | PositionAwarness\r");

  // reprogram the ports that are set as alternate functions or
  // locked coming out of reset. (PA2-5, PB2-3, PD7, PF0)
  // After this call these ports are set
  // as GPIO inputs and can be freely re-programmed to change config.
  // or assign to alternate any functions available on those pins
  PortFunctionInit();

  // Your hardware initialization function calls go here
  InitializeHardware();

  // now initialize the Events and Services Framework and start it running
  ErrorType = ES_Initialize(ES_Timer_RATE_1mS);
  if ( ErrorType == Success ) {
    ErrorType = ES_Run();
  }
//if we got to here, there was an error
  switch (ErrorType){
    case FailedPointer:
      puts("Failed on NULL pointer");
      break;
    case FailedInit:
      puts("Failed Initialization");
      break;
    default:
      puts("Other Failure");
      break;
  }
  for(;;)   // hang after reporting error
    ;
}
