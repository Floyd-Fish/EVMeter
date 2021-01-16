//
// Created by summer on 2020/3/21.
//
#include "stdio.h"
#include "math.h"
#include "fsm.h"
#include "key.h"
#include "ev.h"

#include "ssd1306.h"
#include "tcs34725.h"
#include "opt3001.h"

//  Initialize ev meter struct
evp evm;

void EVMeter_Init()
{
    /* Initialize peripherals */
    SSD1306_Init();
    TCS34725_Init();
    OPT3001_Init();
    HAL_Delay(100);

    /* Initialize EVMeter parameters */
    evm.state = TIME_ADJ_STATE;
    evm.iso_index = 11;                  //  iso = 1000
    evm.iso = ISO[evm.iso_index];
    evm.time_index = 18;                //  time = 1/60s
    evm.time = SHUTTER_F[evm.time_index];
    evm.aperture_index = 3;             //  aperture = F2.8
    evm.aperture = APERTURE[evm.aperture_index];
    evm.color_temp = 2000;              //  color_temp = 2000K
    evm.lux = 0.01;                      //  lux = 0.01;
}

void fsm_main(void)
{
    uint8_t key = getKeyValue();
    flushKeyBuffer();

    //  Update System State
    if (key == KEY_SWITCH)
    {
        if (evm.state < LAST_STATE)
            evm.state++;
        else if (evm.state == LAST_STATE)
            evm.state = FIRST_STATE;
    }

    //  Update EV parameters' Index -> Up key
    if (key == KEY_UP)
    {
        switch (evm.state)
        {
            case TIME_ADJ_STATE:
                if (evm.time_index < SHUTTER_F_SIZE-1) 
                    evm.time_index += 1;

                else if (evm.time_index == SHUTTER_F_SIZE-1) 
                    evm.time_index = 0;

                break;
    
            case APERTURE_ADJ_STATE:
                if (evm.aperture_index < APERTURE_SIZE-1) 
                    evm.aperture_index += 1;

                else if (evm.aperture_index == APERTURE_SIZE-1) 
                    evm.aperture_index = 0;
                break;

            case ISO_ADJ_STATE:
                if (evm.iso_index < ISO_SIZE-1) 
                    evm.iso_index += 1;

                else if (evm.iso_index == ISO_SIZE-1) 
                    evm.iso_index = 0;
                break;

            case EV_ADJ_STATE:
                break;
            
            default:
                break;
        }
    }
    //  Update EV parameters' Index -> Down key
    if (key == KEY_DOWN)
    {
        switch (evm.state)
        {
            case TIME_ADJ_STATE:
                if (evm.time_index > 0) 
                    evm.time_index -= 1;

                else if (evm.time_index == 0) 
                    evm.time_index = SHUTTER_F_SIZE-1;
                break;

            case APERTURE_ADJ_STATE:
                if (evm.aperture_index > 0) 
                    evm.aperture_index -= 1;

                else if (evm.aperture_index == 0) 
                    evm.aperture_index = APERTURE_SIZE-1;
                break;

            case ISO_ADJ_STATE:
                if (evm.iso_index > 0) 
                    evm.iso_index -= 1;

                else if (evm.iso_index == 0) 
                    evm.iso_index = ISO_SIZE-1;
                break;

            case EV_ADJ_STATE:
                break;

            default:
                break;
        }
    }

    // Take a measure
    evm.color_temp = getColorTemp();
    evm.lux = getLuxValue();

    //  Perform calculation and refresh display
    switch (evm.state)
    {
        case TIME_ADJ_STATE:
            timeAdjMode();
            break;

        case APERTURE_ADJ_STATE:
            apertureAdjMode();
            break;

        case ISO_ADJ_STATE:
            isoAdjMode();
            break;

        case EV_ADJ_STATE:
            evAdjMode();
            break;

        default:
            break;
    }
    
}


/*
 * Mode -> Time adjusting
 */
void timeAdjMode()
{
    evm.time = SHUTTER_F[evm.time_index];
    evm.aperture_index = calcAperture(evm.time, evm.iso, evm.lux);
    refreshEVData();

    drawGUI(evm.time_index,
            evm.iso_index,
            evm.aperture_index,
            evm.color_temp,
            evm.lux,
            evm.state);
}

/*
 * Mode -> Aperture adjusting
 */
void apertureAdjMode()
{
    evm.aperture = APERTURE[evm.aperture_index];
    evm.time_index = calcTime(evm.iso, evm.aperture, evm.lux);
    refreshEVData();
    drawGUI(evm.time_index,
            evm.iso_index,
            evm.aperture_index,
            evm.color_temp,
            evm.lux,
            evm.state);
}

/*
 * Mode -> ISO adjusting
 */
void isoAdjMode()
{
    evm.iso = ISO[evm.iso_index];
    evm.time_index = calcTime(evm.iso, evm.aperture, evm.lux);
    refreshEVData();
    drawGUI(evm.time_index,
            evm.iso_index,
            evm.aperture_index,
            evm.color_temp,
            evm.lux,
            evm.state);
}

/*
 * Mode -> EV adjusting
 */
void evAdjMode()
{
    refreshEVData();
    drawGUI(evm.time_index,
            evm.iso_index,
            evm.aperture_index,
            evm.color_temp,
            evm.lux,
            evm.state);
}

void refreshEVData()
{
    evm.aperture = APERTURE[evm.aperture_index];
    evm.iso = ISO[evm.iso_index];
    evm.time = SHUTTER_F[evm.time_index];
}


void drawGUI(uint8_t t_index, uint8_t i_index, uint8_t a_index,
            uint16_t colorTemp, float lux, uint8_t state)
{
    SSD1306_Fill(Black);    //  Clear frame buffer
    char str_buff[20];      //  str buffer to be displayed

    // -> Logo Displaying diagram <-
    SSD1306_SetCursor(94, 0);
    SSD1306_WriteString(" M ", Font_11x18, Black);

    // -> Aperture Displaying diagram <-
    SSD1306_SetCursor(0, 23);
    sprintf(str_buff, "F%.1f", APERTURE[a_index]);
    SSD1306_WriteString(str_buff, Font_11x18, Black);

    // -> ISO Displaying diagram <-
    sprintf(str_buff, " %d", ISO[i_index]);
    SSD1306_DrawFilledRectangle(0,51, 20, 11, White);
    SSD1306_SetCursor(0, 53);
    SSD1306_WriteString("ISO", Font_7x10, Black);
    SSD1306_SetCursor(SSD1306_GetCursorX(), 53);
    SSD1306_WriteString(str_buff, Font_7x10, White);

    // -> SHUTTER TIME Displaying diagram <-
    //  SHUTTER[30] means 1/4 s
    //  SHUTTER[35] means 0.8 s
    //  SHUTTER[36] means 1.0 s
    if(t_index <= 30)
    {
        sprintf(str_buff, "1/%ds ", SHUTTER[t_index]);
    } else if ((t_index > 30) && (t_index <= 35)) {
        sprintf(str_buff, "0.%ds ", SHUTTER[t_index]);
    } else if (t_index == 36) {
        sprintf(str_buff, "%ds   ", SHUTTER[t_index]);
    }
    SSD1306_SetCursor(0, 0);
    SSD1306_WriteString(str_buff, Font_11x18, White);

    // -> colorTemp Displaying diagram <-
    SSD1306_DrawFilledRectangle(92, 51, 35, 11, White);
    SSD1306_SetCursor(92, 53);
    sprintf(str_buff, "%4dK", colorTemp);
    SSD1306_WriteString(str_buff, Font_7x10, Black);

    // -> State display <-
    SSD1306_SetCursor(100, 23);
    if(state == TIME_ADJ_STATE)
        SSD1306_WriteString("T", Font_11x18, White);
    else if (state == APERTURE_ADJ_STATE)
        SSD1306_WriteString("A", Font_11x18, White);
    else if (state == ISO_ADJ_STATE)
        SSD1306_WriteString("I", Font_11x18, White);
    else if (state == EV_ADJ_STATE)
        SSD1306_WriteString("E", Font_11x18, White);

    SSD1306_UpdateScreen(); //  Update framebuffer's content to screen 
}

float getLuxValue(void)
{
    return OPT3001_CalcLux();   //  Read operation with 100ms delay
}

uint16_t getColorTemp(void)
{
    uint16_t r, g, b, c;
    TCS34725_GetRawData(&r, &g, &b, &c);    //  Read operation with 154ms delay
    return TCS34725_CalcColorTemp_DN40(r, g, b, c);
}
