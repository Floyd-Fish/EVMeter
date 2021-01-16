//
// Created by summer on 2020/3/18.
//

#ifndef EVMETER_EV_H
#define EVMETER_EV_H

#include "main.h"


extern float c_constant;

extern uint16_t ISO[24];
extern float APERTURE[21];
extern uint16_t SHUTTER[37];
extern uint32_t SHUTTER_F[37];

#define SHUTTER_F_SIZE      37//sizeof(SHUTTER_F)/sizeof(SHUTTER_F[0])
#define APERTURE_SIZE       21//sizeof(APERTURE)/sizeof(APERTURE[0])
#define ISO_SIZE            24//sizeof(ISO)/sizeof(ISO[0])

uint8_t calcTime(uint16_t iso, float aperture, float lux);

uint8_t calcISO(float aperture, float time, float lux);

uint8_t calcAperture(float time, uint16_t iso, float lux);

float   calcEVReal(float lux, uint16_t iso);

float   Q_rsqrt( float number);
#endif //EVMETER_EV_H
