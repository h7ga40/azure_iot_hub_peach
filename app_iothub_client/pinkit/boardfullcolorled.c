// https://github.com/ms-iotkithol-jp/IoTKitHoLV3/blob/master/PinKitIoTHubApp/PinKitIoTHubApp/PinKit/BoardFullColorLED.cs

#include <kernel.h>
#include "boardfullcolorled.h"
#include "gpio_api.h"

// 各 LED が接続されるポート
gpio_t portRed;     // 赤
gpio_t portGreen;   // 緑
gpio_t portBlue;    // 青

// コンストラクター
void BoardFullColorLED_Init()
{
	// 各 LED の InputPort インスタンス
	gpio_init(&portRed, LED_RED);
	gpio_mode(&portRed, PIN_OUTPUT);

	gpio_init(&portGreen, LED_GREEN);
	gpio_mode(&portGreen, PIN_OUTPUT);

	gpio_init(&portBlue, LED_BLUE);
	gpio_mode(&portBlue, PIN_OUTPUT);
}

/// <summary>
/// 指定の色で LED を点灯、消灯する
/// </summary>
/// <param name="redOn">true ならば赤を点灯</param>
/// <param name="greenOn">true ならば緑を点灯</param>
/// <param name="blueOn">true ならば青を点灯</param>
void BoardFullColorLED_SetRgb(bool redOn, bool greenOn, bool blueOn)
{
	gpio_write(&portRed, redOn);
	gpio_write(&portGreen, greenOn);
	gpio_write(&portBlue, blueOn);
}

/// <summary>
/// 色名指定で LED を点灯する
/// </summary>
/// <param name="color"></param>
void BoardFullColorLED_SetColor(Colors color)
{
	int redFlag = (int)color & (int)Colors_Red;
	int greenFlag = (int)color & (int)Colors_Green;
	int blueFlag = (int)color & (int)Colors_Blue;
	gpio_write(&portRed, redFlag != 0);
	gpio_write(&portGreen, greenFlag != 0);
	gpio_write(&portBlue, blueFlag != 0);
}
