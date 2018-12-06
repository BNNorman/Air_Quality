/*
*
* SHARP dust sensor
*/
#include "SHARP.h"

// uncomment to allow code to adjust zero (_Voc)
//#define ADJUST_ZERO

SHARP::SHARP(uint8_t ledControl,uint8_t analogIn)
{
	_ledControl=ledControl;
	_analogIn=analogIn;

	pinMode(_ledControl,OUTPUT);
	digitalWrite(_ledControl,HIGH);  // low enables

}


float SHARP::getVoc()
{
// returns the current Voc value (volts)
return _Voc;
}

uint16_t SHARP::getRaw()
{
  // maximum recall rate is every 10ms (100hz)
  digitalWrite(_ledControl,LED_ON); 
  delayMicroseconds(280);             // see datasheet
  uint16_t raw=analogRead(_analogIn); // STM32 only takes 8uS
  digitalWrite(_ledControl,LED_OFF);
  return raw;
}

float SHARP::getVoltage()
{
  // calculate the voltage of the sensor. The sensor is normally run with VCC=SENSOR_VOLT_RANGE
  // which is normally 5v but the sensor output tops out at around 3.6v according to the
  // data sheet 
  
  // make sure the calculations are done in floating point otherwise integer division
  // will give a false zero result
  float voltage=(getRaw()/(float) ADC_RANGE)*(float) SENSOR_VOLT_RANGE;

  return voltage;
}

void SHARP::setVoc(float newVoc)
{
  // only use this if you know the sensor is in a zero particle environment (vacuum)
  // then you can call setVoc(getVoltage());
  _Voc=newVoc;
}

float SHARP::getDensity()
{

// returns density in mg/m^3

float volt=getVoltage();

if (volt<_Voc) return 0.0;

// slope is inverted to translate voltage to density
return (volt-_Voc)/_sensitivity;
}
