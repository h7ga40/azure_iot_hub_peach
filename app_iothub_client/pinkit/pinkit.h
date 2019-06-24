/*
 *  PinKitセンサータスク
 */

#ifndef _PINKIT_H_
#define _PINKIT_H_

#include <kernel.h>
#include "accelerometer.h"
#include "temperature.h"
#include "lightsensor.h"
#include "boardfullcolorled.h"

#define PINKIT_PRIORITY			5		/* PinKitタスクの優先度 */

#define PINKIT_STACK_SIZE		1024	/* PinKitタスクのスタック領域のサイズ */

typedef struct pinkit_t {
	SensorReading accel;
	double temperature;
	double humidity;
} pinkit_t;

extern pinkit_t pinkit;

extern void pinkit_task(intptr_t exinf);

#endif // _PINKIT_H_
