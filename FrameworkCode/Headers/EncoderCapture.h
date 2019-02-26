/****************************************************************************
 Header
   EncoderCapture.h

 Module Revision
   1.0.1
	 
****************************************************************************/

#ifndef EncoderCapture_H
#define EncoderCapture_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define WHEEL_1 1
#define WHEEL_2 2
#define BOTH_WHEELS 0

#define WHEEL1A 1
#define WHEEL2A 2
#define WHEEL1B 3
#define WHEEL2B 4

/****************************************************************************
	FUNCTION PROTOTYPES
****************************************************************************/

void Enc_Init(void);
float QueryEncoderTickCount(uint8_t wheel);
void ResetEncoderTickCount(uint8_t wheel);
float QueryEncoderPeriod(uint8_t sensor);
uint32_t QueryEncoderLastEdge(uint8_t sensor);

//***************************************************************************

#endif /* EncoderCapture_H */
