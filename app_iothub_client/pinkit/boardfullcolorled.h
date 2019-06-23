// https://github.com/ms-iotkithol-jp/IoTKitHoLV3/blob/master/PinKitIoTHubApp/PinKitIoTHubApp/PinKit/BoardFullColorLED.cs

#include <stdint.h>
#include <stdbool.h>

#ifndef _BOARD_FULL_COLOR_LED_H_
#define _BOARD_FULL_COLOR_LED_H_

typedef enum Colors
{
	Colors_Black = 0,
	Colors_Red = 1,
	Colors_Green = 2,
	Colors_Yellow = 3,
	Colors_Blue = 4,
	Colors_Magenta = 5,
	Colors_Cyan = 6,
	Colors_White = 7
} Colors;

void BoardFullColorLED_Init();
void BoardFullColorLED_SetRgb(bool redOn, bool greenOn, bool blueOn);
void BoardFullColorLED_SetColor(Colors color);

#endif // _BOARD_FULL_COLOR_LED_H_
