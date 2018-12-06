/***
* SHARP.h 
*
* Sharp GP2Y1010 dust sensor library
*
* This library was written for the STMF103C8 Blue Pill (or equiv)
* The blue pill runs at 72Mhz and the ADC conversion is fast enough
* to be able to plot the sensor Vout when the LED is turned on 
* Slower Arduinos may struggle to sample the Vout exactly 280us after
* without tweaking ADC conversion registers
*
* Also, the ADC reference voltage is 3.3v on other Arduinos it may be 5v.
*
*
***/

#ifndef SHARP_H
#define SHARP_H

#include "arduino.h"

// LED Control
// if you drive the Sharp directly from an MCU LOW turns the LED on
#define LED_ON	LOW
#define LED_OFF	HIGH

// Sensor output measurement
// The Sharp sensor uses a 5V supply so it's output voltage is 0->5v
// although it tops out at around 3.6v (Use a 3.3v zener)- see also SENSITIVITY
// HOWEVER, the STM32F103C8 ADC uses a 3.3v reference voltage so the
// ADC reading conversion to volts is done using 3.3v
#define SENSOR_VOLT_RANGE	3.3

// ADC range
// the STM32F103 has a 12 bit ADC
// Set this to match your hardware
// note the range starts at and max value is one less than 
// binary max
#define ADC_RANGE	4095

// Sensitivity
// Spec says the device typically is 0.5v per 0.1 mg/m^3 (=5.0) but can range
// between 0.35 and 0.65 - this pushes the calculated density up or down.
// If the sensor density reading appears low/high you could try calibrating this value
// on a per sensor basis (ouch!) which means you need some standard
// Sharp say the sensors are calibrated at the factory and not to touch the Pot.

#define SENSITIVITY	5.0 // v/(mg/m^3)

// zero voltage (Voc)
// spec says this typically ranges between 0.6 and 1.5 and is normally 0.9
// To determine your sensor's zero put it into a vacuum storage bag and extract the air
// use the average reading as your Voc
// Note: my sensor took about 5 minutes for this value to settle after power on.
// your code can  set this on the fly using setVoc(newVoc) after averaging several getVoltage()
// values when 
#define INITIAL_VOC	0.01616

class SHARP{


public:

  SHARP(uint8_t ledControl,uint8_t analogIn);

  uint16_t getRaw();  			// returns analogRead() value only
  float getDensity();			// returns (voltage-_Voc)/_sensitivity
  void setVoc(float newVoc);    // zero voltage usually 0.6 to 1.0 volts
  float getVoc();				// returns current _Voc value
  float getVoltage();			// returns the current sensor reading converted to voltage
  
private:

  uint8_t _ledControl;		// pin used to turn on the sensor led
  uint8_t _analogIn;		// analog input from the sensor
  float _Voc=INITIAL_VOC; 	// current zero voltage

  // chart on pg5 of the spec shows linear slope
  float	_sensitivity=SENSITIVITY;	// voltage/density ratio

};

#endif
