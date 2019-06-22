// https://github.com/ms-iotkithol-jp/IoTKitHoLV3/blob/master/PinKitIoTHubApp/PinKitIoTHubApp/PinKit/Temperature.cs

#include <kernel.h>
#include <math.h>
#include "temperature.h"
#include "analogin_api.h"

analogin_t aiThermistor;
// Adjust VR1 - Default 5000Ω
double VR1;

// B constant - Default 3435Ω
double Bc;

void Temperature_Init()
{
	analogin_init(&aiThermistor, A5);
	VR1 = 5000.0;
	Bc = 3435.0;
}

double Temperature_TakeMeasurement()
{
	int raw = analogin_read_u16(&aiThermistor);

	double tk = 273.0;
	double t25 = tk + 25.0;
	double r25 = 10000.0;
	double t = 1.0 / (log(VR1 * raw / (4096 - raw) / r25) / Bc + 1.0 / t25) - tk;
	return t;
}
