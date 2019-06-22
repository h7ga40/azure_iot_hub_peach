// https://github.com/ms-iotkithol-jp/IoTKitHoLV3/blob/master/PinKitIoTHubApp/PinKitIoTHubApp/PinKit/LightSensor.cs

#include <stdint.h>
#include <stdbool.h>

#ifndef _LIGHT_SENSOR_H_
#define _LIGHT_SENSOR_H_

void LightSensor_Init();
double LightSensor_TakeMeasurement();

#endif // _LIGHT_SENSOR_H_
