#include "Light.h"
Light::Light(byte mpin,byte* pins,byte numLevel)
{
	Light::mainPin=mpin;
        dimPins=pins;
	levels=numLevel;
	Light::off();
        LIGHT_STATE=(byte)STATE_OFF;
        DIM_LEVEL=0;
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
   for(int j=0;j<levels;j++)
   {
	if(j==level)digitalWrite(dimPins[j],HIGH);
	else digitalWrite(dimPins[j],LOW);
   }
   DIM_LEVEL=level;
}
