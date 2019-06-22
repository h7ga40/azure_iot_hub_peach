// https://github.com/ms-iotkithol-jp/IoTKitHoLV3/blob/master/PinKitIoTHubApp/PinKitIoTHubApp/PinKit/Accelerometer.cs

#include <stdint.h>
#include <stdbool.h>

#ifndef _ACCELEROMETER_H_
#define _ACCELEROMETER_H_

typedef struct SensorReading
{
	double X;
	double Y;
	double Z;
} SensorReading;

void Accelerometer_Init(int clockRate, int tmout);
bool Accelerometer_TakeMeasurements(SensorReading *result);


#endif // _ACCELEROMETER_H_
