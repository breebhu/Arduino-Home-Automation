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
              if (value[0] == '0')
                L->off();
              else if (value[0] == '1')
                L->dim(1);
              else if (value[0] == '2')
                L->dim(2);
              else
                L->dim(3);

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
              if (value[0] == '0')
                F1->off();
              else if (value[0] == '1')
                F1->regulate(1);
              else if (value[0] == '2')
                F1->regulate(2);
              else
                F1->regulate(3);

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
            int index = 21;
            //New state set //The seemingly random indexes are the places where values will be present when data is sent through the correct format.
            if (page[index] = '?')
            {
              LIGHT_TRESHOLD = 0;
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
              for (; page[index] != '&'; index++)
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
  if (LIGHT_INTENSITY < LIGHT_THRESHOLD)   //dim(1) means highly dim, dim(3) is very bright
  {
    cl.println("On at brightness ");
    if (LIGHT_INTENSITY < L2)
      cl.println("3</p>");
    else if (LIGHT_INTENSITY < L1)
      cl.println("2</p>");
    else
      cl.println("3</p>");
  }
  else
    cl.println("Off</p>");
}

void fanRecommend(EthernetClient cl)
{
  if (AC1->getState() == OFF)
  {
    if (TEMPERATURE > T1 && HUMIDITY > H1)
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

void ACRecommend(EthernetClient cl)
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
  else if (L->getDimLevel() == 1)
    cl.println("value=\"1\" />");
  else if (L->getDimLevel() == 2)
    cl.println("value=\"2\" />");
  else
    cl.println("value=\"3\" />");

  cl.println("<p id=\"lightStatus\">Current status is"); //Brightness
  if (L->getState() == ON)
  {
    cl.println("on at brightness ");
    if (L->getDimLevel() == 1)
      cl.println("1</p>");
    else if (L->getDimLevel() == 2)
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

  cl.println("Temperature:<input type=\"number\" id=\"ACTemperature\" name=\"ACTemperature\" min=\"17\" max=\"30\" value=\"");
  cl.println(AC1->getTemp());
  cl.println("\" />");
  cl.println("<br />");

  cl.println("Fan speed:<input type=\"number\" id=\"ACSpeed\" name=\"ACSpeed\" min=\"1\" max=\"3\" value=\"");
  cl.println(AC1->getFanSpeed());
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

  //Light
  cl.println("<h2>Light</h2>");
  cl.println("<br />");

  cl.println("<p id=\"lightStatus\">Current status is"); //Brightness
  if (L->getState() == ON)
  {
    cl.println("on at brightness ");
    if (L->getDimLevel() == 1)
      cl.println("1</p>");
    else if (L->getDimLevel() == 2)
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
    if (L->getDimLevel() == 1)
      cl.println("1</p>");
    else if (L->getDimLevel() == 2)
      cl.println("2</p>");
    else
      cl.println("3</p>");
  }
  else
    cl.println("off</p>");

  cl.println("<p>On below:");
  cl.print("<input type=\"number\" name=\"lightThreshold\" id=\"lightThreshold\" min=");
  cl.print(L1);
  cl.print(" max=1023 value=");
  cl.print(LIGHT_THRESHOLD);
  cl.println(" />");
  cl.println("</p>");

  cl.println("<p>Brightness level 2 below:");
  cl.print("<input type=\"number\" name=\"L1\" id=\"L1\" min=");
  cl.print(L2);
  cl.print(" max=");
  cl.print(LIGHT_THRESHOLD);
  cl.print(" value=");
  cl.print(L1);
  cl.println(" />");
  cl.println("</p>");


  cl.println("<p>Brightness level 3 below:");
  cl.print("<input type=\"number\" name=\"L2\" id=\"L2\" min=0 max=");
  cl.print(L1);
  cl.print(" value=");
  cl.print(L2);
  cl.println(" />");
  cl.println("</p>");

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

  cl.println("<p>Ambient temperature:");
  cl.print("<input type=\"number\" name=\"ambientTemperature\" id=\"ambientTemperature\" min=21 max=34 value=");
  cl.print(FAN_CUTOFF - 2);
  cl.println(" />");
  cl.println("</p>");

  cl.println("<p>Default AC fan speed:");
  cl.print("<input type=\"number\" name=\"ACSpeed\" id=\"ACSpeed\" min=1 max=3 value=");
  cl.print(DEFAULT_AC_FAN_SPEED);
  cl.println(" />");
  cl.println("</p>");

  cl.println("<p>Fan on above temperature:");
  cl.print("<input type=\"number\" name=\"T1\" id=\"T1\" min=0 max=");
  cl.print(AC_CUTOFF);
  cl.print(" value=");
  cl.print(T1);
  cl.println(" />");
  cl.println("</p>");

  cl.println("<p>Fan on above humidity:");
  cl.print("<input type=\"number\" name=\"H1\" id=\"H1\" min=0 max=");
  cl.print(H2);
  cl.print(" value=");
  cl.print(H1);
  cl.println(" />");
  cl.println("</p>");

  cl.println("<p>Fan speed 2 above humidity:");
  cl.print("<input type=\"number\" name=\"H2\" id=\"H2\" min=");
  cl.print(H1);
  cl.print(" max=");
  cl.print(H3);
  cl.print(" value=");
  cl.print(H2);
  cl.println(" />");
  cl.println("</p>");

  cl.println("<p>Fan speed 3 above humidity:");
  cl.print("<input type=\"number\" name=\"H3\" id=\"H3\" min=");
  cl.print(H2);
  cl.print(" max=100 value=");
  cl.print(H3);
  cl.println(" />");
  cl.println("</p>");

  cl.println("<input type=\"submit\" />");
  cl.println("</form>");

  cl.println("<br/>");
  cl.println("<input type=\"button\" id=\"refreshButton\" value=\"Refresh\" onclick=\"window.location.reload()\" />");
  cl.println("<input type=\"button\" id=\"manualButton\" value=\"Manual mode\" onclick=\"window.location='/manual.html'\" />");
  cl.println("<input type=\"button\" id=\"autoButton\" value=\"Auto mode\" onclick=\"window.location='/'\"/>");
  cl.println("</body>");
  cl.println("</html>");
}
