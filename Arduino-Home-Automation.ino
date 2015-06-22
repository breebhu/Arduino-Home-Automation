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
const byte AUTO_MODE=31;   
const byte MANUAL_MODE=27; 
byte MODE=AUTO_MODE;

//Sensors
volatile byte NUM_PERSONS=0;
boolean NO_MOTION=false;
int LIGHT_INTENSITY=0;
byte TEMPERATURE=0;
byte HUMIDITY=0;

//Auto mode preferences
byte FAN_CUTOFF=28;
byte AC_CUTOFF=24;
byte AC_AMBIENT_TEMP=22;
byte DEFAULT_AC_FAN_SPEED=1;
const byte T5=24,T4=20,T3=16,T2=14,T1=12; //Needs to be set high for final demo since heating the sensor is easier than cooling 
const int LIGHT_THRESHOLD=600;
const int L1=500,L2=400,L3=300;
const byte RH_THRESHOLD=80;

//declare pins to be used in circuit
byte* fanRegulatePins=new byte[3];    
byte* lightRegulatePins=new byte[3];
const byte laserPin1=2, laserPin2=3;
//pushButton pins
const byte pushButtonFan=21;
const byte pushButtonLight=20;
const byte pushButtonAC=18;

//create instances of devices
Light* L;
Fan* F1;
AirConditioner* AC1;

//intialze variables related to laser person count
volatile byte laser[2][5];
volatile boolean detected=false;
boolean flag=true;
volatile byte count=0;
volatile byte prevState[2];

void setup()
{
   //start the system
   //initialize devices
   //while uploading sync time to PC
   fanRegulatePins[0]=27;  //Initialize pins
fanRegulatePins[1]=28;
fanRegulatePins[2]=29; 
lightRegulatePins[0]=23;
lightRegulatePins[1]=24;
lightRegulatePins[2]=25;
   unsigned long*** data=new unsigned long**[2];
for(int j=0;j<2;j++)
{
  data[j]=new unsigned long*[3];
  for(int k=0;k<3;k++)data[j][k]=new unsigned long[2];
} // data for 24 and 25 C for TL AC
data[0][1][0]=0xB24D5F;
data[0][1][1]=0xA040BF;
data[0][0][0]=0xB24D9F;
data[0][0][1]=0x6040BF;
data[0][2][0]=0xB24D3F;
data[0][2][1]=0xC040BF;
data[1][0][0]=0xB24D9F;
data[1][0][1]=0x60C03F;
data[1][1][0]=0xB24D5F;
data[1][1][1]=0xA0C03F;
data[1][2][0]=0xB24D3F;
data[1][2][1]=0xC0C03F;
unsigned long* offData=new unsigned long[2];
offData[0]=0xB24D7B;
offData[1]=0x84E01F;
F1=new Fan(26,fanRegulatePins,3);
L=new Light(22,lightRegulatePins,3);  
AC1=new AirConditioner(600,470,1550,4400,4300,5000,38,data,offData,2,3,9,24,25);
   // start the Ethernet connection and the server:
   Ethernet.begin(mac, ip);
   server.begin();
}
void loop()
{
  if(flag)
  {
    
    if(digitalRead(laserPin1)==1&&digitalRead(laserPin2)==1)
    {
      laser[0][0]=1;
      laser[1][0]=1;
      count++;
      prevState[0]=1; 
      prevState[1]=1;
      attachInterrupt(0,detectLaser1,CHANGE);
      attachInterrupt(1,detectLaser2,CHANGE);
      flag=false;
    }
  }
  //read data from sensors
    readSensorData();
  //check if auto mode or manual mode from website and update accordingly
  if(MODE==AUTO_MODE)
  {
    //check state of devices based on present data
    if(NUM_PERSONS>0)
    {
        if(TEMPERATURE>FAN_CUTOFF)
        {
          if(F1->getState()!=OFF)
          {
            F1->off();
          }
          AC1->set(AC_AMBIENT_TEMP,DEFAULT_AC_FAN_SPEED); 
    
        }
        else if(TEMPERATURE<AC_CUTOFF)
        {
            AC1->off();
    	if(TEMPERATURE>T1)
    	{
    	F1->on();
             if(TEMPERATURE>T5||HUMIDITY>RH_THRESHOLD)  
                F1->regulate(5);
             else if(TEMPERATURE>T4)
                F1->regulate(4);
             else if(TEMPERATURE>T3)
                F1->regulate(3);
             else if(TEMPERATURE>T2)
                F1->regulate(2);
             else if(TEMPERATURE>T1)
                F1->regulate(1);	
    	}
             else 
                F1->off();
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
       AC1->off();
       F1->off();
       L->off();
    }    
      
  }
  
  handleWebRequest();
}

void readSensorData()
{
  
}

void handleWebRequest()
{
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
                        L->dim(1);
                     else if(value[0]=='2')
                        L->dim(2);
                     else
                        L->dim(3);
                     
                     for(int i=0;i<5;i++)value[i]=' '; //Char array/String
                     
                     //Set fan state
                     value[0]=page[54];
                     if(value[0]=='1') //Char array/String
                        F1->on();
                     else
                        F1->off();
                     
                     for(int i=0;i<5;i++)value[i]=' '; //Char array/String
                     
                     //Set fan speed
                     value[0]=page[65];
                     if(value[0]=='0') //Char array/String
                        F1->off();
                     else if(value[0]=='1')
                        F1->regulate(1);
                     else if(value[0]=='2')
                        F1->regulate(2);
                     else 
                        F1->regulate(3);
                     
                     for(int i=0;i<5;i++)value[i]=' '; //Char array/String
                     
                     //Set AC
                     value[0]=page[76];
                     if(value[0]=='0') //Char array/String
                        AC1->off();
                     else
                     {
                        value[1]=page[92];   //Temperature
                        value[2]=page[93];   //Temperature
                        value[3]=page[103];
                        byte reqTemp=((byte)value[1]-48)*10 + ((byte)value[2]-48);
                        byte reqSpeed=(byte)value[3]-48;
                        AC1->set(reqTemp,reqSpeed);
                     }
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

void lightRecommend(EthernetClient cl)
{
    if(LIGHT_INTENSITY<LIGHT_THRESHOLD)   //dim(1) means highly dim, dim(3) is very bright
    {
       cl.println("On at brightness ");
      if(LIGHT_INTENSITY<L1)
         cl.println("1</p>");
      else if(LIGHT_INTENSITY<L2)
         cl.println("2</p>");
      else if(LIGHT_INTENSITY<L3)
         cl.println("3</p>");
    }
    else
      cl.println("Off</p>");
}

void fanRecommend(EthernetClient cl)
{
   if(TEMPERATURE>FAN_CUTOFF)
      cl.println("Off</p>");
    
   else if(TEMPERATURE<AC_CUTOFF)
   {
      if(TEMPERATURE>T1)
      {
         cl.println("On at speed ");
         if(TEMPERATURE>T5||HUMIDITY>RH_THRESHOLD)   
            cl.println("5</p>");
         else if(TEMPERATURE>T4)
            cl.println("4</p>");
         else if(TEMPERATURE>T3)
            cl.println("3</p>");
         else if(TEMPERATURE>T2)
            cl.println("2</p>");
         else 
            cl.println("1</p>");
      }
      else 
         cl.println("Off</p>");
   }
}

void ACRecommend(EthernetClient cl)
{
   if(TEMPERATURE>FAN_CUTOFF)
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

void sendManualPage(EthernetClient cl)
{
   cl.println("<!DOCTYPE html");
   cl.println("<html>");
   cl.println("<head>");
   cl.println("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />");
   cl.println("<title>Manual Mode</title>");
   cl.println("</head>");
   cl.println("<body>");
   cl.println("<h1>Currently in manual mode. Go to the auto mode page to set it to auto and/or change the auto mode settings.<h1>"); //Uncomment
   cl.println("<form method=\"get\">");
   
   cl.println("<h2>Light</h2>");
   if(L->getState()==ON) //Light on/off
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
   
   cl.println("Brightness:<input type=\"number\" id=\"lightBrightness\" name=\"lightBrightness\" min=\"0\" max=\"3\" "); //Brightness
   if(L->getState()==OFF)
      cl.println("value=\"0\" />");
   else if(L->getDimLevel()==1)
      cl.println("value=\"1\" />");
   else if(L->getDimLevel()==2)
      cl.println("value=\"2\" />");
   else 
      cl.println("value=\"3\" />");
      
   cl.println("<p id=\"lightStatus\">Current status is"); //Brightness
   if(L->getState()==ON)
   {
      cl.println("on at brightness ");
      if(L->getDimLevel()==1)
         cl.println("1</p>");
      else if(L->getDimLevel()==2)
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
   if(F1->getState()==ON)  //Fan on/off
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
   
   cl.println("Speed:<input type=\"number\" id=\"fanSpeed\" name=\"fanSpeed\" min=\"0\" max=\"3\" "); //Fan speed
   if(F1->getState()==OFF)
      cl.println("value=\"1\" />");
   else if(F1->getSpeed()==1)
      cl.println("value=\"1\" />");
   else if(F1->getSpeed()==2)
      cl.println("value=\"2\" />");
   else 
      cl.println("value=\"3\" />");
   
   cl.println("<p id=\"fanStatus\">Current status is ");
   if(F1->getState()==ON)
   {
      cl.println("on at speed ");
      if(F1->getSpeed()==1)
         cl.println("1</p>");
      else if(F1->getSpeed()==2)
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
   
   if(AC1->getState()==ON)
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
   
   cl.println("Temperature:<input type=\"number\" id=\"ACTemperature\" name=\"ACTemperature\" min=\"18\" max=\"30\" value=\""); 
   cl.println(AC1->getTemp());
   cl.println("\" />");
   cl.println("<br />");
   
   cl.println("Fan speed:<input type=\"number\" id=\"ACSpeed\" name=\"ACSpeed\" min=\"1\" max=\"3\" value=\"");
   cl.println(AC1->getFanSpeed());
   cl.println("\" />");
   
   cl.println("<p id=\"ACStatus\">Current status is ");
   if(AC1->getState()==ON)
   {
      cl.println("on at temperature ");
      cl.println((int)AC1->getTemp());
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
   cl.println("<input type=\"button\" id=\"autoButton\" value=\"Auto mode\" />");
   cl.println("<input type=\"button\" id=\"backButton\" value=\"Back\" onclick=\"window.location='/'\"/>");
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
   cl.println("<h1>Currently in auto mode. Go to manual mode to set to manual mode. Go to auto mode to set to auto mode and/or change auto mode settings."); //Change Uncomment
   
   //Light
   cl.println("<h2>Light</h2>");
   cl.println("<br />");
   
   cl.println("<p id=\"lightStatus\">Current status is"); //Brightness
   if(L->getState()==ON)
   {
      cl.println("on at brightness ");
      if(L->getDimLevel()==1)
         cl.println("1</p>");
      else if(L->getDimLevel()==2)
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
   if(F1->getState()==ON)
   {
      cl.println("on at speed ");
      if(F1->getSpeed()==1)
         cl.println("1</p>");
      else if(F1->getSpeed()==2)
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
   if(AC1->getState()==ON)
   {
      cl.println("on at temperature ");
      cl.println((int)AC1->getTemp());
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
   cl.println("<input type=\"button\" id=\"manualButton\" value=\"Manual mode\" onclick=\"window.location='/manual.html'\"/>");
   cl.println("<input type=\"button\" id=\"autoButton\" value=\"Auto mode\" />");
   cl.println("</body>");
   cl.println("</html>");
}
void detectLaser1()
{
  if(digitalRead(laserPin1)==1-prevState[0])
  {
    laser[0][count]=1-prevState[0];
    laser[1][count]=prevState[1];
    prevState[0]=laser[0][count];
    prevState[1]=laser[1][count];
    count++;
    if(count==5)
    {
      if(laser[0][0]==1 && laser[0][1]==0 && laser[0][2]==0 && laser[0][3]==1 && laser [0][4]==1&&
         laser[1][0]==1 && laser[1][1]==1 && laser[1][2]==0 && laser[1][3]==0 && laser [1][4]==1 )
         {
           NUM_PERSONS++;
           detected=true;
         }
         
      else if(laser[1][0]==1 && laser[1][1]==0 && laser[1][2]==0 && laser[1][3]==1 && laser [1][4]==1&&
         laser[0][0]==1 && laser[0][1]==1 && laser[0][2]==0 && laser[0][3]==0 && laser [0][4]==1 )
        {
           NUM_PERSONS--;
           detected=true;
         }
       if(detected)
       {
         count=1;
         prevState[0]=1;
         prevState[1]=1;
         detected=false;
       }
    }
    if(count>4)
    {
      count=4;
      for(int i=0;i<4;i++)
      {
        laser[0][i]=laser[0][i+1];
        laser[1][i]=laser[1][i+1];
      }
    }
  }
}
void detectLaser2()
{
    if(digitalRead(laserPin2)==1-prevState[1])
  {
    laser[1][count]=1-prevState[1];
    laser[0][count]=prevState[0];
    prevState[0]=laser[0][count];
    prevState[1]=laser[1][count];
    count++;
    if(count==5)
    {
      if(laser[0][0]==1 && laser[0][1]==0 && laser[0][2]==0 && laser[0][3]==1 && laser [0][4]==1&&
         laser[1][0]==1 && laser[1][1]==1 && laser[1][2]==0 && laser[1][3]==0 && laser [1][4]==1 )
         {
           NUM_PERSONS++;
           detected=true;
         }
         
      else if(laser[1][0]==1 && laser[1][1]==0 && laser[1][2]==0 && laser[1][3]==1 && laser [1][4]==1&&
         laser[0][0]==1 && laser[0][1]==1 && laser[0][2]==0 && laser[0][3]==0 && laser [0][4]==1 )
        {
           NUM_PERSONS--;
           detected=true;
         }
       if(detected)
       {
         count=1;
         prevState[0]=1;
         prevState[1]=1;
         detected=false;
       }
    }
    if(count>4)
    {
      count=4;
      for(int i=0;i<4;i++)
      {
        laser[0][i]=laser[0][i+1];
        laser[1][i]=laser[1][i+1];
      }
    }
  }
}

