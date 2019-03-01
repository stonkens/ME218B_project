/****************************************************************************

  Header file for template service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef Triangulation_H
#define Triangulation_H

#include "ES_Types.h"
#include <stdint.h>
#include "ES_Configure.h"
#include "ES_Framework.h"

//Public Function prototypes

void Triangulate(float AngleA, float AngleB, float AngleC, float AngleD);
float QueryXCoordinate(void);
float QueryYCoordinate(void);
//Heading is defined as number of degrees CCW from the East
float QueryHeading(void);
float QueryAngle2Origin(void);

#endif /* Triangulation_H */

