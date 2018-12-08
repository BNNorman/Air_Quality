#include <ZPH01.h>


//////////////////////////////////////
bool ZPH01::Validate(uint8_t Checksum)
{
uint8_t tempq=0;

// sum parts 1 to 7
for(uint8_t j=1;j<8;j++)
	{
	tempq+=Data[j];
	}
tempq=(~tempq)+1;

if (tempq==Checksum) return true;
return false;
}


//////////////////////////
bool ZPH01::ReceiveData()
{
// data rate is once per second
uint32_t Start=millis();
while (serial.available()==0)
	{
	if ((millis()-Start)>=RECV_TIMEOUT)	return false;	// timeout
	}
//look for start byte 0x00
Start=millis();
while (serial.peek()!=0xFF)
	{
	serial.read();
	if ((millis()-Start)>=RECV_TIMEOUT) return false; // timeout
	}
	
// read 9 characters or timeout
Start=millis();
for (uint8_t i=0;i<sizeof ZPH01::Data;i++)
	{
	ZPH01::Data[i]=serial.read();
	while (serial.available()==0)
		{
		if ((millis()-Start)>=RECV_TIMEOUT)	return false; // timeout
		}
	}
	
// we have received 9 bytes - check the checksum
if (Validate(Data[8])) return true;
return false;
}

/////////////////////////////////
float ZPH01::GetLowPercent()
{
if (!ReceiveData()) return -1;	// special case

return ((float)ZPH01::Data[3]+0.1*ZPH01::Data[4]);
}