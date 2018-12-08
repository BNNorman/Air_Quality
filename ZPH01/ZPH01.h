#ifndef ZPH01_H
#define ZPH01_H

#include <arduino.h>


//  Serial data format
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


#define RECV_TIMEOUT 2000	// 2 SECONDS

class ZPH01
{

public:
	// take input from any stream object
	ZPH01(Stream &s):serial(s){};
		
	// returns -1 if no serial data available
	// could be a timeout or invalid checksum
	// otherwise returns the %
	float GetLowPercent();

private:
	Stream& serial;
	uint8_t Data[9]={0};
	
	bool Validate(uint8_t Checksum);
	bool ReceiveData();
};


#endif