#ifndef NATIVE

#include <Arduino.h>
#include "rgbledxx.h"

// Constructor
RGBLEDxx::RGBLEDxx(int rPin, int gPin, int bPin, int type) {
	redPin = rPin;
	greenPin = gPin;
	bluePin = bPin;
	commonType = type;

	turnOff();
}

void RGBLEDxx::writeRGB(int red, int green, int blue) {
  writeRed(red);
  writeGreen(green);
  writeBlue(blue);
}

void RGBLEDxx::writeRed(int red) {
	redValue = red;
	redMappedValue = mapValue(redValue);
	analogWrite(redPin,redMappedValue);
}

void RGBLEDxx::writeGreen(int green) {
	greenValue = green;
	greenMappedValue = mapValue(greenValue);
	analogWrite(greenPin,greenMappedValue);
}

void RGBLEDxx::writeBlue(int blue) {

	blueValue = blue;
	blueMappedValue = mapValue(blueValue);
	analogWrite(bluePin,blueMappedValue);
}

void RGBLEDxx::turnOff() {
	writeRed(0);
	writeGreen(0);
	writeBlue(0);
}

void RGBLEDxx::writeRandom() {
	int r = random(0,255);
	int g = random(0,255);
	int b = random(0,255);

	writeRGB(r,g,b);
}

int RGBLEDxx::mapValue(int value) {
	value = (value < 0) ? 0 : (value > 255) ? 255 : value;
	value = (commonType == COMMON_ANODE) ? 255-value : value;
	return value;
}

// Convert a given HSV (Hue Saturation Value) to RGB(Red Green Blue) and set the led to the color
//   h is hue value, integer between 0 and 360
//   s is saturation value, double between 0 and 1
//   v is value, double between 0 and 1
// Stolen from: http://eduardofv.com/read_post/179-Arduino-RGB-LED-HSV-Color-Wheel-
// Based on: http://www.splinter.com.au/converting-hsv-to-rgb-colour-using-c/
void RGBLEDxx::writeHSV(int h, double s, double v) {
  //this is the algorithm to convert from RGB to HSV
  double r=0;
  double g=0;
  double b=0;

  double hf=h/60.0;

  int i=(int)floor(h/60.0);
  double f = h/60.0 - i;
  double pv = v * (1 - s);
  double qv = v * (1 - s*f);
  double tv = v * (1 - s * (1 - f));

  switch (i)
  {
  case 0: //rojo dominante
    r = v;
    g = tv;
    b = pv;
    break;
  case 1: //verde
    r = qv;
    g = v;
    b = pv;
    break;
  case 2:
    r = pv;
    g = v;
    b = tv;
    break;
  case 3: //azul
    r = pv;
    g = qv;
    b = v;
    break;
  case 4:
    r = tv;
    g = pv;
    b = v;
    break;
  case 5: //rojo
    r = v;
    g = pv;
    b = qv;
    break;
  }

  //set each component to a integer value between 0 and 255
  int red=constrain((int)255*r,0,255);
  int green=constrain((int)255*g,0,255);
  int blue=constrain((int)255*b,0,255);

  writeRGB(red,green,blue);
}


// Cycle through the color wheel
// Stolen from: http://eduardofv.com/read_post/179-Arduino-RGB-LED-HSV-Color-Wheel-
void RGBLEDxx::writeColorWheel(int dly) {
  //The Hue value will vary from 0 to 360, which represents degrees in the color wheel
  for(int hue=0;hue<360;hue++)
  {
    writeHSV(hue,1,1); //We are using Saturation and Value constant at 1
    delay(dly); //each color will be shown for 10 milliseconds
  }
}

#endif
