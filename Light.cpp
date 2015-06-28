#include "Light.h"
Light::Light(byte mpin, byte* pins, byte numLevel)
{
  mainPin = mpin;
  dimPins = pins;
  levels = numLevel;
  Light::off();
  LIGHT_STATE = (byte)STATE_OFF;
  BRIGHT_LEVEL = 0;
  pinMode(mainPin, OUTPUT);
  for (int i = 0; i < numLevel; i++)
    pinMode(dimPins[i], OUTPUT);
}
Light::Light(byte* pins, byte numLevel)
{
  mainPin = pins[numLevel - 1];
  dimPins = pins;
  levels = numLevel;
  Light::off();
  LIGHT_STATE = (byte)STATE_OFF;
  BRIGHT_LEVEL = 0;
  pinMode(mainPin, OUTPUT);
  for (int i = 0; i < numLevel; i++)
    pinMode(dimPins[i], OUTPUT);
}
Light::~Light()
{
}

byte Light::on()
{
  dim(levels);
  digitalWrite(mainPin, HIGH);
  return (byte)LIGHT_STATE;
}

byte Light::off()
{
  for (int j = 0; j < levels; j++)digitalWrite(dimPins[j], LOW);
  digitalWrite(mainPin, LOW);
  LIGHT_STATE = (byte)STATE_OFF;
  return LIGHT_STATE;
}

byte Light::getState()
{
  return LIGHT_STATE;
}

byte Light::getBrightLevel()
{
  return BRIGHT_LEVEL;
}

void Light::dim(byte level)
{
  if (level < 1)off();
  else
  {
    if (mainPin != dimPins[levels - 1])digitalWrite(mainPin, HIGH);
    for (int j = 0; j < levels; j++)
    {
      if (j == level - 1)digitalWrite(dimPins[j], HIGH);
      else digitalWrite(dimPins[j], LOW);
    }
    BRIGHT_LEVEL = level;
    LIGHT_STATE = STATE_ON;
  }
}
