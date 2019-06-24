/*
 *  PinKitセンサータスク
 */

#include "pinkit.h"
#include <math.h>

pinkit_t pinkit;

void pinkit_task(intptr_t exinf)
{
	Accelerometer_Init(100, 1000);
	Temperature_Init();
	BoardFullColorLED_Init();
	LightSensor_Init();

	for (;;) {
		Accelerometer_TakeMeasurements(&pinkit.accel);
		BoardFullColorLED_SetRgb(fabs(pinkit.accel.X) > 0.5, fabs(pinkit.accel.Y) > 0.5, fabs(pinkit.accel.Z) > 0.5);
		pinkit.temperature = Temperature_TakeMeasurement();
		pinkit.humidity = LightSensor_TakeMeasurement();

		dly_tsk(1000);
	}
}

