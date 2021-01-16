//
// Created by summer on 2020/3/18.
//

#ifndef EVMETER_KEY_H
#define EVMETER_KEY_H

#include "main.h"

#define KEY0_Pin            GPIO_PIN_1
#define KEY0_GPIO_Port      GPIOA
#define KEY1_Pin            GPIO_PIN_2
#define KEY1_GPIO_Port      GPIOA
#define KEY2_Pin            GPIO_PIN_3
#define KEY2_GPIO_Port      GPIOA


/* delay time */
#define KEY_100MS       8
#define KEY_200MS       20
#define KEY_500MS       50
#define KEY_1S          100
#define KEY_2S          200

#define KEY_UP      1
#define KEY_DOWN    2
#define KEY_SWITCH  3

extern void keyScan(void);
uint8_t getKeyValue(void);
void flushKeyBuffer(void);

#endif //EVMETER_KEY_H
