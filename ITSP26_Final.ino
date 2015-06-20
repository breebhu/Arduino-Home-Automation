#include <Light.h>
#include <AirConditioner.h>
#include <Fan.h>

#include <SPI.h>
#include <Ethernet.h>
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10,9,0,138);
// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
boolean clientConnected=false;

//HTTP requests
char page[110];
char value[5];
int index=0;

const byte ON=1;
const byte OFF=0;

//Modes
const byte AUTO_MODE=31;   //31?
const byte MANUAL_MODE=27; //27?
byte MODE=AUTO_MODE;


byte AC_STATE=OFF;

//Sensors
byte NUM_PERSONS=0;
boolean NO_MOTION=false;
byte LIGHT_INTENSITY=0;
byte TEMPERATURE=0;
byte HUMIDITY=0;

//Auto mode preferences
byte FAN_CUTOFF=28;
byte AC_CUTOFF=24;
byte AC_AMBIENT_TEMP=22;
byte DEFAULT_AC_FAN_SPEED=1;
const byte T5=24,T4=20,T3=16,T2=14,T1=12;
const byte LIGHT_THRESHOLD=600;
const byte L1=500,L2=400,L3=300;

//Appliances
byte* fanRegulatePins=new byte[3];  //For readability
fanRegulatePins[0]=27;  //Initialize pins
fanRegulatePins[1]=28;
fanRegulatePins[2]=29;
extern Fan* F1=new Fan(26,fanRegulatePins,3);   
byte* lightRegulatePins=new byte[3];
lightRegulatePins[0]=23;
lightRegulatePins[1]=24;
lightRegulatePins[2]=25;
extern Light* L=new Light(22,lightRegulatePins,3);
unsigned long data[13][3][2];

unsigned long offData[2];
extern AirConditioner* AC1=new AirConditioner(600,470,1550,4400,4300,5000,38,,,2,3,9);
void setup()
{
   //start the system
   //initialize devices
   //while uploading sync time to PC
   
   // start the Ethernet connection and the server:
   Ethernet.begin(mac, ip);
   server.begin();
}
void loop()
{
  //read data from sensors
    readSensorData();
  //check if auto mode or manual mode from website and update accordingly
  if(MODE==AUTO_MODE)
  {
    //check state of devices based on present data
    if(TEMPERATURE>FAN_CUTOFF)
    {
      if(F1->getState()!=OFF)
      {
        F1->off();
      }
      if(AC_STATE!=ON)acON(); //Change once header is done
      setAC(AC_AMBIENT_TEMP,DEFAULT_AC_FAN_SPEED); 
    }
    else if(TEMPERATURE<AC_CUTOFF)
    {
      acOFF();
      if(TEMPERATURE>T5)   //Humidity instead of temperature seems like a better option
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

    if(LIGHT_INTENSITY<LIGHT_THRESHOLD)   //dim(1) means highly dim, dim(3) is very bright
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
    
      // listen for incoming clients
      EthernetClient client = server.available();
      for(int i=0;i<110;i++)page[i]=' '; //Char array/string
      if (client) {
      int count=0;
      int storeRequest=0;

      // an http request ends with a blank line
      boolean currentLineIsBlank = false;
      while (client.connected()) {
         if (client.available()) {
            char c = client.read();
            if(storeRequest==1)
               page[count++]=c; //Char array/string
            if(c=='/' && storeRequest!=2)
               storeRequest=1;
            if(c==' ' && storeRequest)
               storeRequest=2;
               
            // if you've gotten to the end of the line (received a newline
            // character) and the line is blank, the http request has ended,
            // so you can send a reply
            if (c == '\n' && currentLineIsBlank) {
               //Status
               if(page[0]==' ') //Char array/String
               {
                  MODE=AUTO_MODE;
                  // send a standard http response header
                  client.println("HTTP/1.1 200 OK");
                  client.println("Content-Type: text/html");
                  client.println("Connection: close"); // the connection will be closed after completion of the response
                  client.println();
                  //Send page to client
                  sendStatusPage(client);
               }
               
               //Manual
               else if(page[0]=='m') //Char array/String
               {
                  //New state set //The seemingly random indexes are the places where values will be present when data is sent through the correct format.
                  if(page[11]=='?') //Char array/String
                  {
                     for(int i=0;i<5;i++)value[i]=' '; //Char array/String
                     
                     //Set light on or off
                     value[0]=page[24]; //Char array/String
                     if(value[0]=='1') //Char array/String
                        L->on();
                     else
                        L->off();
                        
                     for(int i=0;i<5;i++)value[i]=' '; //Char array/String
                     
                     //Set brightness
                     value[0]=page[42];
                     if(value[0]=='0')
                        L->off();
                     else if(value[0]=='1')
                     {
                        L->dim(1);
                     }
                     else if(value[0]=='2')
                     {
                        L->dim(2);
                     }
                     else if(value[0]=='3')
                     {
                        L->dim(3);
                     }

                     //Set others
                  }
                  MODE=MANUAL_MODE;
                  // send a standard http response header
                  client.println("HTTP/1.1 200 OK");
                  client.println("Content-Type: text/html");
                  client.println("Connection: close"); // the connection will be closed after completion of the response
                  client.println();
                  //Send page to client
                  sendManualPage(client);
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

void readSensorData()
{
  
}

