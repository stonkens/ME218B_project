Notes on Integrating the sample Color Sensor code into your project.

The repository is a running application that you can use to test your
hardware setup. The code uses I2C0 which has SCL on PB2 and SDA on PB3.
When running, it will print to the screen once a second the most recent
readings from the sensor.

The code is structured as a service that will run continiously sampling
the data from the color sensor every 350ms. On every read, it will update 
the internal variables. There are getter functions for each of the Clear,
Red, Green & Blue results (all uint16_t).

Once the new service is integrated into your project, you simply use the 
getter functions to get the latest color info to use in your decision making.

TO integrate the service into your project:

Edit ES_Configure.h
1) increment the number of services
2) fill in the entries for the Header file name, Init function, run function
and set the queue size to at least 5.
3) Add the new event types to the ES_EventType_t definiton. You can copy these 
from the ES_Configure in the sample application. You need all of the events
that begin with EV_I2C_
4) associate a timer with the I2C service by adding the PostI2CService function 
to the definition of one of the timer response functions. 
5) create a symbolic name (must be I2C_TIMER) and define it as the number of the
timer that you chose at step 4
6) add IsI2C0Finished to the EVENT_CHECK_LIST definition

Edit ES_EventCheckWrapper.h
add a #include for "I2CService.h"

In your code, in the module(s) that you want to reference the getter functions,
#include the header I2CService.h

The getter functions available are:
uint16_t I2C_GetClearValue(void);
uint16_t I2C_GetRedValue(void);
uint16_t I2C_GetGreenValue(void);
uint16_t I2C_GetBlueValue(void);