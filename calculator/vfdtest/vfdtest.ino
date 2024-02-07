// vfdtest - first test for the VFD
// Based on work by qrt@qland.de
// See: https://github.com/qrti/VFD-HCS-12SS59T

#include "VFD.h"

const int Pin_LED		=	3;

void setup()
{
  pinMode(Pin_LED, OUTPUT);
  digitalWrite(Pin_LED, HIGH);

	Serial.begin(9600); //Nodig?
	Vfd.init();

	Vfd.write("QRT@QLAND_DE * ABCDEFGHIJKLMNOPQRSTUVWXYZ * 0123456789 * ");		// display
	Vfd.scroll(20);																// scroll left  (+) 20 * 0.01 chars per second
}																				//        right (-)
																				//        stop  (0)
void loop()
{
  digitalWrite(Pin_LED, LOW);
  delay(200);
  digitalWrite(Pin_LED, HIGH);
  delay(1000);
}
