#include "HS.h"

//Thermistor table from datasheet
float HS::thermistorTable[80] =
{ 32.74, 31.11, 29.57, 28.12, 26.74, 25.45, 24.22, 23.06, 21.96, 20.92, 19.94, 19.00, 18.12, 17.28, 16.48, 15.73, 15.02, 14.34, 13.69, 13.08,
12.50, 11.95, 11.42, 10.92, 10.45, 10, 9.57, 9.16, 8.77, 8.40, 8.05, 7.71, 7.39, 7.09, 6.80, 6.52, 6.26, 6.00, 5.76, 5.53,
5.31, 5.10, 4.90, 4.71, 4.53, 4.35, 4.19, 4.03, 3.87, 3.73, 3.59, 3.45, 3.32, 3.20, 3.08, 2.97, 2.86, 2.76, 2.66, 2.57,
2.47, 2.39, 2.30, 2.22, 2.15, 2.07, 2, 1.93, 1.86, 1.80, 1.74, 1.68, 1.62, 1.57, 1.52, 1.47, 1.42, 1.37, 1.33, 1.29 };

HS::HS(byte hPin, byte tPin, int pdResistor)
{
	humidityPin = hPin;
	temperaturePin = tPin;
	rawData = 0;
	voltageEquivalent = 0;
	pullDownResistor = pdResistor;
	resistanceEquivalent = 0;
	temperature = 0;
	humidity = 0;
}

int HS::getHumidity()
{
	rawData = analogRead(humidityPin);
	voltageEquivalent = map(rawData, 0, 1023, 0, 5000);
	humidity = map(voltageEquivalent, 660, 3135, 20, 95);	//Values from datasheet
	return humidity;
}

int HS::getTemperature()
{
	rawData = analogRead(temperaturePin);
	voltageEquivalent = map(rawData, 0, 1023, 0, 5000);
	resistanceEquivalent = voltageEquivalent*pullDownResistor / (5000 - voltageEquivalent);
	
	while (resistanceEquivalent < thermistorTable[temperature])
		temperature++;

	return temperature;
}