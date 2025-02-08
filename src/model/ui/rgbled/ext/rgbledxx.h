/*
	RGBLED.h - Library for controlling an RGB LED
	Created by Bret Stateham, November 13, 2014
	Released into the public domain
	You can get the latest version from http://github.com/BretStateham/rgbled

	To create an instance, call the RGBLED constructor with the red, green, and blue pins, as well as the type, COMMON_CATHODE or COMMON_ANODE

	There are two basic types of RGB leds, COMMON_CATHODE and COMMON_ANODE:
	COMMON_CATHODE RGB LEDs have a common cathode "leg" that should be connected to ground (GND) arduino
	COMMON_ANODE RGB LEDs have a common anode "leg" that should be connected to 5V on the arduino

	You need to physically connect the RGB LED's Red, Green and Blue pins to Pulse Width Modulation (PWM) capable pins on the Arduino
	On the Arduino, digital pins 3,5,6,9,10 & 11 are PWM pins as indicated by the "~" infront of their pin numbers on the board

	Create an RGBLED instance by passing the red, green, blue and type (COMMON_CATHODE or COMMON_ANODE) into the constructor.

	For example, the following code would create an RGBLED instance named rgbLED for a COMMON_ANODE RGB LEDs with the red, green and blue
	legs connected to 11,9 & 6 respectively:

	RGBLED rgbLed(11,9,6,COMMON_ANODE);

	You can then call then various methods on the RGBLED instance:

	writeRGB(red,green,blue);
	writeRed(red);
	writeGreen(green);
	writeBlue(blue);
	randomColor();
	turnOff();
	mapValue();

*/

#ifndef RGBLEDxx_h
#define RGBLEDxx_h

typedef enum
{
  // Used to indicate the RGB LED is a Common Cathode LED
  COMMON_CATHODE = 0,
  // Used to indicate the RGB LED is a Common Anode LED
  COMMON_ANODE   = 1,
} common_type;

class RGBLEDxx
{
public:

	//LED Pins
	int redPin;
	int greenPin;
	int bluePin;

	//RGB Values
	int redValue;
	int greenValue;
	int blueValue;

	//LED Type: COMMON_ANODE or COMMON_CATHODE
	int commonType;

	//RGB Values mapped based on commonType:
	int redMappedValue;
	int greenMappedValue;
	int blueMappedValue;

	//Constructor
	RGBLEDxx(int redPin,int greenPin, int bluePin, int commonType);

	// Single method to set all pins
	// red,green,blue values range from 0(off)-255(on)
	void writeRGB(int red, int green, int blue);

	// Sets just the red pin
	// Values range from 0(off)-255(on)
	void writeRed(int red);

	// Sets just the greem pin
	// Values range from 0(off)-255(on)
	void writeGreen(int green);

	// Sets just the blue pin
	// Values range from 0(off)-255(on)
	void writeBlue(int blue);

	// Turns all pins off
	void turnOff(void);

	// Show a random color
	void writeRandom(void);

	// Writes the LED based on HSV values
	void writeHSV(int h, double s, double v);

	// Show the HSV Color Wheel
	// Take in the delay (dly) in milliseconds between each color
	void writeColorWheel(int dly);

	// Map a value based on the common_type
	int mapValue(int value);


};

#endif