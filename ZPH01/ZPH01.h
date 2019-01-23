/*
ZPH01.h

Non-blocking code for reading the serial data stream from a ZPH01 sensor

*/
#pragma once

#include <arduino.h>


//  Serial data format (9 bytes)
//
// uint8_t	StartByte;			0xFF
// uint8_t	DetectionType;		0x18 in datasheet
// uint8_t	UnitLowPulseRate;	0x00 in datasheet
// uint8_t	IntegerPart;		low pulse rate integer part 0x00-0x63
// uint8_t	DecimalPart;		low pulse rate decimal part 0x00-0x63
// uint8_t	Reserved1;			0x00
// uint8_t	Mode;				0x01  == sample period is 1 second
// uint8_t	Reserved2;			0x00
// uint8_t	Checksum;			0x00-0xFF calculated sum of parts 1 though 7 negated plus 1
//



class ZPH01
{
enum ZPH01_SENSOR_STATUS {pendingFF,pending18,reading};	// used to control loop()


public:
	// take input from any stream object
	ZPH01(Stream &s);
	
	// getLowPercent()
	// returns -1 if no serial data available (dataValid=false)
	// could be a timeout or invalid checksum
	// otherwise returns the %
	float getLowPercent();
	
	bool dataValid=false;	// set after a sucessful read
	
	// loop() is non-blocking
	// if data capture is successful it sets dataValid. You should test this
	void loop();			// call in loop() to capture data - non-blocking

private:
	Stream *_ss;			
	uint8_t rxData[9]={0};
	
	bool validate();
	ZPH01_SENSOR_STATUS sensorStatus=pendingFF;
	uint8_t count=0;
	float lowPulseRate=0;
};