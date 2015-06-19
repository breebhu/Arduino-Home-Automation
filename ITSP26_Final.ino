#include <Light.h>

#include <Fan.h>


const byte ON=1;
const byte OFF=0;
const byte AUTO_MODE=31;
const byte MANUAL_MODE=27;
byte MODE;
byte AC_STATE;
byte NUM_PERSONS;
boolean NO_MOTION=false;
byte LIGHT_INTENSITY;
byte TEMPERATURE;
byte HUMIDITY;
byte FAN_CUTOFF=28;
byte AC_CUTOFF=24;
byte AC_AMBIENT_TEMP;
byte DEFAULT_AC_FAN_SPEED;
boolean SWING_PREFERENCE=true; 
const byte T5=24,T4=20,T3=16,T2=14,T1=12;
const byte LIGHT_THRESHOLD=500;
const byte L1=0,L2=0,L3=0;
byte* a=new byte[3];
extern Fan* F1=new Fan(2,a,3);
extern Light* L=new Light(1,a,3);
void setup()
{
   //start the system
   //initialize devices
   //while uploading sync time to PC
   
   
}
void loop()
{
  //check if auto mode or manual mode from website and update accordingly
  if(MODE==AUTO_MODE)
  {
    //read data from sensors
    readSensorData();
    //check state of devices based on present data
    if(TEMPERATURE>FAN_CUTOFF)
    {
      if(F1->getState()!=OFF)
      {
        F1->off();
      }
      if(AC_STATE!=ON)acON();
      setAC(AC_AMBIENT_TEMP,DEFAULT_AC_FAN_SPEED,SWING_PREFERENCE);
    }
    else if(TEMPERATURE<AC_CUTOFF)
    {
      acOFF();
      if(TEMPERATURE>T5)
      F1->regulate(5);
      else if(TEMPERATURE>T4)
      F1->regulate(4);
      else if(TEMPERATURE>T3)
      F1->regulate(3);
      else if(TEMPERATURE>T2)
      F1->regulate(2);
      else if(TEMPERATURE>T1)
      F1->regulate(1);
      else F1->off();
      
    }
    if(LIGHT_INTENSITY<LIGHT_THRESHOLD)
    {
      if(LIGHT_INTENSITY<L1)
      L->dim(1);
      else if(LIGHT_INTENSITY<L2)
      L->dim(2);
      else if(LIGHT_INTENSITY<L3)
      L->dim(3);
      L->on();
    }
    else
    {
      L->off();
    }
    
  }
  else
  {
    //in manual mode, read data from user webpage and modify things accordingly
  }
}

void readSensorData()
{
  
}
int setAC(int temp,int fan,boolean swing)
{
}
int acON()
{
}
int acOFF()
{
}
