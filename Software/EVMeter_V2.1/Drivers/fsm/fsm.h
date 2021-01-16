//
// Created by summer on 2020/3/21.
//

#ifndef EVMETER_FSM_H
#define EVMETER_FSM_H

//#include "stdint.h"
#include "main.h"

/* State Definition */
#define TIME_ADJ_STATE          (uint8_t)(0)
#define APERTURE_ADJ_STATE      (uint8_t)(1)
#define ISO_ADJ_STATE           (uint8_t)(2)
#define EV_ADJ_STATE            (uint8_t)(3)

#define FIRST_STATE             TIME_ADJ_STATE
#define LAST_STATE              EV_ADJ_STATE

typedef struct ev_param {
    uint8_t iso_index;
    uint16_t iso;

    uint8_t time_index;
    uint16_t time;

    uint8_t aperture_index;
    float aperture;

    uint16_t color_temp;
    float lux;

    uint8_t state;
} evp;

enum evm_state
{
    timeAdj = 0,
    apertureAdj = 1,
    isoAdj = 2,
    evAdj = 3
};

void EVMeter_Init();
void fsm_main(void);

void drawGUI(uint8_t t_index, uint8_t i_index, uint8_t a_index,
             uint16_t colorTemp, float lux, uint8_t state);

void timeAdjMode();
void apertureAdjMode();
void isoAdjMode();
void evAdjMode();

float getLuxValue(void);
uint16_t getColorTemp(void);
void refreshEVData();


#endif //EVMETER_FSM_H
