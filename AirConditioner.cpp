#include "AirConditioner.h"

Airconditoner::AirConditioner(int markl,int space0,int space1, int hmark, int hspace, int gap, byte freq,unsigned long*** D,unsgined long offD[],byte tlevels,intfspeeds,byte pin):MARK_LENGTH(markl),SPACE0(space0),SPACE1(space1),HMARK(hmark),HSPACE(hspace),GAP(gap),FREQUENCY(freq)
{
	data=D;
	offData[0]=offD[0];
	offData[1]=offD[1];
	tempLevels=tlevels;
	fanSpeeds=fspeeds;
	acPin=pin;
	pinMode(acPin,OUTPUT);
	set(24,3);
	delay(1000);
	off();
}
void AirConditioner::set(byte temperature, byte fspeed)
{
	sendData(data[temperature-1][fspeed-1][0],data[temperature-1][fspeed-1][1]);
	TEMPERATURE=temperature;
	FAN_SPEED=fspeed;
	STATE=ON;
}
void AirConditioner::sendData(unsigned long data0,unsigned long data01)
{
  irsend.enableIROut(FREQUENCY);
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
  irsend.space(0);
}
void AirConditioner::off()
{
   sendData(offData[0],offData[1]);
   STATE=OFF;
}
byte Airconditioner::getTemp()
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