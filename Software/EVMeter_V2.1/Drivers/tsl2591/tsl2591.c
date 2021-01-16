#include "tsl2591.h"

uint8_t TSL2591_Gain, TSL2591_Time;

static void TSL2591_WriteByte(uint8_t addr, uint8_t data)
{
    addr = addr | TSL2591_COMMAND_BIT;
    uint8_t buf[1] = { 0 };
    buf[0] = data;
    HAL_I2C_Mem_Write(TSL2591_I2C_PORT, TSL2591_ADDR, addr,
            I2C_MEMADD_SIZE_8BIT, buf, 1, 0x20);
}

static uint8_t TSL2591_ReadByte(uint8_t addr)
{
    addr = addr | TSL2591_COMMAND_BIT;
    uint8_t buf[1] = { 0 };
    HAL_I2C_Mem_Read(TSL2591_I2C_PORT, TSL2591_ADDR+1, addr,
            I2C_MEMADD_SIZE_8BIT, buf, 1, 0x20);
    return buf[0];
}

static uint16_t TSL2591_ReadWord(uint8_t addr)
{
    addr = addr | TSL2591_COMMAND_BIT;
    uint8_t buf[2] = { 0, 0 };
    HAL_I2C_Mem_Read(TSL2591_I2C_PORT, TSL2591_ADDR+1, addr,
                     I2C_MEMADD_SIZE_8BIT, buf, 2, 0x20);
    return ((buf[1] << 8) | buf[0] & 0xff);
}

void TSL2591_Enable(void)
{
    TSL2591_WriteByte(TSL2591_REGISTER_ENABLE,
            TSL2591_ENABLE_AEN | TSL2591_ENABLE_AIEN |
            TSL2591_ENABLE_POWERON | TSL2591_ENABLE_NPIEN);
}

void TSL2591_Disable(void)
{
    TSL2591_WriteByte(TSL2591_REGISTER_ENABLE,
                      TSL2591_ENABLE_POWEROFF);
}

uint8_t TSL2591_GetGain(void)
{
    /*************************************************
        LOW_AGAIN           = (0X00)        (1x)
        MEDIUM_AGAIN        = (0X10)        (25x)
        HIGH_AGAIN          = (0X20)        (428x)
        MAX_AGAIN           = (0x30)        (9876x)
    *************************************************/
    uint8_t data;
    data = TSL2591_ReadByte(TSL2591_REGISTER_CONTROL);
    TSL2591_Gain = data & 0x30;
    return data & 0x30;
}

void TSL2591_SetGain(uint8_t Gain)
{
    uint8_t control=0;
    if(Gain == TSL2591_GAIN_LOW || Gain == TSL2591_GAIN_MED
        || Gain == TSL2591_GAIN_HIGH || Gain == TSL2591_GAIN_MAX)
    {
        control =  TSL2591_ReadByte(TSL2591_REGISTER_CONTROL);
        control &= 0xCF; //0b11001111
        control |= Gain;
        TSL2591_WriteByte(TSL2591_REGISTER_CONTROL, control);
        TSL2591_Gain = Gain;
    }
}

uint8_t TSL2591_GetIntegralTime()
{
    uint8_t control=0;
    /************************************************************
        ATIME_100MS = (0x00)      100 millis   MAX COUNT 36863
        ATIME_200MS = (0x01)      200 millis   MAX COUNT 65535
        ATIME_300MS = (0x02)      300 millis   MAX COUNT 65535
        ATIME_400MS = (0x03)      400 millis   MAX COUNT 65535
        ATIME_500MS = (0x04)      500 millis   MAX COUNT 65535
        ATIME_600MS = (0x05)      600 millis   MAX COUNT 65535
    ************************************************************/
    control = TSL2591_ReadByte(TSL2591_REGISTER_CONTROL);
    TSL2591_Time = control & 0x07;
    return control & 0x07; //0b00000111
}

void TSL2591_SetIntegralTime(uint8_t Time)
{
    uint8_t control=0;
    if(Time < 0x06){
        control =  TSL2591_ReadByte(TSL2591_REGISTER_CONTROL);
        control &= 0xf8; //0b11111000
        control |= Time;
        TSL2591_WriteByte(TSL2591_REGISTER_CONTROL, control);
        TSL2591_Time = Time;
    }
}

uint16_t TSL2591_ReadChannel0(void)
{
    return TSL2591_ReadWord(TSL2591_REGISTER_CHAN0_LOW);
}

uint16_t TSL2591_ReadChannel1(void)
{
    return TSL2591_ReadWord(TSL2591_REGISTER_CHAN1_LOW);
}

uint8_t TSL2591_Init(void)
{
    TSL2591_Enable();
    TSL2591_SetGain(TSL2591_GAIN_MED);        //25X GAIN
    TSL2591_SetIntegralTime(TSL2591_INTEGRATIONTIME_100MS);          //200ms Integration time
    TSL2591_WriteByte(TSL2591_REGISTER_PERSIST_FILTER, 0x01);     //filter
    TSL2591_Disable();
    return 0;
}

double TSL2591_ReadLux(void)
{
    uint16_t atime, max_counts, ch0, ch1;
    TSL2591_Enable();
    for(uint8_t i = 0; i < TSL2591_Time; i++)
    {
        HAL_Delay(120);
    }

    ch0 = TSL2591_ReadChannel0();
    ch1 = TSL2591_ReadChannel1();
    TSL2591_Disable();


    //  TSL2591_Enable();
    //  TSL2591_WriteByte(TSL2591_CLEAR_INT, 0x13);
    //  TSL2591_Disable();

    atime = 100 * TSL2591_Time + 100;

    if(TSL2591_Time == TSL2591_INTEGRATIONTIME_100MS){
        max_counts = MAX_COUNT_100MS;
    }else{
        max_counts = MAX_COUNT;
    }

    uint8_t gain_t;
    if (ch0 >= max_counts || ch1 >= max_counts){
        gain_t = TSL2591_GetGain();
        if(gain_t != TSL2591_GAIN_LOW)
        {
            gain_t = ((gain_t>>4)-1)<<4;
            TSL2591_SetGain(gain_t);
            ch0 = 0;
            ch1 = 0;
            while(ch0 <= 0 || ch1 <=0){
                ch0 = TSL2591_ReadChannel0();
                ch1 = TSL2591_ReadChannel1();
            }
            HAL_Delay(100);
        }
    }

    double again;
    again = 1.0;
    if(TSL2591_Gain == TSL2591_GAIN_MED){
        again = 25.0;
    }else if(TSL2591_Gain == TSL2591_GAIN_HIGH){
        again = 428.0;
    }else if(TSL2591_Gain == TSL2591_GAIN_MAX){
        again = 9876.0;
    }
    double cpl;
    double lux;

    cpl = (atime * again) / TSL2591_LUX_DF;
    lux = (((float)ch0 - (float)ch1)) * (1.0F - ((float)ch1 / (float)ch0)) / cpl;
    // lux1 = (int)((ch0 - (2 * ch1)) / Cpl);
    // lux2 = ((0.6 * channel_0) - (channel_1)) / Cpl;
    // This is a two segment lux equation where the first
    // segment (Lux1) covers fluorescent and incandescent light
    // and the second segment (Lux2) covers dimmed incandescent light
    return lux;
}
