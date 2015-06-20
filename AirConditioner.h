#ifndef AIRCONDITIONER_H
#define AIRCONDITIONER_H

#include <Arduino.h>
#include<IRremote.h>
#define TOPBIT 0x800000
#define STATE_ON 1
#define STATE_OFF 0
class AirConditioner
{
  private:
   int MARK_LENGTH;//=600
   int SPACE0;//=470
   int SPACE1;//=1550
   int HMARK;//=4400
   int HSPACE;//=4300
   int GAP;//=5000
   unsigned long*** data;
   unsigned long offData[2];
   byte tempLevels;
   byte fanSpeeds;
   byte frequency;//=38
   byte TEMPERATURE;
   byte FAN_SPEED;
   byte STATE;
   IRsend irsend;
   byte ACpin;
   byte T_MIN;
   byte T_MAX;
  public:
	AirConditioner(int markl,int space0,int space1, int hmark, int hspace, int gap, byte freq,unsigned long*** D,unsigned long offD[],byte tlevels,int fspeeds,byte pin,byte tmax,byte tmin);
 	void sendData(unsigned long data0,unsigned long data1);
	void off();
	void set(byte temperature,byte fspeed);
	byte getTemp();
	byte getFanSpeed();
	byte getState();
	int tempHash(byte t);
};
#endif
