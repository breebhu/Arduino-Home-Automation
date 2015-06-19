#include "Fan.h"
Fan::Fan(byte mpin,byte* pins,byte numLevel)
{
	Fan::mainPin=mpin;
        regulatePins=pins;
	levels=numLevel;
	Fan::off();
        FAN_STATE=(byte)STATE_OFF;
        FAN_SPEED=0;
}

Fan::~Fan()
{
}

byte Fan::on()
{
   digitalWrite(mainPin,HIGH);
   FAN_STATE=STATE_ON;
   return (byte)FAN_STATE;
}

byte Fan::off()
{
   digitalWrite(mainPin,LOW);
   FAN_STATE=(byte)STATE_OFF;
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
   for(int j=0;j<levels;j++)
   {
	if(j==speed)digitalWrite(regulatePins[j],HIGH);
	else digitalWrite(regulatePins[j],LOW);
   }
   FAN_SPEED=speed;
}
