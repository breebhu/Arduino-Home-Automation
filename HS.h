/*
 * Library for reading from SL-HS-220.
 * The conversion values are taken from the datasheet.
 */
#ifndef HS_H
#define HS_H

#include<Arduino.h>

class HS
{
public:
	HS(byte hPin, byte tPin, int pdResistor);
	int getHumidity();
	int getTemperature();
private:
	static float thermistorTable[80];
	byte humidityPin;
	byte temperaturePin;
	int rawData;
	int voltageEquivalent;
	int pullDownResistor;
	int resistanceEquivalent;
	int temperature;
	int humidity;
};

#endif
