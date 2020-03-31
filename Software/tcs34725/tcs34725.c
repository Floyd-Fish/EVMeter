#include "tcs34725.h"

uint8_t TCS34725_IntergrationTime = 0;
uint8_t TCS34725_Gain = 0;

static void TCS34725_WriteByte(uint8_t addr, uint8_t data)
{
    addr = addr | TCS34725_COMMAND_BIT;
    uint8_t buf[1] = { 0 };
    buf[0] = data;
    HAL_I2C_Mem_Write(TCS34725_I2C_PORT, TCS34725_ADDR, addr,
                      I2C_MEMADD_SIZE_8BIT, buf, 1, 0x10);
}

static uint8_t TCS34725_ReadByte(uint8_t addr)
{
    addr = addr | TCS34725_COMMAND_BIT;
    uint8_t buf[1] = { 0 };
    HAL_I2C_Mem_Read(TCS34725_I2C_PORT, TCS34725_ADDR, addr,
                     I2C_MEMADD_SIZE_8BIT, buf, 1, 0x10);
    return buf[0];
}

static uint16_t TCS34725_ReadWord(uint8_t addr)
{
    addr = addr | TCS34725_COMMAND_BIT;
    uint8_t buf[2] = { 0, 0 };
    HAL_I2C_Mem_Read(TCS34725_I2C_PORT, TCS34725_ADDR, addr,
                     I2C_MEMADD_SIZE_8BIT, buf, 2, 0x10);
    return ((buf[1] << 8) | (buf[0] & 0xff));
}

uint8_t TCS34725_ReadID(void)
{
		return TCS34725_ReadByte(TCS34725_ID);
}

void TCS34725_Init(void)
{
    TCS34725_SetIntegrationTime(TCS34725_INTEGRATIONTIME_154MS);
    TCS34725_SetGain(TCS34725_GAIN_16X);
		TCS34725_IntergrationTime = TCS34725_INTEGRATIONTIME_154MS;
		TCS34725_Gain = TCS34725_GAIN_16X;
    TCS34725_Enable();
}

void TCS34725_Enable(void)
{
    TCS34725_WriteByte(TCS34725_ENABLE, TCS34725_ENABLE_PON);
    HAL_Delay(3);
    TCS34725_WriteByte(TCS34725_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
    /* Set a delay for the integration time.
    This is only necessary in the case where enabling and then
    immediately trying to read values back. This is because setting
    AEN triggers an automatic integration, so if a read RGBC is
    performed too quickly, the data is not yet valid and all 0's are
    returned */
    switch (TCS34725_IntergrationTime)
    {
        case TCS34725_INTEGRATIONTIME_2_4MS:
            HAL_Delay(3);
            break;
        case TCS34725_INTEGRATIONTIME_24MS :
            HAL_Delay(24);
            break;
        case TCS34725_INTEGRATIONTIME_50MS :
            HAL_Delay(50);
            break;
        case TCS34725_INTEGRATIONTIME_101MS :
            HAL_Delay(101);
            break;
        case TCS34725_INTEGRATIONTIME_154MS :
            HAL_Delay(154);
            break;
        case TCS34725_INTEGRATIONTIME_700MS :
            HAL_Delay(600);
            break;
        default:
            break;
    }
}

void TCS34725_Disable(void)
{
    uint8_t reg = 0;
    reg = TCS34725_ReadByte(TCS34725_ENABLE);
    TCS34725_WriteByte(TCS34725_ENABLE, reg & ~(TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN));
}

void TCS34725_SetIntegrationTime(uint8_t itime)
{
    /* Update the timing register */
    TCS34725_WriteByte(TCS34725_ATIME, itime);
    /* Update value placeholders */
    TCS34725_IntergrationTime = itime;
}

void TCS34725_SetGain(uint8_t gain)
{
    /* Update the gain register */
    TCS34725_WriteByte(TCS34725_CONTROL, gain);
    /* Update value placeholders */
    TCS34725_Gain = gain;
}

void TCS34725_GetRawData(uint16_t *r, uint16_t *g, uint16_t *b,
                            uint16_t *c)
{
    *c = TCS34725_ReadWord(TCS34725_CDATAL);
    *r = TCS34725_ReadWord(TCS34725_RDATAL);
    *g = TCS34725_ReadWord(TCS34725_GDATAL);
    *b = TCS34725_ReadWord(TCS34725_BDATAL);

    /* Set a delay for the integration time */
    switch (TCS34725_IntergrationTime)
    {
        case TCS34725_INTEGRATIONTIME_2_4MS:
            HAL_Delay(3);
            break;
        case TCS34725_INTEGRATIONTIME_24MS :
            HAL_Delay(24);
            break;
        case TCS34725_INTEGRATIONTIME_50MS :
            HAL_Delay(50);
            break;
        case TCS34725_INTEGRATIONTIME_101MS :
            HAL_Delay(101);
            break;
        case TCS34725_INTEGRATIONTIME_154MS :
            HAL_Delay(154);
            break;
        case TCS34725_INTEGRATIONTIME_700MS :
            HAL_Delay(700);
            break;
        default:
            break;
    }
}

void TCS34725_GetRGB(float *r, float *g, float *b)
{
    uint16_t red, green, blue, clear;
    TCS34725_GetRawData(&red, &green, &blue, &clear);
    uint32_t sum = clear;

    // Avoid divide by zero errors ... if clear = 0 return black
    if (clear == 0) {
        *r = *g = *b = 0;
        return;
    }

    *r = (float)red / sum * 255.0;
    *g = (float)green / sum * 255.0;
    *b = (float)blue / sum * 255.0;
}

uint16_t TCS34725_CalcColorTemp(uint16_t r, uint16_t g, uint16_t b)
{
    float X, Y, Z;      /* RGB to XYZ correction */
    float xc, yc;       /* Chromaticity co-ordinates */
    float n;
    float cct;

    if (r == 0 && g == 0 && b == 0)
        return 0;

    /* 1. Map RGB values to their XYZ counterparts.    */
    /* Based on 6500K fluorescent, 3000K fluorescent   */
    /* and 60W incandescent values for a wide range.   */
    /* Note: Y = Illuminance or lux                    */
    X = (-0.14282F * r) + (1.54924F * g) + (-0.95641F * b);
    Y = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);
    Z = (-0.68202F * r) + (0.77073F * g) + (0.56332F * b);

    /* 2. Calculate the chromaticity co-ordinates      */
    xc = (X) / (X + Y + Z);
    yc = (Y) / (X + Y + Z);

    /* 3. Use McCamy's formula to determine the CCT    */
    n = (xc - 0.3320F) / (0.1858F - yc);

    /* Calculate the final CCT */
    cct = (449.0F * powf(n, 3)) + (3525.0F * powf(n, 2))
            + (6823.3F * n) + 5520.33F;

    /* Return the results in degrees Kelvin */
    return (uint16_t)cct;
}

uint16_t TCS34725_CalcColorTemp_DN40(uint16_t r, uint16_t g,
                                     uint16_t b, uint16_t c)
{
    uint16_t r2, b2; /* RGB values minus IR component */
    uint16_t sat;        /* Digital saturation level */
    uint16_t ir;         /* Inferred IR content */

    if (c == 0) {
        return 0;
    }

    /* Analog/Digital saturation:
     *
     * (a) As light becomes brighter, the clear channel will tend to
     *     saturate first since R+G+B is approximately equal to C.
     * (b) The TCS34725 accumulates 1024 counts per 2.4ms of integration
     *     time, up to a maximum values of 65535. This means analog
     *     saturation can occur up to an integration time of 153.6ms
     *     (64*2.4ms=153.6ms).
     * (c) If the integration time is > 153.6ms, digital saturation will
     *     occur before analog saturation. Digital saturation occurs when
     *     the count reaches 65535.
     */
    if ((256 - TCS34725_IntergrationTime) > 63) {
        /* Track digital saturation */
        sat = 65535;
    } else {
        /* Track analog saturation */
        sat = 1024 * (256 - TCS34725_IntergrationTime);
    }

    /* Ripple rejection:
     *
     * (a) An integration time of 50ms or multiples of 50ms are required to
     *     reject both 50Hz and 60Hz ripple.
     * (b) If an integration time faster than 50ms is required, you may need
     *     to average a number of samples over a 50ms period to reject ripple
     *     from fluorescent and incandescent light sources.
     *
     * Ripple saturation notes:
     *
     * (a) If there is ripple in the received signal, the value read from C
     *     will be less than the max, but still have some effects of being
     *     saturated. This means that you can be below the 'sat' value, but
     *     still be saturating. At integration times >150ms this can be
     *     ignored, but <= 150ms you should calculate the 75% saturation
     *     level to avoid this problem.
     */
    if ((256 - TCS34725_IntergrationTime) <= 63) {
        /* Adjust sat to 75% to avoid analog saturation if atime < 153.6ms */
        sat -= sat / 4;
    }

    /* Check for saturation and mark the sample as invalid if true */
    if (c >= sat) {
        return 0;
    }

    /* AMS RGB sensors have no IR channel, so the IR content must be */
    /* calculated indirectly. */
    ir = (r + g + b > c) ? (r + g + b - c) / 2 : 0;

    /* Remove the IR component from the raw RGB values */
    r2 = r - ir;
    b2 = b - ir;

    if (r2 == 0) {
        return 0;
    }

    /* A simple method of measuring color temp is to use the ratio of blue */
    /* to red light, taking IR cancellation into account. */
    uint16_t cct = (3810 * (uint32_t)b2) / /** Color temp coefficient. */
                   (uint32_t)r2 +
                   1391; /** Color temp offset. */

    return cct;
}

double TCS34725_CalcLux(uint16_t r, uint16_t g, uint16_t b) {
    double illuminance;

    /* This only uses RGB ... how can we integrate clear or calculate lux */
    /* based exclusively on clear since this might be more reliable?      */
    illuminance = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);

    return illuminance;
}

void TCS34725_SetInterrupt(uint8_t i) {
    uint8_t r = TCS34725_ReadByte(TCS34725_ENABLE);
    if (i) {
        r |= TCS34725_ENABLE_AIEN;
    } else {
        r &= ~TCS34725_ENABLE_AIEN;
    }
    TCS34725_WriteByte(TCS34725_ENABLE, r);
}

void TC34725_ClearInterrupt(void)
{
    TCS34725_WriteByte(TCS34725_ADDR, TCS34725_COMMAND_BIT | 0x66);
}

void TCS34725_SetIntLimits(uint16_t low, uint16_t high)
{
    TCS34725_WriteByte(0x04, low & 0xFF);
    TCS34725_WriteByte(0x05, low >> 8);
    TCS34725_WriteByte(0x06, high & 0xFF);
    TCS34725_WriteByte(0x07, high >> 8);
}
