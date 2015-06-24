volatile byte laser[2][5];
volatile boolean detected = false;
boolean flag = true;
volatile byte count = 0;
volatile byte prevState[2];
volatile byte numPersons = 0;
const byte pin1 = 2, pin2 = 3;
void setup()
{

}
void loop()
{
  if (flag)
  {
    if (digitalRead(pin1) == 1 && digitalRead(pin2) == 1)
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
  Serial.println(numPersons);
  delay(5000);

}
void detectLaser1()
{
  if (digitalRead(pin1) == 1 - prevState[0])
  {
    laser[0][count] = 1 - prevState[0];
    laser[1][count] = prevState[1];
    prevState[0] = laser[0][count];
    prevState[1] = laser[1][count];
    count++;
    if (count == 5)
    {
      if (laser[0][0] == 1 && laser[0][1] == 0 && laser[0][2] == 0 && laser[0][3] == 1 && laser [0][4] == 1 &&
          laser[1][0] == 1 && laser[1][1] == 1 && laser[1][2] == 0 && laser[1][3] == 0 && laser [1][4] == 1 )
      {
        numPersons++;
        detected = true;
      }

      else if (laser[1][0] == 1 && laser[1][1] == 0 && laser[1][2] == 0 && laser[1][3] == 1 && laser [1][4] == 1 &&
               laser[0][0] == 1 && laser[0][1] == 1 && laser[0][2] == 0 && laser[0][3] == 0 && laser [0][4] == 1 )
      {
        numPersons--;
        detected = true;
      }
      if (detected)
      {
        count = 1;
        prevState[0] = 1;
        prevState[1] = 1;
      }
    }
    if (count > 4)
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
void detectLaser2()
{
  if (digitalRead(pin2) == 1 - prevState[1])
  {
    laser[1][count] = 1 - prevState[1];
    laser[0][count] = prevState[0];
    prevState[0] = laser[0][count];
    prevState[1] = laser[1][count];
    count++;
    if (count == 4)
    {
      if (laser[0][0] == 1 && laser[0][1] == 0 && laser[0][2] == 0 && laser[0][3] == 1 && laser [0][4] == 1 &&
          laser[1][0] == 1 && laser[1][1] == 1 && laser[1][2] == 0 && laser[1][3] == 0 && laser [1][4] == 1 )
        numPersons++;

      else if (laser[1][0] == 1 && laser[1][1] == 0 && laser[1][2] == 0 && laser[1][3] == 1 && laser [1][4] == 1 &&
               laser[0][0] == 1 && laser[0][1] == 1 && laser[0][2] == 0 && laser[0][3] == 0 && laser [0][4] == 1 )
        numPersons--;
      count = 1;
      prevState[0] = 1;
      prevState[1] = 1;
    }
    if (count > 4)
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

