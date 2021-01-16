//
// Created by summer on 2020/3/18.
//

#include "ev.h"
#include "math.h"

#define ABS(x)      x > 0 ? x : -x


float c_constant = 250.0f;

uint16_t ISO[24] = {
        50, 100, 125, 160, 200, 250, 320, 400,
        500, 640, 800, 1000, 1250, 1600, 2000, 2500,
        3200, 4000, 5000, 6400, 8000, 10000, 12800, 25600
};

float APERTURE[21] = {
        1.4, 1.8, 2.0, 2.8, 3.5, 4.0, 4.5, 5.0,
        5.6, 6.3, 7.1, 8.0, 9.0, 10.0, 11.0, 13.0,
        14.0, 16.0, 18.0, 20.0, 22.0
};

uint16_t SHUTTER[37] = {
        /* values at the beginning should be used as 1/x */
        4000, 3200, 2500, 2000, 1600, 1250, 1000,
        800, 640, 500, 400, 320, 250, 200, 160, 125,
        100, 80, 60, 50, 40, 30, 25, 20, 15, 13, 10, 8, 6, 5, 4,
        /* values below represents : 0.3s, 0.4s, 0.5s, 0.6s, 0.8s, 1s */
        3, 4, 5, 6, 8, 1
};


uint32_t SHUTTER_F[37] = {
        250, 313, 400 ,500 ,625 ,800 ,1000 ,
        1250 ,1563 , 2000 , 2500, 3125, 4000, 5000, 6250, 8000,
        10000, 12500, 16667, 20000, 25000, 33333, 40000, 50000, 66667, 76923, 100000, 125000, 166667, 200000, 250000,
        // values below represents : 0.3s, 0.4s, 0.5s, 0.6s, 0.8s, 1s
        300000, 400000, 500000, 600000, 800000, 1000000
};

/**
 * @brief   Calculate Exposure time according to 
 *          specified ISO and Aperture with Lux 
 *          measured by OPT3001
 * 
 * @param   iso       -> input iso value(50-25600)
 * @param   aperture  -> input aperture value(F1.4-F22)
 * @param   lux       -> lux(float) measured by OPT3001
 * @return  shutter time index to indicate shutter value
 * @note    
 * 
 */
uint8_t calcTime(uint16_t iso, float aperture, float lux)
{
    uint32_t time;
    uint8_t i = 0, index = 0;
    float previous_t, next_t;
    time = (uint32_t)(1000000.0 * (aperture * aperture) * c_constant / (lux * (float)iso));

    /* Skip the value below, and prevent from overflow */
    if (time >= SHUTTER_F[SHUTTER_F_SIZE-1])
        return SHUTTER_F_SIZE-1;
    //else if (time <= SHUTTER_F[0])
        //return 0;

    /* avoid MAXIMUM illumination condition */

    for (i = 0; i < SHUTTER_F_SIZE-1; i++)
    {
        if(time <= SHUTTER_F[i])
            break;
    }
    
    if (i == 0)
        return 0;
    else 
    {
        /* Calculate diff */
        previous_t = SHUTTER_F[i-1];
        next_t = SHUTTER_F[i];
        /* Get the Closest Standard value */
        index = (fabs(time-previous_t) > fabs(time-next_t)) ? i : i-1;
        return index;
    }
}

/**
 * @brief   Calculate ISO value according to specified
 *          Aperture and time with lux measured by OPT3001
 * 
 * @param   aperture  -> input aperture value(F1.4-F22)
 * @param   time      -> input time value(0.00025s-1.0s|1/4000-1.0)
 * @param   lux       -> lux(float) measured by OPT3001
 * @return  ISO index to indicate ISO value
 * @note    
 * 
 */
uint8_t calcISO(float aperture, float time, float lux)
{
    uint16_t iso;
    uint8_t i = 0, index = 0;
    uint16_t previous_iso, next_iso;
    iso = (aperture * aperture * lux) / (time * c_constant);
    /* Skip the value below, and prevent from overflow */
    if (iso > ISO[ISO_SIZE-1])
        return ISO_SIZE-1;

    for(i = 0; i < ISO_SIZE-1; i++)
    {
        if (iso <= ISO[i])
            break;
    }

    if (i == 0)
        return 0;
    else
    {
        previous_iso = ISO[i-1];
        next_iso = ISO[i];

        index = (ABS(iso-previous_iso) > ABS(iso-next_iso)) ? i : i-1;
        return index;
    }
}

/**
 * @brief   Calculate Aperture value according to specified
 *          time and iso with lux measured by OPT3001
 * 
 * @param   time      -> input time value(0.00025s-1.0s|1/4000-1.0)
 * @param   iso       -> input iso value(50-25600)
 * @param   lux       -> lux(float) measured by OPT3001
 * @return  Aperture index to indicate Aperture value
 * @note    
 * 
 */
uint8_t calcAperture(float time, uint16_t iso, float lux)
{
    double aperture;
    uint8_t i = 0, index = 0, cnt = 0;
    float previous_aperture, next_aperture;
    aperture = sqrtf((iso * lux * (time/1000000)) / c_constant);
    /* Skip the value below, and prevent from overflow */

    if (aperture >= APERTURE[APERTURE_SIZE-1])
        return APERTURE_SIZE-1;

    for (i = 0; i < APERTURE_SIZE-1; i++)
    {
        if (aperture >= APERTURE[i])
            cnt++;
    }

    if (cnt == 0)
        return 0;
    else 
    {
        previous_aperture = APERTURE[cnt-1];
        next_aperture = APERTURE[cnt];
        index = (fabs(aperture-previous_aperture) > fabs(aperture-next_aperture)) ? cnt : cnt-1;
        return index;        
    }
}

/**
 * @brief   Calculate read EV value according only to lux
 *          measured by OPT3001 on constant condition(ISO)
 * @param   lux -> lux value measured by OPT3001
 * @param   iso -> input ISO value to set constant env.
 * @return  Real EV value in float
 * @note 
 * 
 */
float calcEVReal(float lux, uint16_t iso)
{
    float EVr;
    EVr = log2(lux * iso / c_constant);
    return EVr;
}

float Q_rsqrt( float number )
{
    long i;
    float x2, y;
    const float threehalfs = 1.5F;
    x2 = number * 0.5F;
    y   = number;
    i   = * ( long * ) &y;
    // evil floating point bit level hacking
    i   = 0x5f3759df - ( i >> 1 ); // what the fuck?
    y   = * ( float * ) &i;
    y   = y * ( threehalfs - ( x2 * y * y ) );
    // 1st iteration
    // y   = y * ( threehalfs - ( x2 * y * y ) );
    // 2nd iteration, this can be removed

    return y;
}