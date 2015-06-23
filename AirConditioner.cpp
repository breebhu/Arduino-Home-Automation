#include "AirConditioner.h"

AirConditioner::AirConditioner(int markl,int space0,int space1, int hmark, int hspace, int gap, byte freq,unsigned long*** D,unsigned long offD[],byte tlevels,int fspeeds,byte pin,byte tmax,byte tmin):MARK_LENGTH(markl),SPACE0(space0),SPACE1(space1),HMARK(hmark),HSPACE(hspace),GAP(gap),frequency(freq),ACpin(pin),T_MAX(tmax),T_MIN(tmin)
{
	data=D;
	offData[0]=offD[0];
	offData[1]=offD[1];
	tempLevels=tlevels;
	fanSpeeds=fspeeds;
	pinMode(ACpin,OUTPUT);
	set(24,3);
	delay(1000);
	off();
}
void AirConditioner::set(byte temperature, byte fspeed)
{
	sendData(data[tempHash(temperature)][fspeed-1][0],data[tempHash(temperature)][fspeed-1][1]);
	TEMPERATURE=temperature;
	FAN_SPEED=fspeed;
	STATE=STATE_ON;
}
void AirConditioner::sendData(unsigned long data0,unsigned long data01)
{
  irsend.enableIROut(frequency);
  unsigned long data02=data0;
  unsigned long data03=data01;
  irsend.mark(HMARK);
  irsend.space(HSPACE);
  for (int i = 0; i < 24; i++) {
    if (data0 & TOPBIT) {
      irsend.mark(MARK_LENGTH);  
      irsend.space(SPACE1);  
    } 
    else {
      irsend.mark(MARK_LENGTH);
      irsend.space(SPACE0);  
    }
    data0 <<= 1;
  }
  for (int i = 0; i < 24; i++) {
    if (data01 & TOPBIT) {
      irsend.mark(MARK_LENGTH);  
      irsend.space(SPACE1);  
    } 
    else {
         irsend.mark(MARK_LENGTH);
      irsend.space(SPACE0);  
    }
    data01 <<= 1;
  }
  irsend.mark(MARK_LENGTH);
  
  irsend.space(GAP);
  
  irsend.mark(HMARK);
  irsend.space(HSPACE);
  for (int i = 0; i < 24; i++) {
    if (data02 & TOPBIT) {
      irsend.mark(MARK_LENGTH);  
      irsend.space(SPACE1);  
    } 
    else {
      irsend.mark(MARK_LENGTH);
      irsend.space(SPACE0);  
    }
    data02 <<= 1;
  }
  for (int i = 0; i < 24; i++) {
    if (data03 & TOPBIT) {
      irsend.mark(MARK_LENGTH);  
      irsend.space(SPACE1);  
    } 
    else {
      irsend.mark(MARK_LENGTH);
      irsend.space(SPACE0);  
    }
    data03 <<= 1;
  }
  irsend.mark(MARK_LENGTH);
  irsend.space(0);
}
void AirConditioner::off()
{
   sendData(offData[0],offData[1]);
   STATE=STATE_OFF;
}
byte AirConditioner::getTemp()
{
    return TEMPERATURE;
}
byte AirConditioner::getFanSpeed()
{
   return  FAN_SPEED;
}
byte AirConditioner::getState()
{
   return STATE;
}
int AirConditioner::tempHash(byte t)
{
    return (byte)(t-T_MIN);
}
