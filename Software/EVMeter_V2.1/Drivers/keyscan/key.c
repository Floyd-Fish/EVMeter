//
// Created by summer on 2020/3/18.
//
#include "key.h"
#include "stdbool.h"

/* define Read GPIO pins for each key */
#define PORT_KEY_UP         HAL_GPIO_ReadPin (KEY0_GPIO_Port, KEY0_Pin)
#define PORT_KEY_DOWN       HAL_GPIO_ReadPin (KEY1_GPIO_Port, KEY2_Pin)
#define PORT_KEY_SWITCH     HAL_GPIO_ReadPin (KEY2_GPIO_Port, KEY1_Pin)

/* Counter for time counting */
uint32_t    keyUpTimer;
uint32_t    keyDownTimer;
uint32_t    keySwitchTimer;

/* KeyState */
bool     upPressing, downPressing, switchPressing;
/* ACK flag */
bool     upNeedAck,  downNeedAck,  switchNeedAck;

void keyScan()
{
    /* Up Key */
    if (PORT_KEY_UP == 0)
    {
        keyUpTimer++;
        if (keyUpTimer > KEY_100MS)
        {
            if (upPressing == 0)
                upNeedAck = 1;
            upPressing = 1;
        }
    }
    else
    {
        keyUpTimer = 0;
        upPressing = 0;
    }

    /* Down Key */
    if (PORT_KEY_DOWN == 0)
    {
        keyDownTimer++;
        if (keyDownTimer > KEY_100MS)
        {
            if (downPressing == 0)
                downNeedAck = 1;
            downPressing = 0;
        }
    }
    else
    {
        keyDownTimer = 0;
        downPressing = 0;
    }

    /* Switch Key */
    if (PORT_KEY_SWITCH == 0)
    {
        keySwitchTimer++;
        if (keySwitchTimer > KEY_100MS*2)
        {
            if (switchPressing == 0)
                switchNeedAck = 1;
            switchPressing = 0;
        }
    }
    else
    {
        keySwitchTimer = 0;
        switchPressing = 0;
    }
}

uint8_t getKeyValue(void)
{
    if (upNeedAck)
    {
        upNeedAck = 0;
        return KEY_UP;
    }

    if (downNeedAck)
    {
        downNeedAck = 0;
        return KEY_DOWN;
    }

    if (switchNeedAck)
    {
        switchNeedAck = 0;
        return KEY_SWITCH;
    }
}

void flushKeyBuffer(void)
{
    upNeedAck = 0;
    downNeedAck = 0;
    switchNeedAck = 0;
}



















