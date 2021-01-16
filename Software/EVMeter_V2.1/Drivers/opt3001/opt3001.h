#ifndef __OPT3001_H
#define __OPT3001_H

#include "main.h"

/* HAL I2C Config */
extern I2C_HandleTypeDef    hi2c1;
#define OPT3001_I2C_PORT    &hi2c1

/*  Address Select   |   Hardware connection
 *     0x44 << 1     |       ADDR to GND
 *     0x45 << 1     |       ADDR to VCC
 *     0x46 << 1     |       ADDR to SDA
 *     0x47 << 1     |       ADDR to SCL
 */   
#define OPT3001_ADDR            		(0x44<<1)

/* Register MAP */
#define OPT3001_RESULT_REG              0x00
#define OPT3001_CONFIG_REG              0x01
#define OPT3001_LOWLIMIT_REG            0x02
#define OPT3001_HIGHLIMIT_REG           0x03
#define OPT3001_MANUFACTUREID_REG       0x7E
#define OPT3001_DEVICEID_REG            0x7F

/* Conversion Time Params */
#define DEFAULT_CONFIG_800              0b1100110000010000 // 800ms
#define DEFAULT_CONFIG_800_OS           0b1100101000010000 // 800ms, one shot
#define DEFAULT_CONFIG_100              0b1100010000010000 // 100ms
#define DEFAULT_CONFIG_100_OS           0b1100001000010000 // 100ms, one shot

#define DEFAULT_CONFIG_SHDWN            0xC810 // Shut down converter

/* 	CONFIG REGISTER BITS:| RN3 RN2 RN1 RN0 | CT M1 M0 OVF | CRF FH FL L | Pol ME FC1 FC0 |
	RN3 to RN0 = Range select:
                    1100 by default, enables auto-range 
	CT = Conversion time bit
                    0 = 100ms conversion time
                    1 = 800ms conversion time (default)
	M1 to M0 = Mode bits
                    00 = Shutdown mode
                    01 = Single shot mode
                    10 = Continuous conversion (default)
                    11 = Continuous conversion
	OVF (Bit 8)     Overflow flag. When set the conversion result is overflown.
	CRF (Bit 7)     Conversion ready flag. Sets at end of conversion. Clears by read or write of the Configuration register.
	FH (Bit 6)      Flag high bit. Read only. Sets when result is higher that TH register. Clears when Config register is 
					read or when Latch bit is 0 and the result goes bellow TH register.
	FL (Bit 5)      Flag low bit. Read only. Sets when result is lower that TL register. Clears when Config register is read 
                    or when Latch bit is 0 and the result goes above TL register.
	L (Bit 4)       Latch bit. Read/write bit. Default 1, Controls Latch/transparent functionality of FH and FL bits. When 
                    L = 1 the Alert pin works in window comparator mode with Latched functionality When L = 0 the Alert pin 
                    works in transparent mode and the two limit registers provide the hysteresis.
	Pol (Bit 3)     Polarity. Read/write bit. Default 0, Controls the active state of the Alert pin. Pol = 0 means Alert 
                    active low.
	ME (Bit 2)      Exponent mask. In fixed range modes masks the exponent bits in the result register to 0000.
	FC1 to FC0  -   Fault count bits. Read/write bits. Default 00 - the first fault will trigger the alert pin.
*/

#define OPT3001_CFG_FL          (1 << 5)
#define OPT3001_CFG_FH          (1 << 6)
#define OPT3001_CFG_CRF         (1 << 7)
#define OPT3001_CFG_OVF         (1 << 8)
#define OPT3001_CFG_L           (1 << 4)
#define OPT3001_CFG_POL         (1 << 3)
#define OPT3001_CFG_ME          (1 << 2)
#define OPT3001_CFG_CT          (1 << 11)
#define OPT3001_CFG_FC_SHIFT    0
#define OPT3001_CFG_FC_MASK     (0x03 << OPT3001_CFG_FC_SHIFT)
#define OPT3001_CFG_M_SHIFT     9
#define OPT3001_CFG_M_MASK      (0x03 << OPT3001_CFG_M_SHIFT)
#define OPT3001_CFG_RN_SHIFT    12
#define OPT3001_CFG_RN_MASK     (0xf << OPT3001_CFG_RN_SHIFT)

/* Functions */

void OPT3001_WriteData(uint8_t addr, uint16_t data);

uint16_t OPT3001_ReadData(void);

uint8_t OPT3001_Init(void);

float OPT3001_CalcLux(void);

uint16_t OPT3001_ReadID(void);

uint16_t OPT3001_ReadManu_ID(void);

#endif