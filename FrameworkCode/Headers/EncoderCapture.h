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

/****************************************************************************
	FUNCTION PROTOTYPES
****************************************************************************/

void Enc_Init(void);
float Enc_GetTickCount(uint8_t wheel);
void Enc_ResetTickCount(uint8_t wheel);
float Enc_GetPeriod(uint8_t sensor);
uint32_t Enc_GetLastEdge(uint8_t sensor);

//***************************************************************************

#endif /* EncoderCapture_H */