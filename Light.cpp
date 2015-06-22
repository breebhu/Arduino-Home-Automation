#include "Light.h"
//#include <Arduino.h>
Light::Light(byte mpin,byte* pins,byte numLevel)
{
	mainPin=mpin;
        dimPins=pins;
	levels=numLevel;
	Light::off();
        LIGHT_STATE=(byte)STATE_OFF;
        DIM_LEVEL=0;
        pinMode(mainPin,OUTPUT);
        for(int i=0;i<numLevel;i++)
        	pinMode(dimPins[i],OUTPUT);
}

Light::~Light()
{
}

byte Light::on()
{
   digitalWrite(mainPin,HIGH);
   LIGHT_STATE=STATE_ON;
   return (byte)LIGHT_STATE;
}

byte Light::off()
{
   digitalWrite(mainPin,LOW);
   LIGHT_STATE=(byte)STATE_OFF;
   return LIGHT_STATE;
}

byte Light::getState()
{
   return LIGHT_STATE;
}

byte Light::getDimLevel()
{
   return DIM_LEVEL;
}

void Light::dim(byte level)
{
   for(int j=1;j<=levels;j++)
   {
	if(j==level)digitalWrite(dimPins[j-1],HIGH);
	else digitalWrite(dimPins[j-1],LOW);
   }
   DIM_LEVEL=level;
}
