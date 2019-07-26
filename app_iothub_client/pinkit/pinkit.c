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
		gpio_write(&user_led, pinkit.ledOn);

		dly_tsk(10000);
	}
}

