//Header files to be included
#include "Light.h"
#include "AirConditioner.h"
#include "Fan.h"
#include "HS.h"
#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac [] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
boolean clientConnected = false;

//HTTP requests
char page[150];
char value[5];
int index = 0;

//declare device state variables ON and OFF
const byte ON = 1;
const byte OFF = 0;

//Modes
const byte AUTO_MODE = 31;
const byte MANUAL_MODE = 27;
volatile byte MODE = AUTO_MODE;

//Sensors
const byte COUNT_VAL = 25;	//No. of values to sample before taking average
volatile byte NUM_PERSONS = 0;	//number of persons present in room initialized to zero
int LIGHT_INTENSITY = 0;
int rawLightIntensity[4] = { 0, 0, 0, 0 };	//stores light intensity from four LDRs
int rawLightIntensity_T[4] = { 0, 0, 0, 0 };	//stores the cumulative light intensity from the sensors for purpose of taking average
int TEMPERATURE_T = 0;		//stores cumulative temperature for purpose of taking average
byte numTempReadings = 0;	//stores number of temperature samplings done
byte numLDRReadings = 0;	//stores number of LDR samplings done
byte TEMPERATURE_ACTUAL=0;	//stores temperature as read by sensor
byte TEMPERATURE =0;		//stores temperature according to the limits needed for thresholds to work, used to correct for slight discrepancies in sensor data
byte HUMIDITY = 0;		//stores humidity as measured by sensor
int HUMIDITY_T = 0;		//stores cumulative humidity

boolean SENSOR_INITIALIZED = false;	//is used to check whether the average is already taken or if not, to use current values as average until enough data is collected, valid for first run
volatile boolean changeLaser=false;	//is true if a person is detected entering or exiting
//Auto mode preferences
byte FAN_CUTOFF = 25; //temperature above which fan is switched off and AC is switched on
byte AC_CUTOFF = 23; //temperature below which AC is switched off and fan is switched on
byte AC_AMBIENT_TEMP = 22;
byte DEFAULT_AC_FAN_SPEED = 1;

//levels for fan and light; not constant since user can set it through auto mode settings
byte T3 = 16, T2 = 14, T1 = 12; //Denotes lower level of range,i.e. 5th level for T>T5,needs to be set high for final demo since heating the sensor is easier than cooling
byte H1 = 50, H2 = 60, H3 = 80;	//Humidity levels for fan speed. Since we don't use humidity much, this seemed like the best place to put it
int LIGHT_THRESHOLD = 700;//above which there is no need for light at all
int L1 = 500, L2 = 400;	//Since only 3 brightness levels

//declare pins to be used in circuit
byte* fanRegulatePins = new byte[3];
byte* lightRegulatePins = new byte[3];
const byte laserPin1 = 2, laserPin2 = 3;

//intialze variables related to laser person count
volatile byte laser[2][5];	//stores states of laser for matching to particular patterns of entry or exit 
volatile boolean detected = false;	
boolean flag = true;
volatile byte count = 0;	//stores index of state to updated in the array
volatile byte prevState[2];
volatile boolean errorFlag = false;

//pushButton pins
const byte pushButtonFan = 6;
const byte pushButtonLight = 7;
byte prevPushButtonFan = 0;
byte prevPushButtonLight = 0;

//pins for HS220
const byte humidityPin = A1;
const byte temperaturePin = A0;
HS hs(humidityPin, temperaturePin, 55);

//pins for LDR
byte LDR_PIN[4] = { A2, A3, A4, A5 };        //Use different pin if required

//LCD variables
//Store lines for displaying on LCD
String lcdLine1 = "    WELCOME   ";
String lcdLine2 = "";
const int lcdCount_T = 1000;
int lcdCount = 0;
boolean changeLCD=false;

//create instances of devices
Light* L;
Fan* F1;
AirConditioner* AC1;
LiquidCrystal lcd(48, 49, 38, 36, 34, 32);

void setup()
{
  //Initialize pins
  fanRegulatePins[0] = 23; 
  fanRegulatePins[1] = 25;
  fanRegulatePins[2] = 27;
  lightRegulatePins[0] = 47;
  lightRegulatePins[1] = 45;
  lightRegulatePins[2] = 43;
  //Initialize LCD
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print(lcdLine1);

  // data for TL AC
  unsigned long*** data = new unsigned long**[14];
  for (int j = 0; j < 14; j++)
  {
    data[j] = new unsigned long*[3];
    for (int k = 0; k < 3; k++)data[j][k] = new unsigned long[2];
  }
  
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

  F1 = new Fan(fanRegulatePins, 3);
  L = new Light(lightRegulatePins, 3);
  AC1 = new AirConditioner(600, 470, 1550, 4400, 4300, 5000, 38, data, offData, 14, 3, 9, 30, 17);
  // start the Ethernet connection and the server:
  Ethernet.begin(mac);
  server.begin();
  delay(2000);
  lcdLine1 = "TEMP:   RH:     ";
  lcd.setCursor(0, 0);
  lcd.print(lcdLine1);
}
void loop()
{
  //for the intialization of laser, checks both of them receive uninterrupted light and then attaches interrupts
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
      lcd.setCursor(0,0);
      lcd.print("  LASER SET  ");
      delay(2000);
    }
  }

  //read data from sensors
  readSensorData();

  //display current states on LCD and modify only after certain number of loop runs
  if (lcdCount == lcdCount_T && !changeLCD)
  {
    lcd.setCursor(0, 0);
    lcdLine1 = lcdLine1.substring(0, 5) + TEMPERATURE + lcdLine1.substring(7, 11) + HUMIDITY + "%   ";
    lcd.print(lcdLine1);
    lcdLine2 = "F:";
    if (F1->getState() == ON)
    {
      lcdLine2 = lcdLine2 + "ON ,";
      lcdLine2 = lcdLine2 + F1->getSpeed();
    }
    else lcdLine2 = lcdLine2 + "OFF,0";
    lcdLine2 = lcdLine2 + " L:";
    if (L->getState() == ON)
    {
      lcdLine2 = lcdLine2 + "ON ,";
      lcdLine2 = lcdLine2 + L->getBrightLevel();
    }
    else lcdLine2 = lcdLine2 + "OFF,0";
    for(int i=0;i<16-lcdLine2.length();i++)lcdLine2=lcdLine2+" ";
    lcd.setCursor(0, 1);
    lcd.print(lcdLine2);
    lcdCount=0;
  }
  else
  {
    lcdCount++;
    if(changeLCD)
    {
      changeLCD=false;
      lcdCount=0;
    }
  }

  //display on LCD the number of persons in room if an entry or exit is detected
  if(changeLaser)
  {
    lcd.setCursor(0,0);
    String tmp="NUM PERSONS:"+NUM_PERSONS+"";
    for(int i=0;i<16-tmp.length();i++)tmp=tmp+" ";
    lcd.print(tmp);
    changeLCD=true;
    changeLaser=false;
    lcdCount=0;
  }
  
  //detect pushbutton presses
  byte tmp1 = digitalRead(pushButtonFan);
  byte tmp2 = digitalRead(pushButtonLight);
  if (tmp1 != prevPushButtonFan)
  {
    if (tmp1 == 1)switchFan();
    prevPushButtonFan = tmp1;
  }
  if (tmp2 != prevPushButtonLight)
  {
    if (tmp2 == 1)switchLight();
    prevPushButtonLight = tmp2;
  }

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
        {
          noInterrupts();
          AC1->set(AC_AMBIENT_TEMP, DEFAULT_AC_FAN_SPEED);
          interrupts();
        }

      }
      else
      {
        if (TEMPERATURE < AC_CUTOFF)
        {
          if (AC1->getState() == ON)
          {
            noInterrupts();
            AC1->off();
            interrupts();
          }
        }

        if ((TEMPERATURE > T1 || HUMIDITY > H1) && AC1->getState() == OFF)
        {
          if (HUMIDITY > H3 || TEMPERATURE > T3)
            F1->regulate(3);
          else if ((HUMIDITY > H2 && TEMPERATURE > T3) || (HUMIDITY > H1 && TEMPERATURE > T3))
            F1->regulate(2);
          else
            F1->regulate(1);
        }
        else
        {
          if (F1->getState() != OFF)
            F1->off();
        }
      }

      if (LIGHT_INTENSITY < LIGHT_THRESHOLD) //dim(1) means highly dim, dim(3) is very bright
      {
        if(L->getBrightLevel()==0)
        {
          if(LIGHT_INTENSITY<300)
          L->dim(3);
          else if(LIGHT_INTENSITY<425)
          L->dim(2);
          else if(LIGHT_INTENSITY<500)
          L->dim(1);
          else L->off();
        }
        else if(L->getBrightLevel()==1)
        {
          if(LIGHT_INTENSITY<350)
          L->dim(3);
          else if(LIGHT_INTENSITY<425)
          L->dim(2);
          else if(LIGHT_INTENSITY>600)L->off();
        }
        else if(L->getBrightLevel()==2)
        {
          if(LIGHT_INTENSITY<400)
          L->dim(3);
          else if(LIGHT_INTENSITY<500)
          L->dim(2);
          else if(LIGHT_INTENSITY<600)
          L->dim(1);
          else L->off();
        }
        else
        {
          if(LIGHT_INTENSITY<575)
          L->dim(3);
          else if(LIGHT_INTENSITY<675)
          L->dim(2);
          else
          L->dim(1);
        }
      }
      else
      {
        L->off();
      }
    }
    else
    {
      if(AC1->getState()!=OFF)
      {
            noInterrupts();
            AC1->off();
            interrupts();
      }
      F1->off();
      L->off();
    }

  }

  handleWebRequest();
}

void readSensorData()
{
  //measure temperature and humidity
  //check if average already taken or getting data for first time
  if (SENSOR_INITIALIZED)
  {
    //take readings for only fixed number of times
    if (numTempReadings < COUNT_VAL)
    {
      TEMPERATURE_T += hs.getTemperature();
      HUMIDITY_T += hs.getHumidity();
      numTempReadings++;
    }
    //if it reaches a fixed value, take average of the readings
    else if (numTempReadings == COUNT_VAL)
    {
      TEMPERATURE_ACTUAL = (byte) (TEMPERATURE_T / COUNT_VAL);
      HUMIDITY = (byte) (HUMIDITY_T / COUNT_VAL);
      numTempReadings = 0;
      TEMPERATURE_T = 0;
      HUMIDITY_T = 0;
    }
    else
    {
      numTempReadings = 0;
      TEMPERATURE_T = 0;
      HUMIDITY_T = 0;
    }
  }
  else
  {

    TEMPERATURE_ACTUAL = hs.getTemperature();
    HUMIDITY = hs.getHumidity();
    TEMPERATURE_T += TEMPERATURE;
    HUMIDITY_T += HUMIDITY;
    numTempReadings++;
  }

  if (SENSOR_INITIALIZED)
  {
    if (numLDRReadings < COUNT_VAL)
    {
      for (int j = 0; j < 4; j++)rawLightIntensity_T[j] += analogRead(LDR_PIN[j]);
      numLDRReadings++;
    }
    else if (numLDRReadings == COUNT_VAL)
    {
      for (int j = 0; j < 4; j++)rawLightIntensity[j] = (int) (rawLightIntensity_T[j] / COUNT_VAL);
      numLDRReadings = 0;
      for (int j = 0; j < 4; j++)rawLightIntensity_T[j] = 0;
      LIGHT_INTENSITY = (int) (rawLightIntensity[0] + rawLightIntensity[1] + rawLightIntensity[2] + rawLightIntensity[3]) / 4;
    }
    else
    {
      numLDRReadings = 0;
      for (int j = 0; j < 4; j++)rawLightIntensity_T[j] = 0;
    }
  }
  else
  {
    for (int j = 0; j < 4; j++)
    {
      rawLightIntensity[j] = analogRead(LDR_PIN[j]);
      rawLightIntensity_T[j] += rawLightIntensity[j];
    }
    numLDRReadings++;
    LIGHT_INTENSITY = (int) (rawLightIntensity[0] + rawLightIntensity[1] + rawLightIntensity[2] + rawLightIntensity[3]) / 4;
  }
  if (!SENSOR_INITIALIZED)SENSOR_INITIALIZED = true;
  TEMPERATURE=TEMPERATURE_ACTUAL-5;
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
              if (F1->getState() == ON)
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
              {
                if(AC1->getState()!=OFF)
                {
                  noInterrupts();
                  AC1->off();
                  interrupts();
                }
              }
              else
              {
                value[1] = page[92];   //Temperature
                value[2] = page[93];   //Temperature
                value[3] = page[103];
                byte reqTemp = ((byte) value[1] - 48) * 10 + ((byte) value[2] - 48);
                byte reqSpeed = (byte) value[3] - 48;
                if(AC1->getState()!=ON||AC1->getTemp()!=reqTemp||AC1->getFanSpeed()!=reqSpeed)
                {
                  noInterrupts();
                  AC1->set(reqTemp, reqSpeed);
                  interrupts();
                }
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

              T2 = 0;
              index += 4;
              for (; page[index] != '&'; index++)
              {
                T2 *= 10;
                T2 += (int) page[index] - 48;
              }

              H2 = 0;
              index += 4;
              for (; page[index] != '&'; index++)
              {
                H2 *= 10;
                H2 += (int) page[index] - 48;
              }

              T3 = 0;
              index += 4;
              for (; page[index] != '&'; index++)
              {
                T3 *= 10;
                T3 += (int) page[index] - 48;
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
        if (HUMIDITY > H3 || TEMPERATURE > T3)
          cl.println("3</p>");
        else if ((HUMIDITY > H2 && TEMPERATURE > T3) || (HUMIDITY > H1 && TEMPERATURE > T3))
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
  cl.print(T2);
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
  cl.print("Fan speed 2 above temperature:");
  cl.print("<input type=\"number\" name=\"T2\" id=\"T2\" min=\"");
  cl.print(T1);
  cl.print("\" max=\"");
  cl.print(T3);
  cl.print("\" value=\"");
  cl.print(T2);
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
  cl.print("Fan speed 3 above temperature:");
  cl.print("<input type=\"number\" name=\"T3\" id=\"T3\" min=\"");
  cl.print(T2);
  cl.print("\" max=\"");
  cl.print(AC_CUTOFF);
  cl.print("\" value=\"");
  cl.print(T3);
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
        if (NUM_PERSONS != 0)
          NUM_PERSONS--;
        else
          errorFlag = true;
        detected = true;
      }
      if (detected)
      {

        changeLaser=true;
        count = 1;
        prevState[0] = 1;
        prevState[1] = 1;
        detected = false;
      }
    }
    boolean flag1 = true;
    for (int i = 0; i < count - 1 && flag1; i++)
    {
      for (int j = i + 1; j < count && flag1; j++)
      {
        if (laser[0][i] == laser[0][j] && laser[1][i] == laser[1][j])
        {
          count = i + 1;
          flag1 = false;
        }
      }

    }
    if (count > 4)
    {
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
        if (NUM_PERSONS != 0)
          NUM_PERSONS--;
        else
          errorFlag = true;
        detected = true;
      }
      if (detected)
      {
        changeLaser=true;
        count = 1;
        prevState[0] = 1;
        prevState[1] = 1;
        detected = false;
      }
    }
    boolean flag1 = true;
    for (int i = 0; i < count - 1 && flag1; i++)
    {
      for (int j = i + 1; j < count && flag1; j++)
      {
        if (laser[0][i] == laser[0][j] && laser[1][i] == laser[1][j])
        {

          count = i + 1;
          flag1 = false;
        }
      }

    }
    if (count > 4)
    {
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
