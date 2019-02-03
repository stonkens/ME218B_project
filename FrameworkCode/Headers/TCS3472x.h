/**************************************************************************
Header file for the TCS3472x Color Sensor
**************************************************************************/
#ifndef _TCS3472x_H_
#define _TCS3472x_H_

#define TCS3472x_ADDRESS          (0x29)

#define TCS3472x_COMMAND_BIT      (0x80)

#define TCS3472x_ENABLE_REG       (0x00)    /* address of the Eanble/control register */
#define TCS3472x_ENABLE_AIEN      (0x10)    /* Interrupt Enable */
#define TCS3472x_ENABLE_WEN       (0x08)    /* Wait enable - set to activate the timer to wait between conversions */
#define TCS3472x_ENABLE_AEN       (0x02)    /* conversion Enable - set to active the ADC and begin conversions */
#define TCS3472x_ENABLE_PON       (0x01)    /* Power on - set to start the ADC oscillator, wait 2.4ms before converting */

#define TCS3472x_ATIME            (0x01)    /* Address of the Integration time register */

// Address 0x02 is unused

#define TCS3472x_WTIME_REG        (0x03)    /* Address of the Wait time register */
#define TCS3472x_WTIME_2_4MS      (0xFF)    /* WTIME value for 2.4ms/0.029s  depending on WLONG */
#define TCS3472x_WTIME_200MS      (0xAD)    /* WTIME value for 200ms/2.4s  depending on WLONG */
#define TCS3472x_WTIME_614MS      (0x00)    /* WTIME value for 614ms/7.45s  depending on WLONG */

#define TCS3472x_AILTL_REG        (0x04)    /* Address of Clear channel lower interrupt threshold, low byte */

#define TCS3472x_AILTH_REG        (0x05)    /* Address of Clear channel lower interrupt threshold, hi byte */

#define TCS3472x_AIHTL_REG        (0x06)    /* Address of Clear channel upper interrupt threshold, low byte */

#define TCS3472x_AIHTH_REG        (0x07)    /* Address of Clear channel upper interrupt threshold, low byte */

// Addresses 0x08-0x0b are unused

#define TCS3472x_PERS_REG         (0x0C)    /* Address of Persistence register */
#define TCS3472x_PERS_NONE        (0b0000)  /* Every conversion cycle generates an interrupt */
#define TCS3472x_PERS_1_CYCLE     (0b0001)  /* 1 clear channel value outside threshold range generates an interrupt   */
#define TCS3472x_PERS_2_CYCLE     (0b0010)  /* 2 clear channel values outside threshold range generates an interrupt  */
#define TCS3472x_PERS_3_CYCLE     (0b0011)  /* 3 clear channel values outside threshold range generates an interrupt  */
#define TCS3472x_PERS_5_CYCLE     (0b0100)  /* 5 clear channel values outside threshold range generates an interrupt  */
#define TCS3472x_PERS_10_CYCLE    (0b0101)  /* 10 clear channel values outside threshold range generates an interrupt */
#define TCS3472x_PERS_15_CYCLE    (0b0110)  /* 15 clear channel values outside threshold range generates an interrupt */
#define TCS3472x_PERS_20_CYCLE    (0b0111)  /* 20 clear channel values outside threshold range generates an interrupt */
#define TCS3472x_PERS_25_CYCLE    (0b1000)  /* 25 clear channel values outside threshold range generates an interrupt */
#define TCS3472x_PERS_30_CYCLE    (0b1001)  /* 30 clear channel values outside threshold range generates an interrupt */
#define TCS3472x_PERS_35_CYCLE    (0b1010)  /* 35 clear channel values outside threshold range generates an interrupt */
#define TCS3472x_PERS_40_CYCLE    (0b1011)  /* 40 clear channel values outside threshold range generates an interrupt */
#define TCS3472x_PERS_45_CYCLE    (0b1100)  /* 45 clear channel values outside threshold range generates an interrupt */
#define TCS3472x_PERS_50_CYCLE    (0b1101)  /* 50 clear channel values outside threshold range generates an interrupt */
#define TCS3472x_PERS_55_CYCLE    (0b1110)  /* 55 clear channel values outside threshold range generates an interrupt */
#define TCS3472x_PERS_60_CYCLE    (0b1111)  /* 60 clear channel values outside threshold range generates an interrupt */

#define TCS3472x_CONFIG_REG       (0x0D)    /* Address of Config register */
#define TCS3472x_CONFIG_WLONG     (0x02)    /* Choose between short and long (12x) wait times in the WTIME register */

// Address 0x0E is unused

#define TCS3472x_CONTROL_REG      (0x0F)    /* Address of Control register, used to set the gain for the sensor */

// Addresses 0x10 & 0x11 are unused

#define TCS3472x_ID               (0x12)    /* 0x44 = TCS34721/TCS34725, 0x4D = TCS34723/TCS34727 */

#define TCS3472x_STATUS_REG       (0x13)    /* Address of Status register */
#define TCS3472x_STATUS_AINT      (0x10)    /* Clear channel interrupt bit mask*/
#define TCS3472x_STATUS_AVALID    (0x01)    /* Indicates valis data available */

#define TCS3472x_CDATAL_REG       (0x14)    /* Clear channel data */
#define TCS3472x_CDATAH_REG       (0x15)

#define TCS3472x_RDATAL_REG       (0x16)    /* Red channel data */
#define TCS3472x_RDATAH_REG       (0x17)

#define TCS3472x_GDATAL_REG       (0x18)    /* Green channel data */
#define TCS3472x_GDATAH_REG       (0x19)

#define TCS3472x_BDATAL_REG       (0x1A)    /* Blue channel data */
#define TCS3472x_BDATAH_REG       (0x1B)

typedef enum
{
  TCS3472x_INTEGRATIONTIME_2_4MS  = 0xFF,   /**<  2.4ms - 1 cycle    - Max Count: 1024  */
  TCS3472x_INTEGRATIONTIME_24MS   = 0xF6,   /**<  24ms  - 10 cycles  - Max Count: 10240 */
  TCS3472x_INTEGRATIONTIME_50MS   = 0xEB,   /**<  50ms  - 20 cycles  - Max Count: 20480 */
  TCS3472x_INTEGRATIONTIME_101MS  = 0xD5,   /**<  101ms - 42 cycles  - Max Count: 43008 */
  TCS3472x_INTEGRATIONTIME_154MS  = 0xC0,   /**<  154ms - 64 cycles  - Max Count: 65535 */
  TCS3472x_INTEGRATIONTIME_700MS  = 0x00    /**<  700ms - 256 cycles - Max Count: 65535 */
}
tcs34725IntegrationTime_t;

typedef enum
{
  TCS3472x_GAIN_1X                = 0x00,   /**<  No gain  */
  TCS3472x_GAIN_4X                = 0x01,   /**<  4x gain  */
  TCS3472x_GAIN_16X               = 0x02,   /**<  16x gain */
  TCS3472x_GAIN_60X               = 0x03    /**<  60x gain */
}
tcs34725Gain_t;


#endif
