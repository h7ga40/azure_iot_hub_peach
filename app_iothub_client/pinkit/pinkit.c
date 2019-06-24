/*
 *  PinKitセンサータスク
 */

#include "pinkit.h"
#include <math.h>
#include "gpio_api.h"
#include "syssvc/syslog.h"

pinkit_t pinkit;

gpio_t user_led;

void pinkit_task(intptr_t exinf)
{
#if SENSOR_DEBUG
	int iterator = 0;
#endif
	gpio_init(&user_led, LED_USER);
	gpio_mode(&user_led, PIN_OUTPUT);

	Accelerometer_Init(100, 1000);
	Temperature_Init();
	BoardFullColorLED_Init();
	LightSensor_Init();

	for (;;) {
		Accelerometer_TakeMeasurements(&pinkit.accel);
		pinkit.temperature = Temperature_TakeMeasurement();
		pinkit.humidity = LightSensor_TakeMeasurement();

		BoardFullColorLED_SetRgb(fabs(pinkit.accel.X) > 0.5, fabs(pinkit.accel.Y) > 0.5, fabs(pinkit.accel.Z) > 0.5);
		gpio_write(&user_led, pinkit.temperature > 28.0);
#if SENSOR_DEBUG
		iterator++;
		if (iterator >= 100){
			char text[80];
			iterator = 0;
			sprintf(text, "x:%0.3f, y:%0.3f, z:%0.3f, temp:%0.1f", pinkit.accel.X, pinkit.accel.Y, pinkit.accel.Z, pinkit.temperature);
			syslog(LOG_NOTICE, text);
		}
#endif
		dly_tsk(1000);
	}
}

