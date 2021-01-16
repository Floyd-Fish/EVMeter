#include "opt3001.h"
#include "math.h"

//The sensor works in continuous operation mode by default.
static uint16_t default_config = DEFAULT_CONFIG_100;       

void OPT3001_WriteData(uint8_t addr, uint16_t data)
{
	uint8_t buffer[3];
	buffer[0] = addr;
	buffer[1] = data>>8;
	buffer[2] = data & 0x00FF;
	HAL_I2C_Master_Transmit(OPT3001_I2C_PORT, OPT3001_ADDR, buffer, 3, 50);
}

uint16_t OPT3001_ReadData(void)
{

	uint8_t readBuffer[2];
	uint16_t received_data;
	HAL_I2C_Master_Receive(OPT3001_I2C_PORT, OPT3001_ADDR, readBuffer, 2, 50);
	received_data = ((readBuffer[0] << 8) | (readBuffer[1]));
	
	return received_data;
}

uint8_t OPT3001_Init(void)
{
    if (HAL_I2C_IsDeviceReady(&hi2c1, OPT3001_ADDR, 10, 0xFFFF) != HAL_OK) {
		/* Return false */
		//while(1);
		return 0;
	}
    OPT3001_WriteData(OPT3001_CONFIG_REG, default_config);
    return 1;
}

float OPT3001_CalcLux(void)
{
	uint16_t iExponent, iMantissa;
    float final_lux;
    uint16_t rawlux;

	OPT3001_WriteData(OPT3001_RESULT_REG, 0x00);
	HAL_Delay(100);
    rawlux = OPT3001_ReadData();
    iMantissa = rawlux & 0x0FFF;
    iExponent = (rawlux & 0xF000) >> 12;
    final_lux= iMantissa * (0.01 * powf(2.0, (float)iExponent));
    return final_lux;
}

uint16_t OPT3001_ReadID(void)
{

	return OPT3001_ReadData();
}

uint16_t OPT3001_ReadManu_ID(void)
{
    return OPT3001_ReadData();
}