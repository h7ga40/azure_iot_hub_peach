// https://github.com/ms-iotkithol-jp/IoTKitHoLV3/blob/master/PinKitIoTHubApp/PinKitIoTHubApp/PinKit/LightSensor.cs

#include <kernel.h>
#include "lightsensor.h"
#include "analogin_api.h"

/// This library use GHI Electronics's LightSense sensor - https://www.ghielectronics.com/catalog/product/336
/// Please connect the sensor to Socket 1
/// When you want to use this sensor please define 'USE_LIGHTSENSE' macro in Build property of this project
static analogin_t aiChannel;
analogin_t aiLightSense;

void LightSensor_Init()
{
	analogin_init(&aiLightSense, A0);
}

/// <summary>
/// Most dark - 0.0
/// Most bright - 1.0
/// </summary>
/// <returns></returns>
double LightSensor_TakeMeasurement()
{
	int raw = analogin_read_u16(&aiLightSense);

	return (double)raw / 4096.0;
}
