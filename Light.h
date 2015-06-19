#ifndef LIGHT_H
#define LIGHT_H
#include <Arduino.h>
#define STATE_ON 1
#define STATE_OFF 0 
class Light
{
private:
   byte mainPin;
   byte* dimPins;
   byte LIGHT_STATE;
   byte DIM_LEVEL;   
   byte levels; 
public:
   Light(byte mPin,byte* pins,byte numLevel);
   ~Light();
   byte getState();
   byte getDimLevel();
   byte on();
   byte off();
   void dim(byte level);	
};
#endif