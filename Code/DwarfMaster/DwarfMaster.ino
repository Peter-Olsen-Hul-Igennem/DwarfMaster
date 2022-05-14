#include <arduino.h>
#include "Screen.h"
#include "ButtonState.h"
#include "Touch.h"

/*
 * MainState:
 * 0: Play
 * 1: Edit
 */
byte mainState = 0; 

Screen screen;
Touch touch;// = Touch();
Point p;

void setup() {
  Serial.begin(9600);
  delay(1000); // Giving Serial a chance to come online

  String txt = "Button 1";
  String old = "FISSEKARL";
  String buf = screen.getKeyboardInputFromUser(&txt, &old, 9);
  Serial.println(buf);
  //screen.drawBank("00", "1234567890", false);

}

void loop() {
  /*
  p = touch.getTouchPoint();
  if (p.pressed)
  {
    screen.mapPointToKeyboardButton(p);
  }
*/
  
}
