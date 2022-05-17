#ifndef DWARFMASTER_TOUCH_H
#define DWARFMASTER_TOUCH_H

#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

struct Point
{
    bool pressed;
    int x;
    int y;
};

class Touch
{
 public:
    Touch();
    Point getTouchPoint();
    void waitForTouchReleased();

 private:
    const int MINPRESSURE = 200;
    const int MAXPRESSURE = 700;
    const int RESISTANCE  = 350;
    const int XP = 8, XM = 16, YP = 17, YM = 9; // 240x320 ID=0x9341
    const int TS_LEFT = 937, TS_RT = 99, TS_TOP = 72, TS_BOT = 915;

    TouchScreen ts = TouchScreen(XP, YP, XM, YM, RESISTANCE);
    MCUFRIEND_kbv tft;
};

#endif
