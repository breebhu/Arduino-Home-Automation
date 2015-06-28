/*
 * Library for air conditioner control.
 * Works when IR signal is sent twice.
 * Needs to be provided with the values of marks and spaces of the IR signal
 * Uses IRremote library to send the signal.
 * Output pin is pin 3 for UNO, pin 9 for MEGA
 */

#ifndef AIRCONDITIONER_H
#define AIRCONDITIONER_H

#include <Arduino.h>
#include "IRremote.h"
#define TOPBIT 0x800000
#define STATE_ON 1
#define STATE_OFF 0
class AirConditioner
{
  private:
    //Data for IR signal
    int MARK_LENGTH;//=600
    int SPACE0;//=470
    int SPACE1;//=1550
    int HMARK;//=4400
    int HSPACE;//=4300
    int GAP;//=5000
    byte frequency;//=38

    //Class for sending IR signal
    IRsend irsend;

    //AC codes; each code split into 2 parts since it is too large for unsigned long
    unsigned long*** data;
    unsigned long offData[2];

    //Limits of AC
    byte tempLevels;
    byte fanSpeeds;
    byte T_MIN;
    byte T_MAX;

    //Current states
    byte TEMPERATURE;
    byte FAN_SPEED;
    byte STATE;

    byte ACpin;

  public:
    AirConditioner(int markl, int space0, int space1, int hmark, int hspace, int gap, byte freq, unsigned long*** D, unsigned long offD[], byte tlevels, int fspeeds, byte pin, byte tmax, byte tmin);
    void sendData(unsigned long data0, unsigned long data1);
    void off();
    void set(byte temperature, byte fspeed);
    byte getTemp();
    byte getFanSpeed();
    byte getState();
    int tempHash(byte t);
};
#endif
