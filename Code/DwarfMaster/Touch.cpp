#include "Touch.h"

Touch::Touch()
{
}

Point Touch::getTouchPoint()
{
  Point p;
  p.x = -1;
  p.y = -1;
  p.pressed = false;
  TSPoint tsp = ts.getPoint();
  pinMode(YP, OUTPUT);      //restore shared pins
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH);   //because TFT control pins
  digitalWrite(XM, HIGH);

  if (tsp.x > 940 || tsp.y > 880)
    return p;

  if (tsp.z > MINPRESSURE && tsp.z < MAXPRESSURE)
  {
    p.pressed = true;
    //p.x = map(tsp.x, TS_LEFT, TS_RT, 0, tft.width()); 
    //p.y = map(tsp.y, TS_TOP, TS_BOT, 0, tft.height());
    p.x = map(tsp.y, TS_TOP, TS_BOT, 0, 320);
    p.y = map(tsp.x, TS_RT, TS_LEFT, 0, 240);
  }
  return p;
}
