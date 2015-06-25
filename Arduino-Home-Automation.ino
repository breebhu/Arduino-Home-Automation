#include "Light.h"
#include "AirConditioner.h"
#include "Fan.h"
#include <SPI.h>
#include <Ethernet.h>
#include <dht.h>
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
boolean clientConnected = false;

//HTTP requests
char page[150];
char value[5];
int index = 0;

const byte ON = 1;
const byte OFF = 0;

//Modes
const byte AUTO_MODE = 31;
const byte MANUAL_MODE = 27;
volatile byte MODE = AUTO_MODE;

//Sensors
const byte COUNT_VAL=25;
dht DHT;
volatile byte NUM_PERSONS = 0;
boolean NO_MOTION = false;
int LIGHT_INTENSITY = 0;
int rawLightIntensity[4];
int rawLightIntensity_T[4];
int TEMPERATURE_T=0;
byte numDHTReadings=0;
byte numLDRReadings=0;
long measureTime=0;
byte TEMPERATURE = 0;
byte HUMIDITY = 0;
int HUMIDITY_T;

boolean SENSOR_INITIALIZED=false;

//Auto mode preferences
byte FAN_CUTOFF = 28; //temperature above which fan is switched off and AC is switched on
byte AC_CUTOFF = 24; //temperature below which AC is switched off and fan is switched on
byte AC_AMBIENT_TEMP = 22;
byte DEFAULT_AC_FAN_SPEED = 1;

//levels for fan and light; not constant since user can set it through auto mode settings
byte T5 = 24, T4 = 20, T3 = 16, T2 = 14, T1 = 12; //Denotes lower level of range,i.e. 5th level for T>T5,needs to be set high for final demo since heating the sensor is easier than cooling
byte H1 = 50, H2 = 60, H3 = 80;	//Humidity levels for fan speed. Since we don't use humidity much, this seemed like the best place to put it
int LIGHT_THRESHOLD = 600;
int L1 = 500, L2 = 400;	//Since only 3 brightness levels

//declare pins to be used in circuit
byte* fanRegulatePins = new byte[3];
byte* lightRegulatePins = new byte[3];
const byte laserPin1 = 2, laserPin2 = 3;
//pushButton pins
const byte pushButtonFan = 21;
const byte pushButtonLight = 20;
//DHT pin
const byte DHT22_PIN=22;
//pins for LDR
byte LDR_PIN[4];


//create instances of devices
Light* L;
Fan* F1;
AirConditioner* AC1;

//intialze variables related to laser person count
volatile byte laser[2][5];
volatile boolean detected = false;
boolean flag = true;
volatile byte count = 0;
volatile byte prevState[2];

//struct for DHT22 sensor data
struct
{
    uint32_t total;
    uint32_t ok;
    uint32_t crc_error;
    uint32_t time_out;
    uint32_t connect;
    uint32_t ack_l;
    uint32_t ack_h;
    uint32_t unknown;
} stat = { 0,0,0,0,0,0,0,0};

void setup()
{
  //start the system
  //initialize devices
  //while uploading sync time to PC
  fanRegulatePins[0] = 27; //Initialize pins
  fanRegulatePins[1] = 28;
  fanRegulatePins[2] = 29;
  lightRegulatePins[0] = 23;
  lightRegulatePins[1] = 24;
  lightRegulatePins[2] = 25;
  
  //declare LDR pins and set sensor pins to input mode
  
  unsigned long*** data = new unsigned long**[14];
  for (int j = 0; j < 14; j++)
  {
    data[j] = new unsigned long*[3];
    for (int k = 0; k < 3; k++)data[j][k] = new unsigned long[2];
  }
  // data for TL AC
  data[0][0][0] = 0xB24D9F;
  data[0][0][1] = 0x6000FF;
  data[0][1][0] = 0xB24D5F;
  data[0][1][1] = 0xA000FF;
  data[0][2][0] = 0xB24D3F;
  data[0][2][1] = 0xC000FF;
  data[1][0][0] = 0xB24D9F;
  data[1][0][1] = 0x6010EF;
  data[1][1][0] = 0xB24D5F;
  data[1][1][1] = 0xA010EF;
  data[1][2][0] = 0xB24D3F;
  data[1][2][1] = 0xC010EF;
  data[2][0][0] = 0xB24D9F;
  data[2][0][1] = 0x6030CF;
  data[2][1][0] = 0xB24D5F;
  data[2][1][1] = 0xA030CF;
  data[2][2][0] = 0xB24D3F;
  data[2][2][1] = 0xC030CF;
  data[3][0][0] = 0xB24D9F;
  data[3][0][1] = 0x6020DF;
  data[3][1][0] = 0xB24D5F;
  data[3][1][1] = 0xA020DF;
  data[3][2][0] = 0xB24D3F;
  data[3][2][1] = 0xC020DF;
  data[4][0][0] = 0xB24D9F;
  data[4][0][1] = 0x60609F;
  data[4][1][0] = 0xB24D5F;
  data[4][1][1] = 0xA0609F;
  data[4][2][0] = 0xB24D3F;
  data[4][2][1] = 0xC0609F;
  data[5][0][0] = 0xB24D9F;
  data[5][0][1] = 0x60708F;
  data[5][1][0] = 0xB24D5F;
  data[5][1][1] = 0xA0708F;
  data[5][2][0] = 0xB24D3F;
  data[5][2][1] = 0xC0708F;
  data[6][0][0] = 0xB24D9F;
  data[6][0][1] = 0x6050AF;
  data[6][1][0] = 0xB24D5F;
  data[6][1][1] = 0xA050AF;
  data[6][2][0] = 0xB24D3F;
  data[6][2][1] = 0xC050AF;
  data[7][0][0] = 0xB24D9F;
  data[7][0][1] = 0x6040BF;
  data[7][1][0] = 0xB24D5F;
  data[7][1][1] = 0xA040BF;
  data[7][2][0] = 0xB24D3F;
  data[7][2][1] = 0xC040BF;
  data[8][0][0] = 0xB24D9F;
  data[8][0][1] = 0x60C03F;
  data[8][1][0] = 0xB24D5F;
  data[8][1][1] = 0xA0C03F;
  data[8][2][0] = 0xB24D3F;
  data[8][2][1] = 0xC0C03F;
  data[9][0][0] = 0xB24D9F;
  data[9][0][1] = 0x60D02F;
  data[9][1][0] = 0xB24D5F;
  data[9][1][1] = 0xA0D02F;
  data[9][2][0] = 0xB24D3F;
  data[9][2][1] = 0xC0D02F;
  data[10][0][0] = 0xB24D9F;
  data[10][0][1] = 0x60906F;
  data[10][1][0] = 0xB24D5F;
  data[10][1][1] = 0xA0906F;
  data[10][2][0] = 0xB24D3F;
  data[10][2][1] = 0xC0906F;
  data[11][0][0] = 0xB24D9F;
  data[11][0][1] = 0x60807F;
  data[11][1][0] = 0xB24D5F;
  data[11][1][1] = 0xA0807F;
  data[11][2][0] = 0xB24D3F;
  data[11][2][1] = 0xC0807F;
  data[12][0][0] = 0xB24D9F;
  data[12][0][1] = 0x60A05F;
  data[12][1][0] = 0xB24D5F;
  data[12][1][1] = 0xA0A05F;
  data[12][2][0] = 0xB24D3F;
  data[12][2][1] = 0xC0A05F;
  data[13][0][0] = 0xB24D9F;
  data[13][0][1] = 0x60B04F;
  data[13][1][0] = 0xB24D5F;
  data[13][1][1] = 0xA0B04F;
  data[13][2][0] = 0xB24D3F;
  data[13][2][1] = 0xC0B04F;

  unsigned long* offData = new unsigned long[2];
  offData[0] = 0xB24D7B;
  offData[1] = 0x84E01F;
  F1 = new Fan(26, fanRegulatePins, 3);
  L = new Light(22, lightRegulatePins, 3);
  AC1 = new AirConditioner(600, 470, 1550, 4400, 4300, 5000, 38, data, offData, 2, 3, 9, 24, 25);
  // start the Ethernet connection and the server:
  Ethernet.begin(mac);
  server.begin();
  attachInterrupt(2, switchFan, CHANGE);
  attachInterrupt(3, switchLight, CHANGE);
  delay(2000);
}
void loop()
{
  if (flag)
  {

    if (digitalRead(laserPin1) == 1 && digitalRead(laserPin2) == 1)
    {
      laser[0][0] = 1;
      laser[1][0] = 1;
      count++;
      prevState[0] = 1;
      prevState[1] = 1;
      attachInterrupt(0, detectLaser1, CHANGE);
      attachInterrupt(1, detectLaser2, CHANGE);
      flag = false;
    }
  }
  //read data from sensors
  readSensorData();
  //check if auto mode or manual mode from website and update accordingly
  if (MODE == AUTO_MODE)
  {
    //check state of devices based on present data
    if (NUM_PERSONS > 0)
    {
      if (TEMPERATURE > FAN_CUTOFF)
      {
        if (F1->getState() != OFF)
          F1->off();
        if (AC1->getState() != ON || AC1->getTemp() != AC_AMBIENT_TEMP || AC1->getFanSpeed() != DEFAULT_AC_FAN_SPEED)
          AC1->set(AC_AMBIENT_TEMP, DEFAULT_AC_FAN_SPEED);

      }
      else
      {
        if (TEMPERATURE < AC_CUTOFF)
        {
          if (AC1->getState() == ON)
            AC1->off();
        }

        if ((TEMPERATURE > T1 || HUMIDITY > H1) && AC1->getState() == OFF)
        {
          if (HUMIDITY > H3||TEMPERATURE>T3)
            F1->regulate(3);
          else if ((HUMIDITY > H2&&TEMPERATURE>T3)||(HUMIDITY>H1&&TEMPERATURE>T3))
            F1->regulate(2);
          else F1->regulate(1);
        }
        else
        {
          if (F1->getState() != OFF)
            F1->off();
        }
      }

      if (LIGHT_INTENSITY < LIGHT_THRESHOLD) //dim(1) means highly dim, dim(3) is very bright
      {
        L->on();
        if (LIGHT_INTENSITY < L2)
          L->dim(3);
        else if (LIGHT_INTENSITY < L1)
          L->dim(2);
        else
          L->dim(1);
      }
      else
      {
        L->off();
      }
    }
    else
    {
      AC1->off();
      F1->off();
      L->off();
    }

  }

  handleWebRequest();
}

void readSensorData()
{
    
    noInterrupts();
    if(millis()-measureTime>2000)
    {
      int chk = DHT.read22(DHT22_PIN);
      stat.total++;
      switch (chk)
      {
      case DHTLIB_OK:
          stat.ok++;
          //Serial.print("OK,\t");
          if(SENSOR_INITIALIZED)
          {
            if(numDHTReadings<COUNT_VAL)
            {
              TEMPERATURE_T+=DHT.temperature;
              HUMIDITY_T+=DHT.humidity;
              numDHTReadings++;
            }
            else if(numDHTReadings==COUNT_VAL)
            {
              TEMPERATURE=(byte)(TEMPERATURE_T/COUNT_VAL);
              HUMIDITY=(byte)(HUMIDITY_T/COUNT_VAL);
              numDHTReadings=0;
              TEMPERATURE_T=0;
              HUMIDITY_T=0;
            }
            else
            {
              numDHTReadings=0;
              TEMPERATURE_T=0;
              HUMIDITY_T=0;
            }
          }
          else
          {
              
              TEMPERATURE=DHT.temperature;
              HUMIDITY=DHT.humidity;
              TEMPERATURE_T+=TEMPERATURE;
              HUMIDITY_T+=HUMIDITY;
              numDHTReadings++;
          }
          break;
      case DHTLIB_ERROR_CHECKSUM:
          stat.crc_error++;          
          //Serial.print("Checksum error,\t");
          break;
      case DHTLIB_ERROR_TIMEOUT:
          stat.time_out++;
          //Serial.print("Time out error,\t");
          break;
      case DHTLIB_ERROR_CONNECT:
          stat.connect++;
          //Serial.print("Connect error,\t");
          break;
      case DHTLIB_ERROR_ACK_L:
          stat.ack_l++;
          //Serial.print("Ack Low error,\t");
          break;
      case DHTLIB_ERROR_ACK_H:
          stat.ack_h++;
          //Serial.print("Ack High error,\t");
          break;
      default:
          stat.unknown++;
          //Serial.print("Unknown error,\t");
          break;
      }
      if(SENSOR_INITIALIZED)
      {
        if(numLDRReadings<COUNT_VAL)
        {
          for(int j=0;j<4;j++)rawLightIntensity_T[j]+=analogRead(LDR_PIN[j]);
          numLDRReadings++;
        }
        else if(numLDRReadings==COUNT_VAL)
        {
          for(int j=0;j<4;j++)rawLightIntensity[j]=(int)(rawLightIntensity[j]/COUNT_VAL);
          numLDRReadings=0;
          for(int j=0;j<4;j++)rawLightIntensity_T[j]=0;
          LIGHT_INTENSITY=(int)(rawLightIntensity[0]+rawLightIntensity[1]+rawLightIntensity[2]+rawLightIntensity[3])/4;
        }
        else
        {
          numLDRReadings=0;
          for(int j=0;j<4;j++)rawLightIntensity_T[j]=0;
        }
      }
      else
      {
        for(int j=0;j<4;j++)
        {
          rawLightIntensity[j]=analogRead(LDR_PIN[j]);
          rawLightIntensity_T[j]+=rawLightIntensity[j];
        }
        numLDRReadings++;
        LIGHT_INTENSITY=(int)(rawLightIntensity[0]+rawLightIntensity[1]+rawLightIntensity[2]+rawLightIntensity[3])/4;
      }
      measureTime=millis();
     }
      interrupts();
      if(!SENSOR_INITIALIZED)SENSOR_INITIALIZED=true;
}

void handleWebRequest()
{
  // listen for incoming clients
  EthernetClient client = server.available();
  for (int i = 0; i < 110; i++)page[i] = ' ';
  if (client) {
    int count = 0;
    int storeRequest = 0;

    // an http request ends with a blank line
    boolean currentLineIsBlank = false;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (storeRequest == 1)
          page[count++] = c;
        if (c == '/' && storeRequest != 2)
          storeRequest = 1;
        if (c == ' ' && storeRequest)
          storeRequest = 2;

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          //Status
          if (page[0] == ' ')
          {
            MODE = AUTO_MODE;
            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close"); // the connection will be closed after completion of the response
            client.println();
            //Send page to client
            sendStatusPage(client);
          }

          //Manual
          else if (page[0] == 'm')
          {
            //New state set //The seemingly random indexes are the places where values will be present when data is sent through the correct format.
            if (page[11] == '?')
            {
              for (int i = 0; i < 5; i++)value[i] = ' ';

              //Set light on or off
              value[0] = page[24];
              if (value[0] == '1')
                L->on();
              else
                L->off();

              for (int i = 0; i < 5; i++)value[i] = ' ';

              //Set brightness
              value[0] = page[42];
              if (L->getState() == ON)
              {
                if (value[0] == '1')
                  L->dim(1);
                else if (value[0] == '2')
                  L->dim(2);
                else
                  L->dim(3);
              }

              for (int i = 0; i < 5; i++)value[i] = ' ';

              //Set fan state
              value[0] = page[54];
              if (value[0] == '1')
                F1->on();
              else
                F1->off();

              for (int i = 0; i < 5; i++)value[i] = ' ';

              //Set fan speed
              value[0] = page[65];
              if(F1->getState() == ON)
              {
                if (value[0] == '1')
                  F1->regulate(1);
                else if (value[0] == '2')
                  F1->regulate(2);
                else
                  F1->regulate(3);
              }

              for (int i = 0; i < 5; i++)value[i] = ' ';

              //Set AC
              value[0] = page[76];
              if (value[0] == '0')
                AC1->off();
              else
              {
                value[1] = page[92];   //Temperature
                value[2] = page[93];   //Temperature
                value[3] = page[103];
                byte reqTemp = ((byte) value[1] - 48) * 10 + ((byte) value[2] - 48);
                byte reqSpeed = (byte) value[3] - 48;
                AC1->set(reqTemp, reqSpeed);
              }
            }
            MODE = MANUAL_MODE;
            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close"); // the connection will be closed after completion of the response
            client.println();
            //Send page to client
            sendManualPage(client);
          }

          //Auto mode settings
          else if (page[0] == 'a')
          {
            index = 21;
            //New state set //The seemingly random indexes are the places where values will be present when data is sent through the correct format.
            if (page[index] == '?')
            {
              LIGHT_THRESHOLD = 0;
              index += 16;
              for (; page[index] != '&'; index++)
              {
                LIGHT_THRESHOLD *= 10;
                LIGHT_THRESHOLD += (int) page[index] - 48;
              }

              L1 = 0;
              index += 4;
              for (; page[index] != '&'; index++)
              {
                L1 *= 10;
                L1 += (int) page[index] - 48;
              }

              L2 = 0;
              index += 4;
              for (; page[index] != '&'; index++)
              {
                L2 *= 10;
                L2 += (int) page[index] - 48;
              }

              byte ambTemp = 0;
              index += 20;
              for (; page[index] != '&'; index++)
              {
                ambTemp *= 10;
                ambTemp += (int) page[index] - 48;
              }

              FAN_CUTOFF = ambTemp + 2;
              AC_CUTOFF = ambTemp - 2;
              AC_AMBIENT_TEMP = ambTemp - 4;

              index += 9;
              DEFAULT_AC_FAN_SPEED = (int) page[index] - 48;
              index++;

              T1 = 0;
              index += 4;
              for (; page[index] != '&'; index++)
              {
                T1 *= 10;
                T1 += (int) page[index] - 48;
              }

              H1 = 0;
              index += 4;
              for (; page[index] != '&'; index++)
              {
                H1 *= 10;
                H1 += (int) page[index] - 48;
              }

              H2 = 0;
              index += 4;
              for (; page[index] != '&'; index++)
              {
                H2 *= 10;
                H2 += (int) page[index] - 48;
              }

              H3 = 0;
              index += 4;
              for (; page[index] != ' '; index++)
              {
                H3 *= 10;
                H3 += (int) page[index] - 48;
              }
            }
            MODE = AUTO_MODE;
            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close"); // the connection will be closed after completion of the response
            client.println();
            //Send page to client
            sendAutoPage(client);
          }
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}

void lightRecommend(EthernetClient cl)
{
  if (NUM_PERSONS > 0)
  {
    if (LIGHT_INTENSITY < LIGHT_THRESHOLD)   //dim(1) means highly dim, dim(3) is very bright
    {
      cl.println("On at brightness ");
      if (LIGHT_INTENSITY < L2)
        cl.println("3</p>");
      else if (LIGHT_INTENSITY < L1)
        cl.println("2</p>");
      else
        cl.println("1</p>");
    }
    else
      cl.println("Off</p>");
  }
  else
    cl.println("Off</p>");
}

void fanRecommend(EthernetClient cl)
{
  if (NUM_PERSONS > 0)
  {
    if (AC1->getState() == OFF)
    {
      if (TEMPERATURE > T1 || HUMIDITY > H1)
      {
        cl.println("On at speed ");
        if (HUMIDITY > H3)
          cl.println("3</p>");
        else if (HUMIDITY > H2)
          cl.println("2</p>");
        else
          cl.println("1</p>");
      }
      else
        cl.println("Off</p>");
    }

    else
      cl.println("Off</p>");
  }
  else
    cl.println("Off</p>");
}

void ACRecommend(EthernetClient cl)
{
  if (NUM_PERSONS > 0)
  {
    if (TEMPERATURE > FAN_CUTOFF)
    {
      cl.println("On at temperature ");
      cl.println(AC_AMBIENT_TEMP);
      cl.println(" and fan speed ");
      cl.println(DEFAULT_AC_FAN_SPEED);
      cl.println("</p>");
    }

    else
      cl.println("Off</p>");
  }
  else
    cl.println("Off</p>");
}

void sendManualPage(EthernetClient cl)
{
  cl.println("<!DOCTYPE html");
  cl.println("<html>");
  cl.println("<head>");
  cl.println("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />");
  cl.println("<title>Manual Mode</title>");
  cl.println("</head>");
  cl.println("<body>");
  cl.println("<h1>Currently in manual mode. Go to the auto mode page to set it to auto mode. Go to auto mode settings page to change auto mode settings.</h1>");
  cl.print("<h2>");
  cl.print(NUM_PERSONS);
  cl.println(" persons in the room</h2>");
  cl.println("<form method=\"get\">");

  cl.println("<h2>Light</h2>");
  if (L->getState() == ON) //Light on/off
  {
    cl.println("<input type=\"radio\" id=\"lightOnButton\" value=\"1\" name=\"lightButton\" checked=\"checked\"/>On");
    cl.println("<input type=\"radio\" id=\"lightOffButton\" value=\"0\" name=\"lightButton\"/>Off");
  }
  else
  {
    cl.println("<input type=\"radio\" id=\"lightOnButton\" value=\"1\" name=\"lightButton\" />On");
    cl.println("<input type=\"radio\" id=\"lightOffButton\" value=\"0\" name=\"lightButton\" checked=\"checked\"/>Off");
  }
  cl.println("<br/>");

  cl.println("Brightness:<input type=\"number\" id=\"lightBrightness\" name=\"lightBrightness\" min=\"1\" max=\"3\" "); //Brightness
  if (L->getState() == OFF)
    cl.println("value=\"1\" />");
  else if (L->getBrightLevel() == 1)
    cl.println("value=\"1\" />");
  else if (L->getBrightLevel() == 2)
    cl.println("value=\"2\" />");
  else
    cl.println("value=\"3\" />");

  cl.println("<p id=\"lightStatus\">Current status is"); //Brightness
  if (L->getState() == ON)
  {
    cl.println("on at brightness ");
    if (L->getBrightLevel() == 1)
      cl.println("1</p>");
    else if (L->getBrightLevel() == 2)
      cl.println("2</p>");
    else
      cl.println("3</p>");
  }
  else
    cl.println("off</p>");

  cl.println("<p id=\"ldrReading\">LDR reading is");
  cl.println(LIGHT_INTENSITY);
  cl.println("</p>");
  cl.println("<p id=\"lightRecommended\">Recommended:"); //Light Recommended
  lightRecommend(cl);

  cl.println("<input type=\"submit\" />");

  cl.println("<br /><br />");
  cl.println("<h1>Fan</h1>");
  if (F1->getState() == ON)  //Fan on/off
  {
    cl.println("<input type=\"radio\" id=\"fanOnButton\" name=\"fanButton\" value=\"1\" checked=\"checked\"/>On");
    cl.println("<input type=\"radio\" id=\"fanOffButton\" name=\"fanButton\" value=\"0\" />Off");
  }
  else
  {
    cl.println("<input type=\"radio\" id=\"fanOnButton\" name=\"fanButton\" value=\"1\"/>On");
    cl.println("<input type=\"radio\" id=\"fanOffButton\" name=\"fanButton\" value=\"0\" checked=\"checked\"/>Off");
  }
  cl.println("<br />");

  cl.println("Speed:<input type=\"number\" id=\"fanSpeed\" name=\"fanSpeed\" min=\"1\" max=\"3\" "); //Fan speed
  if (F1->getState() == OFF)
    cl.println("value=\"1\" />");
  else if (F1->getSpeed() == 1)
    cl.println("value=\"1\" />");
  else if (F1->getSpeed() == 2)
    cl.println("value=\"2\" />");
  else
    cl.println("value=\"3\" />");

  cl.println("<p id=\"fanStatus\">Current status is ");
  if (F1->getState() == ON)
  {
    cl.println("on at speed ");
    if (F1->getSpeed() == 1)
      cl.println("1</p>");
    else if (F1->getSpeed() == 2)
      cl.println("2</p>");
    else
      cl.println("3</p>");
  }
  else
    cl.println("off</p>");

  cl.println("<p id=\"temperatureReading\">Temperature is ");
  cl.println(TEMPERATURE);
  cl.println("°C</p>");

  cl.println("<p id=\"humidityReading\">Humidity is ");
  cl.println(HUMIDITY);
  cl.println("%</p>");

  cl.println("<p id=\"fanRecommended\">Recommended:");
  fanRecommend(cl);

  cl.println("<input type=\"submit\" />");

  //AC
  cl.println("<br /><br />");
  cl.println("<h1>AC</h1>");

  if (AC1->getState() == ON)
  {
    cl.println("<input type=\"radio\" id=\"ACOnButton\" name=\"ACButton\" value=\"1\" checked=\"checked\"/>On");
    cl.println("<input type=\"radio\" id=\"ACOffButton\" name=\"ACButton\" value=\"0\" />Off");
  }
  else
  {
    cl.println("<input type=\"radio\" id=\"ACOnButton\" name=\"ACButton\" value=\"1\" />On");
    cl.println("<input type=\"radio\" id=\"ACOffButton\" name=\"ACButton\" value=\"0\" checked=\"checked\"/>Off");
  }
  cl.println("<br />");

  cl.print("Temperature:<input type=\"number\" id=\"ACTemperature\" name=\"ACTemperature\" min=\"17\" max=\"30\" value=\"");
  cl.print(AC1->getTemp());
  cl.println("\" />");
  cl.println("<br />");

  cl.print("Fan speed:<input type=\"number\" id=\"ACSpeed\" name=\"ACSpeed\" min=\"1\" max=\"3\" value=\"");
  cl.print(AC1->getFanSpeed());
  cl.println("\" />");

  cl.println("<p id=\"ACStatus\">Current status is ");
  if (AC1->getState() == ON)
  {
    cl.println("on at temperature ");
    cl.println((int) AC1->getTemp());
    cl.println(" and fan speed ");
    cl.println(AC1->getFanSpeed());
    cl.println("</p>");
  }
  else
    cl.println("off</p>");

  cl.println("<p id=\"temperatureReading\">Temperature is ");
  cl.println(TEMPERATURE);
  cl.println("°C</p>");

  cl.println("<p id=\"humidityReading\">Humidity is ");
  cl.println(HUMIDITY);
  cl.println("%</p>");

  cl.println("<p id=\"ACRecommended\">Recommended:");
  ACRecommend(cl);

  cl.println("<input type=\"submit\" />");

  cl.println("<br /><br />");

  cl.println("<input type=\"button\" id=\"refreshButton\" value=\"Refresh\" onclick=\"window.location.reload()\" />");
  cl.println("<input type=\"button\" id=\"autoSettingsButton\" value=\"Auto mode settings\" onclick=\"window.location='/autoModeSettings.html'\" />");
  cl.println("<input type=\"button\" id=\"autoButton\" value=\"Auto mode\" onclick=\"window.location='/'\"/>");
  cl.println("</form>");
  cl.println("</body>");
  cl.println("</html>");
}

void sendStatusPage(EthernetClient cl)
{
  cl.println("<!DOCTYPE html>");
  cl.println("<html>");
  cl.println("<head>");
  cl.println("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />");
  cl.println("<title>Status</title>");
  cl.println("</head>");
  cl.println("<body>");
  cl.println("<h1>Currently in auto mode. Go to manual mode to set to manual mode. Go to auto mode settings page to change auto mode settings.</h1>");

  cl.print("<h2>");
  cl.print(NUM_PERSONS);
  cl.println(" persons in the room</h2>");

  //Light
  cl.println("<h2>Light</h2>");
  cl.println("<br />");

  cl.println("<p id=\"lightStatus\">Current status is"); //Brightness
  if (L->getState() == ON)
  {
    cl.println("on at brightness ");
    if (L->getBrightLevel() == 1)
      cl.println("1</p>");
    else if (L->getBrightLevel() == 2)
      cl.println("2</p>");
    else
      cl.println("3</p>");
  }
  else
    cl.println("off</p>");

  cl.println("<p id=\"ldrReading\">LDR reading is");
  cl.println(LIGHT_INTENSITY);
  cl.println("</p>");

  cl.println("<p id=\"lightRecommended\">Recommended:"); //Light Recommended
  lightRecommend(cl);

  //Fan
  cl.println("<br /><br />");
  cl.println("<h1>Fan</h1>");
  cl.println("<br/>");

  cl.println("<p id=\"fanStatus\">Current status is ");
  if (F1->getState() == ON)
  {
    cl.println("on at speed ");
    if (F1->getSpeed() == 1)
      cl.println("1</p>");
    else if (F1->getSpeed() == 2)
      cl.println("2</p>");
    else
      cl.println("3</p>");
  }
  else
    cl.println("off</p>");

  cl.println("<p id=\"temperatureReading\">Temperature is ");
  cl.println(TEMPERATURE);
  cl.println("°C</p>");

  cl.println("<p id=\"humidityReading\">Humidity is ");
  cl.println(HUMIDITY);
  cl.println("%</p>");

  cl.println("<p id=\"fanRecommended\">Recommended:");
  fanRecommend(cl);

  //AC
  cl.println("<br /><br />");
  cl.println("<h1>AC</h1>");
  cl.println("<br/>");

  cl.println("<p id=\"ACStatus\">Current status is ");
  if (AC1->getState() == ON)
  {
    cl.println("on at temperature ");
    cl.println((int) AC1->getTemp());
    cl.println(" and fan speed ");
    cl.println(AC1->getFanSpeed());
    cl.println("</p>");
  }
  else
    cl.println("off</p>");

  cl.println("<p id=\"temperatureReading\">Temperature is ");
  cl.println(TEMPERATURE);
  cl.println("°C</p>");

  cl.println("<p id=\"humidityReading\">Humidity is ");
  cl.println(HUMIDITY);
  cl.println("%</p>");

  cl.println("<p id=\"ACRecommended\">Recommended:");
  ACRecommend(cl);

  cl.println("<input type=\"button\" id=\"refreshButton\" value=\"Refresh\" onclick=\"window.location.reload()\" />");
  cl.println("<input type=\"button\" id=\"manualButton\" value=\"Manual mode\" onclick=\"window.location='/manual.html'\" />");
  cl.println("<input type=\"button\" id=\"autoButton\" value=\"Auto mode settings\" onclick=\"window.location='/autoModeSettings.html'\" />");
  cl.println("</body>");
  cl.println("</html>");
}

void sendAutoPage(EthernetClient cl)
{
  cl.println("<!DOCTYPE html>");
  cl.println("<html>");
  cl.println("<head>");
  cl.println("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />");
  cl.println("<title>Auto Mode Settings</title>");
  cl.println("</head>");
  cl.println("<body>");

  cl.println("<h1>Currently in auto mode </h1>");
  cl.println("<form method=\"get\">");
  cl.println("<h2>Light</h2>");

  cl.println("<p id=\"ldrReading\">LDR reading is");
  cl.println(LIGHT_INTENSITY);
  cl.println("</p>");

  cl.println("<p id=\"lightStatus\">Current status is"); //Brightness
  if (L->getState() == ON)
  {
    cl.println("on at brightness ");
    if (L->getBrightLevel() == 1)
      cl.println("1</p>");
    else if (L->getBrightLevel() == 2)
      cl.println("2</p>");
    else
      cl.println("3</p>");
  }
  else
    cl.println("off</p>");

  cl.println("<br/>");
  cl.print("On below:");
  cl.print("<input type=\"number\" name=\"lightThreshold\" id=\"lightThreshold\" min=\"");
  cl.print(L1);
  cl.print("\" max=1023 value=\"");
  cl.print(LIGHT_THRESHOLD);
  cl.println("\" />");

  cl.println("<br/>");
  cl.print("Brightness level 2 below:");
  cl.print("<input type=\"number\" name=\"L1\" id=\"L1\" min=\"");
  cl.print(L2);
  cl.print("\" max=\"");
  cl.print(LIGHT_THRESHOLD);
  cl.print("\" value=\"");
  cl.print(L1);
  cl.println("\" />");


  cl.println("<br/>");
  cl.print("Brightness level 3 below:");
  cl.print("<input type=\"number\" name=\"L2\" id=\"L2\" min=0 max=\"");
  cl.print(L1);
  cl.print("\" value=\"");
  cl.print(L2);
  cl.println("\" />");

  cl.println("<br/>");
  cl.println("<input type=\"submit\" />");

  cl.println("<h2>Fan</h2>");

  cl.println("<p id=\"temperatureReading\">Temperature is ");
  cl.println(TEMPERATURE);
  cl.println("°C</p>");

  cl.println("<p id=\"humidityReading\">Humidity is ");
  cl.println(HUMIDITY);
  cl.println("%</p>");

  cl.println("<p id=\"fanStatus\">Current status of fan is ");
  if (F1->getState() == ON)
  {
    cl.println("on at speed ");
    if (F1->getSpeed() == 1)
      cl.println("1</p>");
    else if (F1->getSpeed() == 2)
      cl.println("2</p>");
    else
      cl.println("3</p>");
  }
  else
    cl.println("off</p>");

  cl.println("<p id=\"ACStatus\">Current status of AC is ");
  if (AC1->getState() == ON)
  {
    cl.println("on at temperature ");
    cl.println((int) AC1->getTemp());
    cl.println(" and fan speed ");
    cl.println(AC1->getFanSpeed());
    cl.println("</p>");
  }
  else
    cl.println("off</p>");

  cl.println("<br/>");
  cl.print("Ambient temperature:");
  cl.print("<input type=\"number\" name=\"ambientTemperature\" id=\"ambientTemperature\" min=21 max=34 value=\"");
  cl.print(FAN_CUTOFF - 2);
  cl.println("\" />");

  cl.println("<br/>");
  cl.print("Default AC fan speed:");
  cl.print("<input type=\"number\" name=\"ACSpeed\" id=\"ACSpeed\" min=1 max=3 value=\"");
  cl.print(DEFAULT_AC_FAN_SPEED);
  cl.println("\" />");

  cl.println("<br/>");
  cl.print("Fan on above temperature:");
  cl.print("<input type=\"number\" name=\"T1\" id=\"T1\" min=0 max=\"");
  cl.print(AC_CUTOFF);
  cl.print("\" value=\"");
  cl.print(T1);
  cl.println("\" />");

  cl.println("<br/>");
  cl.print("Fan on above humidity:");
  cl.print("<input type=\"number\" name=\"H1\" id=\"H1\" min=0 max=\"");
  cl.print(H2);
  cl.print("\" value=\"");
  cl.print(H1);
  cl.println("\" />");

  cl.println("<br/>");
  cl.print("Fan speed 2 above humidity:");
  cl.print("<input type=\"number\" name=\"H2\" id=\"H2\" min=\"");
  cl.print(H1);
  cl.print("\" max=\"");
  cl.print(H3);
  cl.print("\" value=\"");
  cl.print(H2);
  cl.println("\" />");

  cl.println("<br/>");
  cl.print("Fan speed 3 above humidity:");
  cl.print("<input type=\"number\" name=\"H3\" id=\"H3\" min=\"");
  cl.print(H2);
  cl.print("\" max=100 value=\"");
  cl.print(H3);
  cl.println("\" />");

  cl.println("<br/>");
  cl.println("<input type=\"submit\" />");
  cl.println("</form>");

  cl.println("<br/>");
  cl.println("<input type=\"button\" id=\"refreshButton\" value=\"Refresh\" onclick=\"window.location.reload()\" />");
  cl.println("<input type=\"button\" id=\"manualButton\" value=\"Manual mode\" onclick=\"window.location='/manual.html'\" />");
  cl.println("<input type=\"button\" id=\"autoButton\" value=\"Auto mode\" onclick=\"window.location='/'\"/>");
  cl.println("</body>");
  cl.println("</html>");
}


void detectLaser1()
{
  if (digitalRead(laserPin1) == 1 - prevState[0])
  {
    laser[0][count] = 1 - prevState[0];
    laser[1][count] = prevState[1];
    prevState[0] = laser[0][count];
    prevState[1] = laser[1][count];
    count++;
    if (count == 5)
    {
      if (laser[0][0] == 1 && laser[0][1] == 0 && laser[0][2] == 0 && laser[0][3] == 1 && laser[0][4] == 1 &&
          laser[1][0] == 1 && laser[1][1] == 1 && laser[1][2] == 0 && laser[1][3] == 0 && laser[1][4] == 1)
      {
        NUM_PERSONS++;
        detected = true;
      }

      else if (laser[1][0] == 1 && laser[1][1] == 0 && laser[1][2] == 0 && laser[1][3] == 1 && laser[1][4] == 1 &&
               laser[0][0] == 1 && laser[0][1] == 1 && laser[0][2] == 0 && laser[0][3] == 0 && laser[0][4] == 1)
      {
        NUM_PERSONS--;
        detected = true;
      }
      if (detected)
      {
        count = 1;
        prevState[0] = 1;
        prevState[1] = 1;
        detected = false;
      }
    }
    if (count > 4)
    {
      boolean flag1 = true;
      for (int i = 0; i < 4 && flag1; i++)
      {
        for (int j = i + 1; j < 6 && flag1; j++)
        {
          if (laser[0][i] == laser[0][j] && laser[1][i] == laser[1][j])
          {
            count = i + 1;
            flag1 = false;
          }
        }

      }
      if (flag1)
      {
        count = 4;
        for (int i = 0; i < 4; i++)
        {
          laser[0][i] = laser[0][i + 1];
          laser[1][i] = laser[1][i + 1];
        }
      }
    }
  }
}
void detectLaser2()
{
  if (digitalRead(laserPin2) == 1 - prevState[1])
  {
    laser[1][count] = 1 - prevState[1];
    laser[0][count] = prevState[0];
    prevState[0] = laser[0][count];
    prevState[1] = laser[1][count];
    count++;
    if (count == 5)
    {
      if (laser[0][0] == 1 && laser[0][1] == 0 && laser[0][2] == 0 && laser[0][3] == 1 && laser[0][4] == 1 &&
          laser[1][0] == 1 && laser[1][1] == 1 && laser[1][2] == 0 && laser[1][3] == 0 && laser[1][4] == 1)
      {
        NUM_PERSONS++;
        detected = true;
      }

      else if (laser[1][0] == 1 && laser[1][1] == 0 && laser[1][2] == 0 && laser[1][3] == 1 && laser[1][4] == 1 &&
               laser[0][0] == 1 && laser[0][1] == 1 && laser[0][2] == 0 && laser[0][3] == 0 && laser[0][4] == 1)
      {
        NUM_PERSONS--;
        detected = true;
      }
      if (detected)
      {
        count = 1;
        prevState[0] = 1;
        prevState[1] = 1;
        detected = false;
      }
    }
    if (count > 4)
    {
      boolean flag1 = true;
      for (int i = 0; i < 4 && flag1; i++)
      {
        for (int j = i + 1; j < 6 && flag1; j++)
        {
          if (laser[0][i] == laser[0][j] && laser[1][i] == laser[1][j])
          {
            count = i + 1;
            flag1 = false;
          }
        }

      }
      if (flag1)
      {
        count = 4;
        for (int i = 0; i < 4; i++)
        {
          laser[0][i] = laser[0][i + 1];
          laser[1][i] = laser[1][i + 1];
        }
      }
    }
  }
}
void switchFan()
{
  if (MODE == AUTO_MODE)MODE = MANUAL_MODE;
  if (F1->getState() == 1)F1->off();
  else F1->on();
}
void switchLight()
{
  if (MODE == AUTO_MODE)MODE = MANUAL_MODE;
  if (L->getState() == 1)L->off();
  else L->on();
}
