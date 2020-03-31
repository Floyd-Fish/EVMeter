#include "ssd1306.h"
#include <string.h>
#include "ssd1306_tests.h"

void SSD1306_TestBorder() {
    SSD1306_Fill(Black);
   
    uint32_t start = HAL_GetTick();
    uint32_t end = start;
    uint8_t x = 0;
    uint8_t y = 0;
    do {
        SSD1306_DrawPixel(x, y, Black);

        if((y == 0) && (x < 127))
            x++;
        else if((x == 127) && (y < 63))
            y++;
        else if((y == 63) && (x > 0)) 
            x--;
        else
            y--;

        SSD1306_DrawPixel(x, y, White);
        SSD1306_UpdateScreen();
    
        HAL_Delay(5);
        end = HAL_GetTick();
    } while((end - start) < 8000);
   
    HAL_Delay(1000);
}

void SSD1306_TestFonts() {
    SSD1306_Fill(Black);
    SSD1306_SetCursor(2, 0);
    SSD1306_WriteString("Font 16x26", Font_16x26, White);
    SSD1306_SetCursor(2, 26);
    SSD1306_WriteString("Font 11x18", Font_11x18, White);
    SSD1306_SetCursor(2, 26+18);
    SSD1306_WriteString("Font 7x10", Font_7x10, White);
    SSD1306_UpdateScreen();
}

void SSD1306_TestFPS() {
    SSD1306_Fill(White);
   
    uint32_t start = HAL_GetTick();
    uint32_t end = start;
    int fps = 0;
    char message[] = "ABCDEFGHIJK";
   
    SSD1306_SetCursor(2,0);
    SSD1306_WriteString("Testing...", Font_11x18, Black);
   
    do {
        SSD1306_SetCursor(2, 18);
        SSD1306_WriteString(message, Font_11x18, Black);
        SSD1306_UpdateScreen();
       
        char ch = message[0];
        memmove(message, message+1, sizeof(message)-2);
        message[sizeof(message)-2] = ch;

        fps++;
        end = HAL_GetTick();
    } while((end - start) < 5000);
   
    HAL_Delay(1000);

    char buff[64];
    fps = (float)fps / ((end - start) / 1000.0);
    snprintf(buff, sizeof(buff), "~%d FPS", fps);
   
    SSD1306_Fill(White);
    SSD1306_SetCursor(2, 18);
    SSD1306_WriteString(buff, Font_11x18, Black);
    SSD1306_UpdateScreen();
}

void SSD1306_TestAll() {
    SSD1306_Init();
    SSD1306_TestFPS();
    HAL_Delay(3000);
    SSD1306_TestBorder();
    SSD1306_TestFonts();
}
