/*
 * Class for fan control.
 * Sets the pins to high as required.
 * The setup could be either a main pin for on/off and pins for speed levels or pins for only speed levels (all pins low means off). The functions take care of it.
 */

#ifndef FAN_H
#define FAN_H
#include <Arduino.h>
#define STATE_ON 1
#define STATE_OFF 0
class Fan
{
  private:
    byte mainPin;
    byte* regulatePins;
    byte levels;
    byte FAN_STATE;
    byte FAN_SPEED;
  public:
    Fan(byte mPin, byte* pins, byte numLevel);
    Fan(byte* pins, byte numLevel);
    ~Fan();
    byte getState();
    byte getSpeed();
    byte on();
    byte off();
    void regulate(byte speed);
};
#endif
