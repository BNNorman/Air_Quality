/*
ZPH01.cpp

Non-blocking code for reading the serial data stream from a ZPH01 sensor

*/

#include <ZPH01.h>

ZPH01::ZPH01(Stream & s)
{
_ss=&s;
}


//////////////////////////////////////
bool ZPH01::validate()
{
uint8_t tempq=0;

// sum parts 1 to 7
for(uint8_t j=1;j<8;j++)
	{
	tempq+=rxData[j];
	}
tempq=(~tempq)+1;

if (tempq==rxData[8]) return true;
return false;
}


/*
loop() should be called from the main program loop()
it gathers serial data in a non-blocking way
*/
void ZPH01::loop()
{
if (!_ss->available()) return;

uint8_t ch=_ss->read();

switch (sensorStatus){

	case pendingFF:
		if (ch!=0xFF) return;
		count=0;
		rxData[count++]=ch;
		sensorStatus=pending18;	// FF should be followed by 0x18
		break;
		
	case pending18:
		if (ch!=0x18){
		// try again
		sensorStatus=pendingFF;
		return;
		}
		rxData[count++]=ch;
		sensorStatus=reading;
		break;
		
	case reading:
		rxData[count++]=ch;
		if (count<9) return;

		if (validate()){
			// rxData[3] has a range 0->99
			// rxData[4] also has a range 0->99
			// so the complete range is 0->99.99
			lowPulseRate=rxData[3]+0.01*rxData[4];
			dataValid=true;
			}
			
		// wait for the next data frame
		sensorStatus=pendingFF;	
	}
}



/////////////////////////////////
float ZPH01::getLowPercent()
{
// low pulserate is cached on first read
// and is only updated if the checksum is
// correct for subsequent reads
if (!dataValid) return -1.0;	// special case

return lowPulseRate;
}
