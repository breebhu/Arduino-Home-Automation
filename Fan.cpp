#include "Fan.h"
//#include<Arduino.h>
Fan::Fan(byte mpin, byte* pins, byte numLevel)
{
  mainPin = mpin;
  regulatePins = pins;
  levels = numLevel;
  Fan::off();
  FAN_STATE = (byte)STATE_OFF;
  FAN_SPEED = 0;
  pinMode(mainPin, OUTPUT);
  for (int i = 0; i < numLevel; i++)
  {
    pinMode(regulatePins[i], OUTPUT);
  }
}

Fan::~Fan()
{
}

byte Fan::on()
{
  digitalWrite(mainPin, HIGH);
  FAN_STATE = STATE_ON;
  return (byte)FAN_STATE;	//Is it really necessary to return?
}

byte Fan::off()
{
  digitalWrite(mainPin, LOW);
  FAN_STATE = (byte)STATE_OFF;
  return FAN_STATE;
}

byte Fan::getState()
{
  return FAN_STATE;
}

byte Fan::getSpeed()
{
  return FAN_SPEED;
}

void Fan::regulate(byte speed)
{
  for (int j = 0; j < levels; j++)
  {
    if (j == speed - 1)digitalWrite(regulatePins[j], HIGH);
    else digitalWrite(regulatePins[j], LOW);
  }
  FAN_SPEED = speed;
}
