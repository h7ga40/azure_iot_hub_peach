// https://github.com/ms-iotkithol-jp/IoTKitHoLV3/blob/master/PinKitIoTHubApp/PinKitIoTHubApp/PinKit/BoardFullColorLED.cs

#include <kernel.h>
#include "boardfullcolorled.h"
#include "pwmout_api.h"

// 各 LED が接続されるポート
pwmout_t portRed;     // 赤
pwmout_t portGreen;   // 緑
pwmout_t portBlue;    // 青

// コンストラクター
void BoardFullColorLED_Init()
{
	// 各 LED の InputPort インスタンス
	pwmout_init(&portRed, LED_RED);
	pwmout_period_ms(&portRed, 2); // 500Hz

	pwmout_init(&portGreen, LED_GREEN);
	pwmout_period_ms(&portGreen, 2); // 500Hz

	pwmout_init(&portBlue, LED_BLUE);
	pwmout_period_ms(&portBlue, 2); // 500Hz
}

/// <summary>
/// 指定の色で LED を点灯、消灯する
/// </summary>
/// <param name="redOn">true ならば赤を点灯</param>
/// <param name="greenOn">true ならば緑を点灯</param>
/// <param name="blueOn">true ならば青を点灯</param>
void BoardFullColorLED_SetRgb(float redOn, float greenOn, float blueOn)
{
	pwmout_write(&portRed, redOn);
	pwmout_write(&portGreen, greenOn);
	pwmout_write(&portBlue, blueOn);
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
	pwmout_write(&portRed, (redFlag != 0) ? 1.0 : 0.0);
	pwmout_write(&portGreen, (greenFlag != 0) ? 1.0 : 0.0);
	pwmout_write(&portBlue, (blueFlag != 0) ? 1.0 : 0.0);
}
